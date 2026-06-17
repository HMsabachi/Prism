#pragma once

#include "Prism/Asset/Asset.h"
#include <unordered_map>
#include <unordered_set>

namespace Prism
{

    class AssetManagerBase : public RefCounted
    {
    public:
        virtual ~AssetManagerBase() = default;

        virtual void Shutdown() = 0;

        virtual AssetType GetAssetType(AssetHandle handle) = 0;
        virtual Ref<Asset> GetAsset(AssetHandle handle) = 0;
        virtual void GetAssetAsync(AssetHandle handle) = 0;

        virtual void AddMemoryOnlyAsset(Ref<Asset> asset) = 0;
        virtual bool ReloadData(AssetHandle handle) = 0;
        virtual bool IsAssetHandleValid(AssetHandle handle) = 0;
        virtual bool IsAssetLoaded(AssetHandle handle) = 0;
        virtual Ref<Asset> GetMemoryAsset(AssetHandle handle) = 0;
        virtual bool IsMemoryAsset(AssetHandle handle) = 0;

        virtual void RegisterDependency(AssetHandle handle, AssetHandle dependency) = 0;
        virtual void SyncWithAssetThread() = 0;

        virtual std::unordered_set<AssetHandle> GetAllAssetsWithType(AssetType type) = 0;
        virtual const std::unordered_map<AssetHandle, Ref<Asset>>& GetLoadedAssets() = 0;
    };

}
