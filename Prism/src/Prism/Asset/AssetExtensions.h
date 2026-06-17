#pragma once

#include <unordered_map>
#include "AssetType.h"

namespace Prism
{

    inline static std::unordered_map<std::string, AssetType> s_AssetExtensionMap =
    {
        // Prism native formats
        { ".pscene",  AssetType::Scene },
        { ".pmesh",   AssetType::Mesh },
        { ".pmat",    AssetType::Material },
        { ".pprefab", AssetType::Prefab },
        { ".Shader",  AssetType::Shader },

        { ".cs", AssetType::ScriptFile },

        // DCC source formats → import as MeshSource
        { ".fbx",  AssetType::MeshSource },
        { ".gltf", AssetType::MeshSource },
        { ".glb",  AssetType::MeshSource },
        { ".obj",  AssetType::MeshSource },
        { ".dae",  AssetType::MeshSource },

        // Textures
        { ".png",  AssetType::Texture },
        { ".jpg",  AssetType::Texture },
        { ".jpeg", AssetType::Texture },
        { ".hdr",  AssetType::EnvMap },

        // Fonts
        { ".ttf", AssetType::Font },
        { ".otf", AssetType::Font },

        // Colliders
        { ".pmc", AssetType::MeshCollider },
    };

}
