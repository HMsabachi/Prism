#pragma once

#include "Prism/Core/Core.h"
#include <PrismShaderCore/Generator/ReflectionGenerator.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <span>
#include <vector>

namespace Prism
{

    struct ShaderCacheEntry
    {
        uint64_t KeyHash = 0;
        uint8_t Backend = 0;
        std::span<const uint8_t> Source_1;
        std::span<const uint8_t> Source_2;
        std::span<const PrismShaderCompiler::DescriptorInfo> Reflection;
    };

    class PrismShaderCache
    {
    public:
        static PrismShaderCache& Get();

        void Init(std::string_view rootPath);
        void Shutdown();

        void AddEntry(const ShaderCacheEntry& entry);
        bool FindEntry(uint64_t keyHash, ShaderCacheEntry* outEntry) const;

    private:
        bool ReadCacheFile(std::string_view filePath);
        bool WriteCacheFile(std::string_view filePath);
        uint64_t GenerateSourceSetFingerprint(std::string_view rootPath);
        uint64_t HashFileContent(std::string_view filePath);

    private:
        std::string m_RootPath;
        uint64_t m_SourceSetFingerprint = 0;

        std::vector<byte> m_EntryData;
        std::unordered_map<uint64_t, uint64_t> m_EntryIndex;
    };

} // namespace Prism
