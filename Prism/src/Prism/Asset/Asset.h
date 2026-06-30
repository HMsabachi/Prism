#pragma once

#include "Prism/Core/UUID.h"

namespace Prism {

    enum class AssetType
    {
        Scene, Mesh, Texture, EnvMap, Audio, Script, Shader, Other
    };

    using AssetHandle = size_t;

    class Asset : public RefCounted
    {
    public:
        AssetHandle Handle;
        AssetType Type;

        std::string FilePath;
        std::string FileName;
        std::string Extension;
        int ParentDirectory;

        virtual ~Asset() {}
    };

}
