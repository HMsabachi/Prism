#pragma once
#include "Prism/Core/UUID.h"
#include <unordered_map>

namespace pybind11
{
    class object;
}

namespace Prism {

class Entity;

struct PythonEntityScriptStorage
{
    UUID ScriptID;
    pybind11::object* Instance = nullptr;
};

struct PythonScriptStorage
{
    std::unordered_map<UUID, PythonEntityScriptStorage> EntityStorage;

    void Store(UUID scriptID, pybind11::object* obj);
    void Remove(UUID scriptID);
    void CopyTo(PythonScriptStorage& other) const;
    void CopyEntityStorage(UUID entityID, UUID targetEntityID, PythonScriptStorage& targetStorage) const;
    void Clear();
};

}
