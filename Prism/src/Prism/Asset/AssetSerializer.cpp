#include "prpch.h"
#include "AssetSerializer.h"
#include "Prism/Asset/AssetManager.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/Renderer.h"
#include "Prism/Renderer/SceneEnvironment.h"
#include "Prism/Renderer/Shader/PrismShader.h"
#include "ModelImporter.h"

#include "yaml-cpp/yaml.h"

namespace Prism {

    void AssetSerializer::CopyMetadata(const Ref<Asset>& from, Ref<Asset>& to) const
    {
        to->Handle = from->Handle;
        to->FilePath = from->FilePath;
        to->FileName = from->FileName;
        to->Extension = from->Extension;
        to->ParentDirectory = from->ParentDirectory;
        to->Type = from->Type;
        to->IsDataLoaded = true;
    }

    bool TextureSerializer::TryLoadData(Ref<Asset>& asset) const
    {
        Ref<Asset> temp = asset;
        asset = Texture2D::Create(asset->FilePath);
        CopyMetadata(temp, asset);
        return asset.As<Texture2D>()->Loaded();
    }

    bool MeshSerializer::TryLoadData(Ref<Asset>& asset) const
    {
        Ref<Asset> temp = asset;
        auto result = ModelImporter::Import(temp->FilePath);
        if (result.Mesh)
        {
            asset = result.Mesh;
            CopyMetadata(temp, asset);
            return true;
        }
        return false;
    }

    bool EnvironmentSerializer::TryLoadData(Ref<Asset>& asset) const
    {
        auto [radiance, irradiance] = Renderer::GetAPI()->CreateEnvironmentMap(asset->FilePath);
        if (!radiance || !irradiance)
            return false;

        Ref<Asset> temp = asset;
        asset = Ref<Environment>::Create(radiance, irradiance);
        CopyMetadata(temp, asset);
        return true;
    }

    void PhysicsMaterialSerializer::Serialize(const Ref<Asset>& asset) const
    {
        Ref<PhysicsMaterial> material = Ref<PhysicsMaterial>(asset);

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "StaticFriction" << material->StaticFriction;
        out << YAML::Key << "DynamicFriction" << material->DynamicFriction;
        out << YAML::Key << "Bounciness" << material->Bounciness;
        out << YAML::EndMap;

        std::ofstream fout(asset->FilePath);
        fout << out.c_str();
    }

    bool PhysicsMaterialSerializer::TryLoadData(Ref<Asset>& asset) const
    {
        std::ifstream stream(asset->FilePath);
        if (!stream.is_open())
            return false;

        Ref<Asset> temp = asset;
        std::stringstream strStream;
        strStream << stream.rdbuf();

        YAML::Node data = YAML::Load(strStream.str());

        float staticFriction = data["StaticFriction"].as<float>();
        float dynamicFriction = data["DynamicFriction"].as<float>();
        float bounciness = data["Bounciness"].as<float>();

        asset = Ref<PhysicsMaterial>::Create(staticFriction, dynamicFriction, bounciness);
        CopyMetadata(temp, asset);
        return true;
    }

    bool ShaderSerializer::TryLoadData(Ref<Asset>& asset) const
    {
        Ref<Asset> temp = asset;
        asset = AssetManager::GetShaderLibrary()->Load(temp->FilePath);
        if (!asset)
            return false;
        CopyMetadata(temp, asset);
        return true;
    }

}
