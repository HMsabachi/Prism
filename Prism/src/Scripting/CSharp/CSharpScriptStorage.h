#pragma once
#include "Prism/Core/UUID.h"
#include <Rolky/ManagedObject.hpp>
#include <unordered_map>

namespace Prism {

class Entity;

struct CSharpEntityScriptStorage
{
    UUID ScriptID;
    Rolky::ManagedObject Instance;
};

struct CSharpScriptStorage
{
    std::unordered_map<UUID, CSharpEntityScriptStorage> EntityStorage;

    void Store(UUID scriptID, Rolky::ManagedObject obj);
    void Remove(UUID scriptID);
    void CopyTo(CSharpScriptStorage& other) const;
    void CopyEntityStorage(UUID entityID, UUID targetEntityID, CSharpScriptStorage& targetStorage) const;
    void Clear();
};

}