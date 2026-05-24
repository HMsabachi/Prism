#include "prpch.h"
#include "CSharpScriptStorage.h"

namespace Prism
{

    void CSharpScriptStorage::Store(UUID scriptID, Rolky::ManagedObject* obj)
    {
        EntityStorage[scriptID] = { scriptID, obj };
    }

    void CSharpScriptStorage::Remove(UUID scriptID)
    {
        EntityStorage.erase(scriptID);
    }

    void CSharpScriptStorage::CopyTo(CSharpScriptStorage& other) const
    {
        other.EntityStorage = EntityStorage;
    }

    void CSharpScriptStorage::CopyEntityStorage(UUID entityID, UUID targetEntityID, CSharpScriptStorage& targetStorage) const
    {
        auto it = EntityStorage.find(entityID);
        if (it != EntityStorage.end())
            targetStorage.EntityStorage[targetEntityID] = it->second;
    }

    void CSharpScriptStorage::Clear()
    {
        EntityStorage.clear();
    }

}