#include "prpch.h"
#include "AssetType.h"

namespace Prism::AssetUtils
{

    AssetType AssetTypeFromString(std::string_view assetType)
    {
        if (assetType == "Scene")         return AssetType::Scene;
        if (assetType == "Prefab")        return AssetType::Prefab;
        if (assetType == "Mesh")          return AssetType::Mesh;
        if (assetType == "StaticMesh")    return AssetType::StaticMesh;
        if (assetType == "MeshSource")    return AssetType::MeshSource;
        if (assetType == "Shader")        return AssetType::Shader;
        if (assetType == "Material")      return AssetType::Material;
        if (assetType == "Texture")       return AssetType::Texture;
        if (assetType == "EnvMap")        return AssetType::EnvMap;
        if (assetType == "Font")          return AssetType::Font;
        if (assetType == "ScriptFile")    return AssetType::ScriptFile;
        if (assetType == "MeshCollider")  return AssetType::MeshCollider;
        return AssetType::None;
    }

    const char* AssetTypeToString(AssetType assetType)
    {
        switch (assetType)
        {
        case AssetType::Scene:        return "Scene";
        case AssetType::Prefab:       return "Prefab";
        case AssetType::Mesh:         return "Mesh";
        case AssetType::StaticMesh:   return "StaticMesh";
        case AssetType::MeshSource:   return "MeshSource";
        case AssetType::Shader:       return "Shader";
        case AssetType::Material:     return "Material";
        case AssetType::Texture:      return "Texture";
        case AssetType::EnvMap:       return "EnvMap";
        case AssetType::Font:         return "Font";
        case AssetType::ScriptFile:   return "ScriptFile";
        case AssetType::MeshCollider: return "MeshCollider";
        default:                      return "None";
        }
    }

}
