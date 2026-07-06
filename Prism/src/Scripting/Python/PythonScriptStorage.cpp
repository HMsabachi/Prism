#include "prpch.h"
#include "PythonScriptStorage.h"

namespace Prism
{

    void PythonScriptStorage::Store(UUID scriptID, pybind11::object* obj)
    {
        EntityStorage[scriptID] = { scriptID, obj };
    }

    void PythonScriptStorage::Remove(UUID scriptID)
    {
        EntityStorage.erase(scriptID);
    }

    void PythonScriptStorage::CopyTo(PythonScriptStorage& other) const
    {
        other.EntityStorage = EntityStorage;
    }

    void PythonScriptStorage::CopyEntityStorage(UUID entityID, UUID targetEntityID, PythonScriptStorage& targetStorage) const
    {
        auto it = EntityStorage.find(entityID);
        if (it != EntityStorage.end())
            targetStorage.EntityStorage[targetEntityID] = it->second;
    }

    void PythonScriptStorage::Clear()
    {
        EntityStorage.clear();
    }

}
