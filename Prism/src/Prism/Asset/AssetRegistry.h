#pragma once

#include "Prism/Asset/AssetMetadata.h"
#include <unordered_map>

namespace Prism
{

    class AssetRegistry
    {
    public:
        const AssetMetadata& Get(AssetHandle handle) const;
        void Set(AssetHandle handle, const AssetMetadata& metadata);

        size_t Count() const { return m_AssetRegistry.size(); }
        bool Contains(AssetHandle handle) const;
        size_t Remove(AssetHandle handle);
        void Clear();

        auto begin()       { return m_AssetRegistry.begin(); }
        auto end()         { return m_AssetRegistry.end(); }
        auto begin() const { return m_AssetRegistry.cbegin(); }
        auto end() const   { return m_AssetRegistry.cend(); }

    private:
        std::unordered_map<AssetHandle, AssetMetadata> m_AssetRegistry;
    };

}
