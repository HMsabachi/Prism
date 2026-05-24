#pragma once
#include "Prism/Core/UUID.h"
#include "Scripting/Python/PythonScriptCore.h"
#include <unordered_map>

namespace Prism {

class Entity;

struct PythonEntityScriptStorage
{
    UUID ScriptID;
    Python::ScriptObject Instance;
};

struct PythonScriptStorage
{
    std::unordered_map<UUID, PythonEntityScriptStorage> EntityStorage;

    void Store(UUID scriptID, Python::ScriptObject obj);
    void Remove(UUID scriptID);
    void CopyTo(PythonScriptStorage& other) const;
    void CopyEntityStorage(UUID entityID, UUID targetEntityID, PythonScriptStorage& targetStorage) const;
    void Clear();
};

}