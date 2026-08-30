#include "prpch.h"
#include "PrismShaderCache.h"
#include "Prism/Core/Hash.h"


#include "Prism/Renderer/RendererAPI.h"

#include <cstring>
#include <fstream>
#include <iterator>

namespace Prism
{
    //[Header]  定长
    //u32  Magic = 0x50534843   // "PSHC"
    //u32  FormatVersion = 1
    //u64  SourceSetFingerprint 
    //u32  EntryCount

    //[IndexTable]  EntryCount × 定长条目
    //u64  KeyHash          // = FNV64(nameHash, passIndex, keywordMask, backend)
    //u64  Offset           // 相对数据块起点的偏移

    //[Entries]  数据块，length - prefixed 连续内存   // 加载后整块保留
    //每条目（按 IndexTable.Offset 定位）：
    //u64   Backend          // 1=OpenGL 2=Vulkan；加载时靠它区分 GLSL / SPIR-V
    //u64  VertexSize + VertexSize 字节
    //u64  FragmentSize + FragmentSize 字节
    //u64  ReflectionSize + ReflectionSize 字节

    // ===== 磁盘格式常量 =====

    constexpr uint64_t SHADER_CACHE_MAGIC = 0x5053484144455243ULL; // "PSHADERC"
    constexpr uint32_t SHADER_CACHE_FORMAT_VERSION = 0x00000100U; // 0.0.1.0

    constexpr uint64_t SHADER_CACHE_FNV_OFFSET_BASIS = 14695981039346656037ULL;
    constexpr uint64_t SHADER_CACHE_FNV_PRIME = 1099511628211ULL;

    inline static constexpr uint64_t FNV1a64(std::span<uint8_t> data)
    {
        uint64_t hash = SHADER_CACHE_FNV_OFFSET_BASIS;
        for (uint8_t byte : data)
        {
            hash ^= static_cast<uint64_t>(byte);
            hash *= SHADER_CACHE_FNV_PRIME;
        }
        return hash;
    }

    struct ShaderCacheHeader
    {
        uint64_t Magic = SHADER_CACHE_MAGIC;
        uint32_t FormatVersion = SHADER_CACHE_FORMAT_VERSION;
        uint64_t SourceSetFingerprint = 0;
        uint32_t EntryCount = 0;
    };

    struct ShaderCacheIndexEntry
    {
        uint64_t KeyHash = 0;
        uint64_t Offset = 0;
    };


    PrismShaderCache& PrismShaderCache::Get()
    {
        static PrismShaderCache s_Instance;
        s_Instance.m_EntryData.reserve(10 * 1024 * 1024);  // 预留 10MB
        return s_Instance;
    }

    void PrismShaderCache::Init(std::string_view rootPath)
    {
        m_RootPath = std::string(rootPath);
        m_SourceSetFingerprint = GenerateSourceSetFingerprint(rootPath);
        ReadCacheFile("DataCache/PrismShaderData.cache");
        PR_CORE_INFO("PrismShaderCache::Init - root '{}', fingerprint 0x{:016x}", m_RootPath, m_SourceSetFingerprint);
    }

    void PrismShaderCache::Shutdown()
    {
        WriteCacheFile("DataCache/PrismShaderData.cache");
        m_EntryData.clear();
        m_EntryIndex.clear();
    }

    bool PrismShaderCache::WriteCacheFile(std::string_view filePath)
    {
        ShaderCacheHeader header;
        header.SourceSetFingerprint = m_SourceSetFingerprint;
        header.EntryCount = static_cast<uint32_t>(m_EntryIndex.size());

        std::vector<ShaderCacheIndexEntry> indexTable;
        indexTable.reserve(m_EntryIndex.size());
        for (const auto& e : m_EntryIndex)
            indexTable.emplace_back(e.first, e.second);

        std::ofstream out(filePath.data(), std::ios::binary);
        if (!out)
        {
            PR_CORE_ERROR("PrismShaderCache::WriteCacheFile - cannot open '{}'", filePath);
            return false;
        }

        out.write(reinterpret_cast<const char*>(&header), sizeof(header));
        out.write(reinterpret_cast<const char*>(indexTable.data()),
            static_cast<std::streamsize>(indexTable.size() * sizeof(ShaderCacheIndexEntry)));
        out.write(reinterpret_cast<const char*>(m_EntryData.data()),
            static_cast<std::streamsize>(m_EntryData.size() * sizeof(byte)));

        out.close();
        if (!out)
        {
            PR_CORE_ERROR("PrismShaderCache::WriteCacheFile - write failed for '{}'", filePath);
            return false;
        }
        return true;
    }

    bool PrismShaderCache::ReadCacheFile(std::string_view filePath)
    {
        std::ifstream in(filePath.data(), std::ios::binary);
        if (!in)
            return false;

        ShaderCacheHeader header;
        in.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (!in)
            return false;

        if (header.Magic != SHADER_CACHE_MAGIC || header.FormatVersion != SHADER_CACHE_FORMAT_VERSION)
            return false;
        if (header.SourceSetFingerprint != m_SourceSetFingerprint)
            return false;

        std::vector<ShaderCacheIndexEntry> indexTable(header.EntryCount);
        in.read(reinterpret_cast<char*>(indexTable.data()),
            static_cast<std::streamsize>(indexTable.size() * sizeof(ShaderCacheIndexEntry)));

        const auto dataStart = static_cast<size_t>(in.tellg());
        in.seekg(0, std::ios::end);
        const auto dataEnd = static_cast<size_t>(in.tellg());
        in.seekg(static_cast<std::streampos>(dataStart), std::ios::beg);

        m_EntryData.resize(dataEnd - dataStart);
        if (!m_EntryData.empty())
        {
            in.read(reinterpret_cast<char*>(m_EntryData.data()),
                static_cast<std::streamsize>(m_EntryData.size()));
        }

        m_EntryIndex.clear();
        m_EntryIndex.reserve(indexTable.size());
        for (const auto& idx : indexTable)
            m_EntryIndex[idx.KeyHash] = idx.Offset;

        return true;
    }

