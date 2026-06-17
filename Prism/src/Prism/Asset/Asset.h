#pragma once

#include "Prism/Core/UUID.h"
#include "Prism/Asset/AssetType.h"

namespace Prism
{

    using AssetHandle = UUID;

    class Asset : public RefCounted
    {
    public:
        AssetHandle Handle = 0;
        uint16_t Flags = (uint16_t)AssetFlag::None;

        virtual ~Asset() {}

        static AssetType GetStaticType() { return AssetType::None; }
        virtual AssetType GetAssetType() const { return AssetType::None; }

        virtual void OnDependencyUpdated(AssetHandle handle) {}

        virtual bool operator==(const Asset& other) const
        {
            return Handle == other.Handle;
        }
        virtual bool operator!=(const Asset& other) const
        {
            return !(*this == other);
        }

    private:
        friend class EditorAssetManager;
        friend class RuntimeAssetManager;
        friend class AssimpMeshImporter;

        bool IsValid() const
        {
            return ((Flags & (uint16_t)AssetFlag::Missing) | (Flags & (uint16_t)AssetFlag::Invalid)) == 0;
        }
        void SetFlag(AssetFlag flag, bool value = true)
        {
            if (value) Flags |= (uint16_t)flag;
            else       Flags &= ~(uint16_t)flag;
        }
    };

}
