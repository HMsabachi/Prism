#pragma once

#include "Prism/Core/Core.h"

namespace Prism
{

    enum class AssetFlag : uint16_t
    {
        None    = 0,
        Missing = BIT(0),
        Invalid = BIT(1)
    };

    enum class AssetType : uint16_t
    {
        None = 0,
        Scene,
        Prefab,
        Mesh,
        StaticMesh,
        MeshSource,
        Shader,
        Material,
        Texture,
        EnvMap,
        Font,
        ScriptFile,
        MeshCollider
    };

    namespace AssetUtils
    {
        AssetType AssetTypeFromString(std::string_view assetType);
        const char* AssetTypeToString(AssetType assetType);
    }

}
