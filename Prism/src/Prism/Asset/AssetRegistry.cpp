#include "prpch.h"
#include "AssetRegistry.h"

namespace Prism
{

    const AssetMetadata& AssetRegistry::Get(AssetHandle handle) const
    {
        PR_CORE_ASSERT(m_AssetRegistry.find(handle) != m_AssetRegistry.end());
        return m_AssetRegistry.at(handle);
    }

    void AssetRegistry::Set(AssetHandle handle, const AssetMetadata& metadata)
    {
        PR_CORE_ASSERT(metadata.Handle == handle);
        PR_CORE_ASSERT(handle != 0);
        m_AssetRegistry[handle] = metadata;
    }

    bool AssetRegistry::Contains(AssetHandle handle) const
    {
        return m_AssetRegistry.find(handle) != m_AssetRegistry.end();
    }

    size_t AssetRegistry::Remove(AssetHandle handle)
    {
        return m_AssetRegistry.erase(handle);
    }

    void AssetRegistry::Clear()
    {
        m_AssetRegistry.clear();
    }

}
