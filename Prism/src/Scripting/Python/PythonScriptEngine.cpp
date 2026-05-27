#include "prpch.h"
#include "PythonScriptEngine.h"
#include "PythonScriptEngineRegistry.h"
#include "PythonScriptStorage.h"
#include "PythonScriptMetaRegistry.h"

#include <Python.h>

#include "Prism/Scene/Scene.h"

#include <filesystem>
#include <algorithm>
#include <imgui.h>

namespace Prism
{
    // Static members
    WeakRef<Scene> PythonScriptEngine::s_SceneContext;
    bool PythonScriptEngine::s_Initialized = false;
    std::unordered_map<UUID, std::unordered_map<UUID, Python::ScriptObject>> PythonScriptEngine::s_PythonScriptObjects;

    void PythonScriptEngine::Initialize()
    {
        PR_PROFILE_FUNCTION();

        if (s_Initialized)
            return;

        if (!Python::ScriptHost::Initialize())
        {
            PR_CORE_ERROR("[Python] 初始化解释器失败！");
            return;
        }

        PythonScriptEngineRegistry::RegisterAll();

        s_Initialized = true;
        PR_CORE_TRACE("[Python] Python 运行时已初始化");

        PythonScriptMetaRegistry::BuildCache();
    }

    void PythonScriptEngine::Shutdown()
    {
        PR_PROFILE_FUNCTION();

        if (!s_Initialized)
            return;

        PythonScriptMetaRegistry::Shutdown();
        Python::ScriptHost::Shutdown();
        s_Initialized = false;
        s_SceneContext = nullptr;
        PR_CORE_INFO("[Python] Python 解释器已关闭");
    }

    PythonEntityScriptStorage& PythonScriptEngine::GetEntityScriptStorage(PythonScriptStorage& storage, UUID scriptID)
    {
        auto it = storage.EntityStorage.find(scriptID);
        PR_CORE_ASSERT(it != storage.EntityStorage.end(), "PythonScript entity not found!");
        return it->second;
    }

    void PythonScriptEngine::SetSceneContext(const WeakRef<Scene>& scene)
    {
        s_SceneContext = scene;
    }

    const WeakRef<Scene>& PythonScriptEngine::GetCurrentSceneContext()
    {
        return s_SceneContext;
    }

    Python::ScriptObject* PythonScriptEngine::GetScriptObject(UUID sceneID, UUID scriptID)
    {
        auto sceneIt = s_PythonScriptObjects.find(sceneID);
        if (sceneIt == s_PythonScriptObjects.end())
            return nullptr;
        auto objIt = sceneIt->second.find(scriptID);
        return objIt != sceneIt->second.end() ? &objIt->second : nullptr;
    }

    void PythonScriptEngine::RemoveScriptObject(PythonScriptStorage& storage, UUID scriptID)
    {
        auto sceneID = s_SceneContext ? s_SceneContext->GetUUID() : UUID(0);
        PR_CORE_ASSERT(sceneID, "没有场景上下文");

        auto sceneIt = s_PythonScriptObjects.find(sceneID);
        if (sceneIt != s_PythonScriptObjects.end())
        {
            sceneIt->second.erase(scriptID);
            if (sceneIt->second.empty())
                s_PythonScriptObjects.erase(sceneIt);
            storage.Remove(scriptID);
        }
    }

    UUID PythonScriptEngine::AddBehaviour(Entity& entity, PythonBehaviourBinding& binding)
    {
        UUID sceneID = s_SceneContext ? s_SceneContext->GetUUID() : UUID(0);
        PR_CORE_ASSERT(sceneID, "没有场景上下文");

        UUID entityID = entity.GetUUID();

        auto* meta = PythonScriptMetaRegistry::GetClassMetadata(binding.ClassID);
        if (!meta)
        {
            PR_CORE_ERROR("[Python] Cannot add behaviour: class metadata not found for ClassID {0}", (uint64_t)binding.ClassID);
            return 0;
        }

        Python::ScriptModule mod = Python::ScriptModule::Import(meta->ModuleName.c_str());
        if (!mod.IsValid())
        {
            PR_CORE_ERROR("[Python] Cannot add behaviour: module not found {0}", meta->ModuleName);
            return 0;
        }

        Python::ScriptClass cls = Python::ScriptClass::From(mod, meta->ClassName.c_str());
        if (!cls.IsValid())
        {
            PR_CORE_ERROR("[Python] Cannot add behaviour: class {0} not found in {1}", meta->ClassName, meta->ModuleName);
            return 0;
        }

        Python::ScriptObject obj = cls.CreateInstance();
        if (!obj.IsValid())
        {
            PR_CORE_ERROR("[Python] Failed to create instance of {0}", meta->ClassName);
            return 0;
        }

        // Push field values from C++ buffer to Python instance
        for (auto& [hash, field] : binding.Fields)
        {
            Buffer& buf = field.GetBuffer();
            if (buf.Data && buf.Size > 0)
                obj.SetFieldRaw(field.GetName().c_str(), buf.Data);
        }

        Python::ScriptObject* entityObj = GetScriptObject(sceneID, entityID);
        if (entityObj)
            obj.SetAttribute("Entity", entityObj->GetRef());

        UUID behaviourID = binding.BehaviourID;
        auto& sceneMap = s_PythonScriptObjects[sceneID];
        auto [it, inserted] = sceneMap.emplace(behaviourID, std::move(obj));
        PR_CORE_ASSERT(inserted, "BehaviourID collision in s_PythonScriptObjects!");

        // Bind fields to the now-stable Python object
        for (auto& [hash, field] : binding.Fields)
            field.SetInstance(&it->second);

        PR_CORE_INFO("[Python] Added behaviour {0} ({1}) to entity {2}", meta->ClassName, (uint64_t)behaviourID, (uint64_t)entityID);
        return behaviourID;
    }

    void PythonScriptEngine::RemoveBehaviour(Entity& entity, UUID behaviourID)
    {
        UUID sceneID = s_SceneContext ? s_SceneContext->GetUUID() : UUID(0);
        PR_CORE_ASSERT(sceneID, "没有场景上下文");
        // Clear field instances and remove binding
        auto& comp = entity.GetComponent<PythonScriptComponent>();
        for (auto& binding : comp.Behaviours)
        {
            if (binding.BehaviourID == behaviourID)
            {
                for (auto& [hash, field] : binding.Fields)
                    field.ClearInstance();
                break;
            }
        }

        auto sceneIt = s_PythonScriptObjects.find(sceneID);
        if (sceneIt != s_PythonScriptObjects.end())
            sceneIt->second.erase(behaviourID);

        comp.Behaviours.erase(
            std::remove_if(comp.Behaviours.begin(), comp.Behaviours.end(),
                [behaviourID](const auto& b) { return b.BehaviourID == behaviourID; }),
            comp.Behaviours.end());

        PR_CORE_INFO("[Python] Removed behaviour {0} from entity {1}", (uint64_t)behaviourID, (uint64_t)entity.GetUUID());
    }

    void PythonScriptEngine::ReleaseAll()
    {
        s_PythonScriptObjects.clear();
    }
}
