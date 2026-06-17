#pragma once

#include "Prism/Asset/AssetManager/AssetManagerBase.h"
#include "Prism/Asset/AssetRegistry.h"
#include <shared_mutex>

namespace Prism
{

    class EditorAssetManager : public AssetManagerBase
    {
    public:
        EditorAssetManager();
        ~EditorAssetManager() override;

        void SetProjectPath(const std::filesystem::path& projectPath);
        const std::filesystem::path& GetAssetPath() const { return m_AssetPath; }

        // AssetManagerBase interface
        void Shutdown() override;

        AssetType GetAssetType(AssetHandle handle) override;
        Ref<Asset> GetAsset(AssetHandle handle) override;
        void GetAssetAsync(AssetHandle handle) override;

        void AddMemoryOnlyAsset(Ref<Asset> asset) override;
        bool ReloadData(AssetHandle handle) override;
        bool IsAssetHandleValid(AssetHandle handle) override;
        bool IsAssetLoaded(AssetHandle handle) override;
        Ref<Asset> GetMemoryAsset(AssetHandle handle) override;
        bool IsMemoryAsset(AssetHandle handle) override;

        void RegisterDependency(AssetHandle handle, AssetHandle dependency) override;
        void SyncWithAssetThread() override;

        std::unordered_set<AssetHandle> GetAllAssetsWithType(AssetType type) override;
        const std::unordered_map<AssetHandle, Ref<Asset>>& GetLoadedAssets() override;

        // Editor-specific
        AssetHandle ImportAsset(const std::string& filepath);
        void ProcessDirectory(const std::string& directory);
        void ReloadRegistry();

        AssetMetadata GetMetadata(AssetHandle handle);
        void SetMetadata(AssetHandle handle, const AssetMetadata& metadata);

    private:
        void LoadAssetRegistry();
        void WriteRegistryToFile();

        AssetType GetAssetTypeFromPath(const std::filesystem::path& filepath);
        std::filesystem::path GetFileSystemPath(const AssetMetadata& metadata);

        std::filesystem::path m_ProjectPath;
        std::filesystem::path m_AssetPath;

        AssetRegistry m_AssetRegistry;
        std::shared_mutex m_AssetRegistryMutex;

        std::unordered_map<AssetHandle, Ref<Asset>> m_LoadedAssets;
        std::unordered_map<AssetHandle, Ref<Asset>> m_MemoryAssets;
        std::shared_mutex m_MemoryAssetsMutex;

        std::unordered_map<AssetHandle, std::unordered_set<AssetHandle>> m_AssetDependencies;
        std::shared_mutex m_AssetDependenciesMutex;
    };

}