    void PrismShaderCache::AddEntry(const ShaderCacheEntry& entry)
    {
        if (entry.KeyHash == 0) return;
        uint64_t totalSize = sizeof(entry.KeyHash) + sizeof(entry.Backend)
            + sizeof(uint64_t) + entry.Source_1.size_bytes()
            + sizeof(uint64_t) + entry.Source_2.size_bytes()
            + sizeof(uint64_t) + entry.Reflection.size_bytes();
        totalSize = (totalSize + 7) & ~7ULL;
        uint64_t oldSize = m_EntryData.size();
        m_EntryData.resize(oldSize + totalSize);
        uint8_t* p = m_EntryData.data() + oldSize / sizeof(uint8_t);
        std::memcpy(p, &entry.KeyHash, sizeof(entry.KeyHash)); p += sizeof(entry.KeyHash);
        std::memcpy(p, &entry.Backend, sizeof(entry.Backend)); p += sizeof(entry.Backend);
        uint64_t size = entry.Source_1.size_bytes();
        std::memcpy(p, &size, sizeof(size)); p += sizeof(size);
        std::memcpy(p, entry.Source_1.data(), size); p += size;
        size = entry.Source_2.size_bytes();
        std::memcpy(p, &size, sizeof(size)); p += sizeof(size);
        std::memcpy(p, entry.Source_2.data(), size); p += size;
        size = entry.Reflection.size_bytes();
        std::memcpy(p, &size, sizeof(size)); p += sizeof(size);
        std::memcpy(p, entry.Reflection.data(), size); p += size;
        m_EntryIndex[entry.KeyHash] = oldSize;
    }
    bool PrismShaderCache::FindEntry(uint64_t keyHash, ShaderCacheEntry* outEntry) const
    {
        using namespace PrismShaderCompiler;
        if (!outEntry) return false;
        auto it = m_EntryIndex.find(keyHash);
        if (it == m_EntryIndex.end())
            return false;
        const uint8_t* p = m_EntryData.data() + it->second;
        uint64_t hash = *reinterpret_cast<const uint64_t*>(p); p += sizeof(uint64_t);
        if (keyHash != hash) return false;
        outEntry->KeyHash = keyHash;
        outEntry->Backend = *reinterpret_cast<const uint64_t*>(p); p += sizeof(uint64_t);
        uint64_t size = *reinterpret_cast<const uint64_t*>(p); p += sizeof(uint64_t);
        outEntry->Source_1 = { p, size / sizeof(uint8_t) }; p += size;
        size = *reinterpret_cast<const uint64_t*>(p); p += sizeof(uint64_t);
        outEntry->Source_2 = { p, size / sizeof(uint8_t) }; p += size;
        size = *reinterpret_cast<const uint64_t*>(p); p += sizeof(uint64_t);
        outEntry->Reflection = { reinterpret_cast<const DescriptorInfo*>(p),
            size / sizeof(DescriptorInfo) }; p += size;
        return true;
    }

    uint64_t PrismShaderCache::GenerateSourceSetFingerprint(std::string_view rootPath)
    {
        struct FileRecord
        {
            std::string RelPath;
            uint64_t ContentHash = 0;
        };
        std::vector<FileRecord> records;

        const std::filesystem::path root(rootPath);
        std::error_code ec;
        if (!std::filesystem::exists(root, ec))
            return 0;

        for (auto it = std::filesystem::recursive_directory_iterator(
                 root, std::filesystem::directory_options::skip_permission_denied, ec);
             it != std::filesystem::recursive_directory_iterator(); it.increment(ec))
        {
            if (ec)
                break;
            if (!it->is_regular_file(ec))
                continue;

            const std::string ext = it->path().extension().string();
            if (ext != ".Shader" && ext != ".glsl")
                continue;

            FileRecord rec;
            rec.RelPath = std::filesystem::relative(it->path(), root).string();
            rec.ContentHash = HashFileContent(it->path().string());
            records.push_back(std::move(rec));
        }
        std::sort(records.begin(), records.end(),
            [](const FileRecord& a, const FileRecord& b) { return a.RelPath < b.RelPath; });

        uint64_t h = SHADER_CACHE_FNV_OFFSET_BASIS;
        for (const auto& r : records)
        {
            h ^= Hash::GenerateFNVHash64(r.RelPath);
            h *= SHADER_CACHE_FNV_PRIME;
            h ^= r.ContentHash;
            h *= SHADER_CACHE_FNV_PRIME;
        }

        return h;
    }

    uint64_t PrismShaderCache::HashFileContent(std::string_view filePath)
    {
        std::ifstream in(filePath.data(), std::ios::binary);
        if (!in) return 0;
        std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        return FNV1a64(bytes);
    }

} // namespace Prism
