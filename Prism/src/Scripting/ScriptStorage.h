#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include "Prism/Core/UUID.h"
#include "Scripting/ScriptObject.h"

namespace Prism {

    class PublicField;

    struct ScriptGroup
    {
        UUID EntityID = 0;
        std::string ModuleName;
        uint32_t MethodMask = 0;
        std::unique_ptr<ScriptObject> Instance;

        std::unordered_map<std::string, std::unique_ptr<PublicField>> Fields;
    };

    struct EntityScriptStorage
    {
        std::unordered_map<std::string, ScriptGroup> Groups;  // moduleName -> ScriptGroup
    };

    class SceneScriptStorage
    {
    public:
        ~SceneScriptStorage();
        EntityScriptStorage& GetOrCreateEntity(UUID entityID);
        EntityScriptStorage* FindEntity(UUID entityID);
        const EntityScriptStorage* FindEntity(UUID entityID) const;
        ScriptGroup* FindGroup(UUID entityID, const std::string& moduleName);

        void RemoveEntity(UUID entityID);
        void RemoveGroup(UUID entityID, const std::string& moduleName);

        bool HasEntity(UUID entityID) const;
        bool HasGroup(UUID entityID, const std::string& moduleName) const;

        void CopyGroupData(UUID dstEntity, UUID srcEntity, const std::string& moduleName);
        void CopyGroupDataFrom(const SceneScriptStorage& source, UUID dstEntity, UUID srcEntity, const std::string& moduleName);

        void Clear();

        auto& GetEntities() { return m_Entities; }
        const auto& GetEntities() const { return m_Entities; }

    private:
        std::unordered_map<UUID, EntityScriptStorage> m_Entities;
    };

} // namespace Prism
