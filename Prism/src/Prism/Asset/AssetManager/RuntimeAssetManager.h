#pragma once

#include "Prism/Asset/AssetManager/AssetManagerBase.h"

namespace Prism
{

    class RuntimeAssetManager : public AssetManagerBase
    {
    public:
        RuntimeAssetManager();
        ~RuntimeAssetManager() override;

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

    private:
        std::unordered_map<AssetHandle, Ref<Asset>> m_LoadedAssets;
    };

}
