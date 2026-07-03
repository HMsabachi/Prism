#pragma once

#include "Asset.h"

namespace Prism {

    class PRISM_API AssetSerializer
    {
    public:
        template<typename T>
        static void SerializeAsset(const Ref<T>& asset)
        {
            static_assert(std::is_base_of<Asset, T>::value, "SerializeAsset only accepts types that inherit from Asset");
            SerializeAsset(asset, asset->Type);
        }

        static Ref<Asset> LoadAssetInfo(const std::string& filepath, AssetHandle parentHandle, AssetType type);
        static Ref<Asset> LoadAssetData(Ref<Asset>& asset);

    private:
        static void SerializeAsset(const Ref<Asset>& asset, AssetType type);
        static Ref<Asset> DeserializeYAML(const Ref<Asset>& asset);
        static void LoadMetaData(Ref<Asset>& asset);
        static void CreateMetaFile(const Ref<Asset>& asset);

        friend class AssetManager;
    };

}
