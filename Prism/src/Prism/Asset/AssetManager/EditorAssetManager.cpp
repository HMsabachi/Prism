#include "prpch.h"
#include "EditorAssetManager.h"

#include "Prism/Asset/AssetExtensions.h"
#include "Prism/Asset/AssetManager/AssetManager.h"
#include "Prism/Utilities/FileSystem.h"
#include "Prism/Utilities/Utilities.h"

#include "yaml-cpp/yaml.h"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace Prism
{

    EditorAssetManager::EditorAssetManager()
    {
    }

    EditorAssetManager::~EditorAssetManager()
    {
        Shutdown();
    }

    void EditorAssetManager::SetProjectPath(const std::filesystem::path& projectPath)
    {
        m_ProjectPath = projectPath;
        m_AssetPath = projectPath / "Assets";
        LoadAssetRegistry();
        ProcessDirectory(m_AssetPath.string());
    }

    void EditorAssetManager::Shutdown()
    {
        WriteRegistryToFile();
        m_LoadedAssets.clear();
        m_MemoryAssets.clear();
        m_AssetRegistry.Clear();
    }

    // ==================== Registry I/O ====================

    void EditorAssetManager::LoadAssetRegistry()
    {
        auto registryPath = m_ProjectPath / "AssetRegistry.preg";
        if (!std::filesystem::exists(registryPath))
            return;

        std::ifstream stream(registryPath);
        if (!stream.is_open())
        {
            PR_CORE_ERROR("Failed to open asset registry: {}", registryPath.string());
            return;
        }

        std::stringstream strStream;
        strStream << stream.rdbuf();
        stream.close();

        YAML::Node data = YAML::Load(strStream.str());
        auto assets = data["Assets"];
        if (!assets)
        {
            PR_CORE_WARN("Asset registry is empty or malformed");
            return;
        }

        for (auto entry : assets)
        {
            std::string filepath = entry["FilePath"].as<std::string>();

            AssetMetadata metadata;
            metadata.Handle = entry["Handle"].as<uint64_t>();
            metadata.FilePath = filepath;
            metadata.Type = AssetUtils::AssetTypeFromString(entry["Type"].as<std::string>());
            metadata.IsDataLoaded = false;

            if (metadata.Type == AssetType::None)
                continue;

            if (metadata.Handle == 0)
                continue;

            std::unique_lock lock(m_AssetRegistryMutex);
            m_AssetRegistry.Set(metadata.Handle, metadata);
        }

        PR_CORE_INFO("Loaded asset registry with {} entries", m_AssetRegistry.Count());
    }

    void EditorAssetManager::WriteRegistryToFile()
    {
        struct AssetRegistryEntry
        {
            std::string FilePath;
            AssetType Type;
        };
        std::map<UUID, AssetRegistryEntry> sortedMap;

        {
            std::shared_lock lock(m_AssetRegistryMutex);
            for (auto& [handle, metadata] : m_AssetRegistry)
            {
                auto fullPath = GetFileSystemPath(metadata);
                if (!std::filesystem::exists(fullPath))
                    continue;

                std::string pathToSerialize = metadata.FilePath.string();
                std::replace(pathToSerialize.begin(), pathToSerialize.end(), '\\', '/');
                sortedMap[metadata.Handle] = { pathToSerialize, metadata.Type };
            }
        }

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Assets" << YAML::BeginSeq;
        for (auto& [handle, entry] : sortedMap)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Handle" << YAML::Value << handle;
            out << YAML::Key << "FilePath" << YAML::Value << entry.FilePath;
            out << YAML::Key << "Type" << YAML::Value << AssetUtils::AssetTypeToString(entry.Type);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;

        auto registryPath = m_ProjectPath / "AssetRegistry.preg";
        std::ofstream fout(registryPath);
        if (fout.is_open())
        {
            fout << out.c_str();
            fout.close();
        }
    }

    // ==================== Import ====================

    AssetType EditorAssetManager::GetAssetTypeFromPath(const std::filesystem::path& filepath)
    {
        auto ext = filepath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        auto it = s_AssetExtensionMap.find(ext);
        if (it != s_AssetExtensionMap.end())
            return it->second;
        return AssetType::None;
    }

    AssetHandle EditorAssetManager::ImportAsset(const std::string& filepath)
    {
        std::filesystem::path absPath = std::filesystem::absolute(filepath);
        std::filesystem::path relativePath = std::filesystem::relative(absPath, m_AssetPath);

        AssetType type = GetAssetTypeFromPath(absPath);
        if (type == AssetType::None)
        {
            PR_CORE_WARN("Unknown asset type for: {}", filepath);
            return 0;
        }

        // Check if already registered
        {
            std::shared_lock lock(m_AssetRegistryMutex);
            for (auto& [handle, metadata] : m_AssetRegistry)
            {
                if (metadata.FilePath == relativePath)
                {
                    return handle;
                }
            }
        }

        AssetMetadata metadata;
        metadata.Handle = UUID();
        metadata.Type = type;
        metadata.FilePath = relativePath;
        metadata.Status = AssetStatus::Ready;
        metadata.FileLastWriteTime = std::filesystem::last_write_time(absPath).time_since_epoch().count();
        metadata.IsDataLoaded = false;

        {
            std::unique_lock lock(m_AssetRegistryMutex);
            m_AssetRegistry.Set(metadata.Handle, metadata);
        }

        PR_CORE_INFO("Imported: {} → {}", relativePath.string(), (uint64_t)metadata.Handle);
        return metadata.Handle;
    }

    void EditorAssetManager::ProcessDirectory(const std::string& directory)
    {
        if (!std::filesystem::exists(directory))
            return;

        for (auto& entry : std::filesystem::recursive_directory_iterator(directory))
        {
            if (!entry.is_regular_file())
                continue;

            auto& path = entry.path();
            AssetType type = GetAssetTypeFromPath(path);
            if (type == AssetType::None)
                continue;

            std::filesystem::path relativePath = std::filesystem::relative(path, m_AssetPath);

            // Check if already registered
            bool found = false;
            {
                std::shared_lock lock(m_AssetRegistryMutex);
                for (auto& [handle, metadata] : m_AssetRegistry)
                {
                    if (metadata.FilePath == relativePath)
                    {
                        found = true;
                        break;
                    }
                }
            }

            if (!found)
                ImportAsset(path.string());
        }

        WriteRegistryToFile();
    }

    void EditorAssetManager::ReloadRegistry()
    {
        m_AssetRegistry.Clear();
        LoadAssetRegistry();
        ProcessDirectory(m_AssetPath.string());
    }

    // ==================== Metadata access ====================

    AssetMetadata EditorAssetManager::GetMetadata(AssetHandle handle)
    {
        std::shared_lock lock(m_AssetRegistryMutex);
        return m_AssetRegistry.Get(handle);
    }

    void EditorAssetManager::SetMetadata(AssetHandle handle, const AssetMetadata& metadata)
    {
        std::unique_lock lock(m_AssetRegistryMutex);
        m_AssetRegistry.Set(handle, metadata);
    }

    std::filesystem::path EditorAssetManager::GetFileSystemPath(const AssetMetadata& metadata)
    {
        return m_AssetPath / metadata.FilePath;
    }

    // ==================== Asset loading ====================

    AssetType EditorAssetManager::GetAssetType(AssetHandle handle)
    {
        return GetMetadata(handle).Type;
    }

    Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle)
    {
        if (handle == 0)
            return nullptr;

        // Memory-only asset
        {
            std::shared_lock lock(m_MemoryAssetsMutex);
            auto it = m_MemoryAssets.find(handle);
            if (it != m_MemoryAssets.end())
                return it->second;
        }

        // Already loaded
        {
            auto it = m_LoadedAssets.find(handle);
            if (it != m_LoadedAssets.end())
                return it->second;
        }

        // Load from disk
        auto metadata = GetMetadata(handle);
        if (!metadata.IsValid())
            return nullptr;

        auto fullPath = GetFileSystemPath(metadata);

        // Asset loading will be implemented by each Asset subclass
        PR_CORE_WARN("Asset loading not yet implemented for type: {}",
            AssetUtils::AssetTypeToString(metadata.Type));
        return nullptr;
    }

    void EditorAssetManager::GetAssetAsync(AssetHandle handle)
    {
        // Stub — background loading thread in future phase
        GetAsset(handle);
    }

    bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle)
    {
        if (handle == 0) return false;
        std::shared_lock lock(m_MemoryAssetsMutex);
        if (m_MemoryAssets.find(handle) != m_MemoryAssets.end())
            return true;
        std::shared_lock lock2(m_AssetRegistryMutex);
        return m_AssetRegistry.Contains(handle);
    }

    bool EditorAssetManager::IsAssetLoaded(AssetHandle handle)
    {
        return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
    }

    void EditorAssetManager::AddMemoryOnlyAsset(Ref<Asset> asset)
    {
        std::unique_lock lock(m_MemoryAssetsMutex);
        m_MemoryAssets[asset->Handle] = asset;
    }

    bool EditorAssetManager::ReloadData(AssetHandle handle)
    {
        auto it = m_LoadedAssets.find(handle);
        if (it != m_LoadedAssets.end())
            m_LoadedAssets.erase(it);
        return GetAsset(handle) != nullptr;
    }

    Ref<Asset> EditorAssetManager::GetMemoryAsset(AssetHandle handle)
    {
        std::shared_lock lock(m_MemoryAssetsMutex);
        auto it = m_MemoryAssets.find(handle);
        return it != m_MemoryAssets.end() ? it->second : nullptr;
    }

    bool EditorAssetManager::IsMemoryAsset(AssetHandle handle)
    {
        std::shared_lock lock(m_MemoryAssetsMutex);
        return m_MemoryAssets.find(handle) != m_MemoryAssets.end();
    }

    // ==================== Dependencies ====================

    void EditorAssetManager::RegisterDependency(AssetHandle handle, AssetHandle dependency)
    {
        std::unique_lock lock(m_AssetDependenciesMutex);
        m_AssetDependencies[handle].insert(dependency);
    }

    void EditorAssetManager::SyncWithAssetThread()
    {
        // Stub — will be implemented with background thread
    }

    std::unordered_set<AssetHandle> EditorAssetManager::GetAllAssetsWithType(AssetType type)
    {
        std::unordered_set<AssetHandle> result;
        std::shared_lock lock(m_AssetRegistryMutex);
        for (auto& [handle, metadata] : m_AssetRegistry)
        {
            if (metadata.Type == type)
                result.insert(handle);
        }
        return result;
    }

    const std::unordered_map<AssetHandle, Ref<Asset>>& EditorAssetManager::GetLoadedAssets()
    {
        return m_LoadedAssets;
    }

}
