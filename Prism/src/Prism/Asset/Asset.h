#pragma once

#include "Prism/Core/UUID.h"

namespace Prism {

    enum class AssetType
    {
        Scene, Mesh, Texture, EnvMap, Audio, Script, PhysicsMat, Shader, Directory, Other, None
    };

    using AssetHandle = UUID;

    class PRISM_API Asset : public RefCounted
    {
    public:
        AssetHandle Handle;
        AssetType Type = AssetType::None;

        std::string FilePath;
        std::string FileName;
        std::string Extension;
        AssetHandle ParentDirectory{};
        bool IsDataLoaded = false;

        virtual ~Asset() {}
    };

    class PhysicsMaterial : public Asset
    {
    public:
        float StaticFriction{};
        float DynamicFriction{};
        float Bounciness{};

        PhysicsMaterial() = default;
        PhysicsMaterial(float staticFriction, float dynamicFriction, float bounciness)
            : StaticFriction(staticFriction), DynamicFriction(dynamicFriction), Bounciness(bounciness)
        {
        }
    };

    class Directory : public Asset
    {
    public:
        std::vector<AssetHandle> ChildDirectories;

        Directory() = default;
    };

}
