#include "prpch.h"
#include "AssetImporter.h"

namespace Prism {

    void AssetImporter::Init()
    {
        s_Serializers[AssetType::Texture] = CreateScope<TextureSerializer>();
        s_Serializers[AssetType::Mesh] = CreateScope<MeshSerializer>();
        s_Serializers[AssetType::EnvMap] = CreateScope<EnvironmentSerializer>();
        s_Serializers[AssetType::PhysicsMat] = CreateScope<PhysicsMaterialSerializer>();
        // Prism 适配：Shader 资源走 ShaderLibrary（PSL）
        s_Serializers[AssetType::Shader] = CreateScope<ShaderSerializer>();
    }

    void AssetImporter::Serialize(const Ref<Asset>& asset)
    {
        if (s_Serializers.find(asset->Type) == s_Serializers.end())
        {
            PR_CORE_WARN("There's currently no importer for assets of type {0}", asset->Extension);
            return;
        }

        s_Serializers[asset->Type]->Serialize(asset);
    }

    bool AssetImporter::TryLoadData(Ref<Asset>& asset)
    {
        if (asset->Type == AssetType::Directory)
            return false;

        if (s_Serializers.find(asset->Type) == s_Serializers.end())
        {
            PR_CORE_WARN("There's currently no importer for assets of type {0}", asset->Extension);
            return false;
        }

        return s_Serializers[asset->Type]->TryLoadData(asset);
    }

    std::unordered_map<AssetType, Scope<AssetSerializer>> AssetImporter::s_Serializers;

}
