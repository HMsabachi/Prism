#pragma once

#include "Prism/Asset/Asset.h"
#include "Prism/Asset/AssetManager/AssetManagerBase.h"

namespace Prism
{

    class PRISM_API AssetManager
    {
    public:
        template<typename T>
        static Ref<T> GetAsset(AssetHandle handle)
        {
            Ref<Asset> asset = GetAsset(handle);
            return asset.As<T>();
        }

        static Ref<Asset> GetAsset(AssetHandle handle)
        {
            return GetAssetManager()->GetAsset(handle);
        }

        static AssetType GetAssetType(AssetHandle handle)
        {
            return GetAssetManager()->GetAssetType(handle);
        }

        static bool IsAssetHandleValid(AssetHandle handle)
        {
            return GetAssetManager()->IsAssetHandleValid(handle);
        }

        static bool IsAssetLoaded(AssetHandle handle)
        {
            return GetAssetManager()->IsAssetLoaded(handle);
        }

        static AssetHandle ImportAsset(const std::string& filepath);

        static void AddMemoryOnlyAsset(Ref<Asset> asset)
        {
            GetAssetManager()->AddMemoryOnlyAsset(asset);
        }

        static void SetAssetManager(Ref<AssetManagerBase> manager);
        static Ref<AssetManagerBase> GetAssetManager();

    private:
        static Ref<AssetManagerBase> s_AssetManager;
    };

}
