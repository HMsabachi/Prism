#include "prpch.h"
#include "ScriptStorage.h"
#include "Scripting/PublicField.h"

namespace Prism {

    SceneScriptStorage::~SceneScriptStorage() = default;

    EntityScriptStorage& SceneScriptStorage::GetOrCreateEntity(UUID entityID)
    {
        return m_Entities[entityID];
    }

    EntityScriptStorage* SceneScriptStorage::FindEntity(UUID entityID)
    {
        auto it = m_Entities.find(entityID);
        return it != m_Entities.end() ? &it->second : nullptr;
    }

    const EntityScriptStorage* SceneScriptStorage::FindEntity(UUID entityID) const
    {
        auto it = m_Entities.find(entityID);
        return it != m_Entities.end() ? &it->second : nullptr;
    }

    ScriptGroup* SceneScriptStorage::FindGroup(UUID entityID, const std::string& moduleName)
    {
        auto* entity = FindEntity(entityID);
        if (!entity) return nullptr;
        auto git = entity->Groups.find(moduleName);
        return git != entity->Groups.end() ? &git->second : nullptr;
    }

    void SceneScriptStorage::RemoveEntity(UUID entityID)
    {
        m_Entities.erase(entityID);
    }

    void SceneScriptStorage::RemoveGroup(UUID entityID, const std::string& moduleName)
    {
        auto* entity = FindEntity(entityID);
        if (entity)
            entity->Groups.erase(moduleName);
    }

    bool SceneScriptStorage::HasEntity(UUID entityID) const
    {
        return m_Entities.find(entityID) != m_Entities.end();
    }

    bool SceneScriptStorage::HasGroup(UUID entityID, const std::string& moduleName) const
    {
        auto* entity = FindEntity(entityID);
        return entity && entity->Groups.find(moduleName) != entity->Groups.end();
    }

    void SceneScriptStorage::CopyGroupData(UUID dstEntity, UUID srcEntity, const std::string& moduleName)
    {
        auto* srcGroup = FindGroup(srcEntity, moduleName);
        if (!srcGroup) return;

        auto& dst = GetOrCreateEntity(dstEntity);
        auto& dstGroup = dst.Groups[moduleName];
        // Copy field stored values if the destination group already has matching fields
        for (auto& [name, srcField] : srcGroup->Fields)
        {
            auto dit = dstGroup.Fields.find(name);
            if (dit != dstGroup.Fields.end())
                dit->second->SetStoredValueRaw(srcField->GetStoredValueBuffer());
        }
    }

    void SceneScriptStorage::CopyGroupDataFrom(const SceneScriptStorage& source, UUID dstEntity, UUID srcEntity, const std::string& moduleName)
    {
        const auto* srcEntityStorage = source.FindEntity(srcEntity);
        if (!srcEntityStorage) return;

        auto sit = srcEntityStorage->Groups.find(moduleName);
        if (sit == srcEntityStorage->Groups.end()) return;
        const auto& srcGroup = sit->second;

        auto& dst = GetOrCreateEntity(dstEntity);
        auto& dstGroup = dst.Groups[moduleName];
        for (auto& [name, srcField] : srcGroup.Fields)
        {
            auto dit = dstGroup.Fields.find(name);
            if (dit != dstGroup.Fields.end())
                dit->second->SetStoredValueRaw(srcField->GetStoredValueBuffer());
        }
    }

    void SceneScriptStorage::Clear()
    {
        m_Entities.clear();
    }

} // namespace Prism
