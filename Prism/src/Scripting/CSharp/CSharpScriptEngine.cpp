#include "prpch.h"
#include "CSharpScriptEngine.h"
#include "CSharpScriptStorage.h"
#include "ScriptEngineRegistry.h"
#include "CSharpScriptMetaRegistry.h"
#include "Prism/Scene/Scene.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/Components.h"
#include <filesystem>
#include <algorithm>
#include <imgui.h>

#include <Rolky/HostInstance.hpp>

namespace Prism
{
    static void RolkyMessageCallback(std::string_view message, Rolky::MessageLevel level)
    {
        if (level & Rolky::MessageLevel::Error) PR_CORE_ERROR("[Rolky] {0}", message);
        else if (level & Rolky::MessageLevel::Warning) PR_CORE_WARN("[Rolky] {0}", message);
        else if (level & Rolky::MessageLevel::Info) PR_CORE_INFO("[Rolky] {0}", message);
        else PR_CORE_TRACE("[Rolky] {0}", message);
    }
    static void RolkyExceptionCallback(std::string_view message)
    {
        PR_CORE_ERROR("[Rolky] {0}", message);
    }

    // Static members
    std::unique_ptr<Rolky::HostInstance> CSharpScriptEngine::s_Host;
    std::unique_ptr<Rolky::AssemblyLoadContext> CSharpScriptEngine::s_LoadContext;
    WeakRef<Scene> CSharpScriptEngine::s_SceneContext;
    Rolky::ManagedAssembly CSharpScriptEngine::s_EngineAssembly;
    Rolky::ManagedAssembly CSharpScriptEngine::s_AppAssembly;
    bool CSharpScriptEngine::s_Initialized = false;
    std::unordered_map<UUID, std::unordered_map<UUID, Rolky::ManagedObject>> CSharpScriptEngine::s_ManagedObjects;

    void CSharpScriptEngine::Initialize()
    {
        PR_PROFILE_FUNCTION();

        Rolky::HostSettings setting;
        setting.RolkyDirectory = "Assets/scripts/net9.0";
        setting.MessageCallback = RolkyMessageCallback;
        setting.ExceptionCallback = RolkyExceptionCallback;
        s_Host = std::make_unique<Rolky::HostInstance>();
        s_Host->Initialize(setting);
        s_LoadContext = std::make_unique<Rolky::AssemblyLoadContext>(std::move(s_Host->CreateAssemblyLoadContext("PrismLoadContext")));
        s_Initialized = true;
    }

    void CSharpScriptEngine::Shutdown()
    {
        s_ManagedObjects.clear();
        if (s_Host && s_LoadContext)
            s_Host->UnloadAssemblyLoadContext(*s_LoadContext);
        if (s_Host)
        {
            s_Host->Shutdown();
            s_Host.reset();
        }
        s_SceneContext = nullptr;
        s_LoadContext = nullptr;
        s_Initialized = false;
    }

    CSharpEntityScriptStorage& CSharpScriptEngine::GetEntityScriptStorage(CSharpScriptStorage& storage, UUID scriptID)
    {
        auto it = storage.EntityStorage.find(scriptID);
        PR_CORE_ASSERT(it != storage.EntityStorage.end(), "CSharpScript entity not found!");
        return it->second;
    }

    Rolky::ManagedObject* CSharpScriptEngine::GetManagedObject(UUID sceneID, UUID scriptID)
    {
        auto sceneIt = s_ManagedObjects.find(sceneID);
        if (sceneIt == s_ManagedObjects.end())
            return nullptr;
        auto objIt = sceneIt->second.find(scriptID);
        return objIt != sceneIt->second.end() ? &objIt->second : nullptr;
    }

    void CSharpScriptEngine::RemoveManagedObject(CSharpScriptStorage& storage, UUID scriptID)
    {
        auto sceneID = s_SceneContext ? s_SceneContext->GetUUID() : UUID(0);
        PR_CORE_ASSERT(sceneID, "没有场景上下文");

        auto sceneIt = s_ManagedObjects.find(sceneID);
        if (sceneIt != s_ManagedObjects.end())
        {
            sceneIt->second.erase(scriptID);
            if (sceneIt->second.empty())
                s_ManagedObjects.erase(sceneIt);
            storage.Remove(scriptID);
        }
        else
            PR_CORE_WARN("[C# Script] Attempted to remove managed object with scriptID {0} but no objects found for sceneID {1}", (uint64_t)scriptID, (uint64_t)sceneID);
        
    }

    UUID CSharpScriptEngine::AddBehaviour(Entity& entity, CSharpBehaviourBinding& binding)
    {
        UUID sceneID = s_SceneContext ? s_SceneContext->GetUUID() : UUID(0);
        PR_CORE_ASSERT(sceneID, "没有场景上下文");

        UUID entityID = entity.GetUUID();

        auto* entityObj = GetManagedObject(sceneID, entityID);
        if (!entityObj)
        {
            PR_CORE_ERROR("[C# Script] Cannot add behaviour: Entity managed object not found for {0}", (uint64_t)entityID);
            return 0;
        }

        auto* meta = CSharpScriptMetaRegistry::GetClassMetadata(binding.ClassID);
        if (!meta)
        {
            PR_CORE_ERROR("[C# Script] Cannot add behaviour: class metadata not found for ClassID {0}", (uint64_t)binding.ClassID);
            return 0;
        }

        auto type = GetAppAssembly().GetType(meta->FullName);
        if (!type)
        {
            PR_CORE_ERROR("[C# Script] Class not found in app assembly: {0}", meta->FullName);
            return 0;
        }

        auto instance = type.CreateInstance();
        if (!instance.IsValid())
        {
            PR_CORE_ERROR("[C# Script] Failed to create instance of {0}", meta->FullName);
            return 0;
        }

        // Push field values from C++ buffer to C# instance
        for (auto& [hash, field] : binding.Fields)
        {
            Buffer buf = field.GetBuffer();
            if (buf.Data && buf.Size > 0)
                instance.SetFieldValueRaw(field.GetName(), buf.Data);
        }

        instance.SetPropertyValueRaw("Entity", entityObj);

        UUID behaviourID = binding.BehaviourID;
        auto& sceneMap = s_ManagedObjects[sceneID];
        auto [it, inserted] = sceneMap.emplace(behaviourID, std::move(instance));
        PR_CORE_ASSERT(inserted, "BehaviourID collision in s_ManagedObjects!");

        // Bind fields to the now-stable managed object
        for (auto& [hash, field] : binding.Fields)
            field.SetInstance(&it->second);

        PR_CORE_INFO("[C# Script] Added behaviour {0} ({1}) to entity {2}", meta->ClassName, (uint64_t)behaviourID, (uint64_t)entityID);
        return behaviourID;
    }

    void CSharpScriptEngine::RemoveBehaviour(Entity& entity, UUID behaviourID)
    {
        UUID sceneID = s_SceneContext ? s_SceneContext->GetUUID() : UUID(0);
        PR_CORE_ASSERT(sceneID, "没有场景上下文");


        auto* obj = GetManagedObject(sceneID, behaviourID);

        // Clear field instances and remove binding
        auto& comp = entity.GetComponent<CSharpScriptComponent>();
        for (auto& binding : comp.Behaviours)
        {
            if (binding.BehaviourID == behaviourID)
            {
                for (auto& [hash, field] : binding.Fields)
                    field.ClearInstance();
                break;
            }
        }

        auto sceneIt = s_ManagedObjects.find(sceneID);
        if (sceneIt != s_ManagedObjects.end())
            sceneIt->second.erase(behaviourID);

        comp.Behaviours.erase(
            std::remove_if(comp.Behaviours.begin(), comp.Behaviours.end(),
                [behaviourID](const auto& b) { return b.BehaviourID == behaviourID; }),
            comp.Behaviours.end());

        PR_CORE_INFO("[C# Script] Removed behaviour {0} from entity {1}", (uint64_t)behaviourID, (uint64_t)entity.GetUUID());
    }

    void CSharpScriptEngine::ReleaseAll()
    {
        s_ManagedObjects.clear();
    }

    void CSharpScriptEngine::LoadEngineAssembly(const std::string& assemblyPath)
    {
        PR_PROFILE_FUNCTION();
        auto path = std::filesystem::absolute(assemblyPath).string();
        s_EngineAssembly = s_LoadContext->LoadAssembly(path);
        ScriptEngineRegistry::RegisterAll();
        auto initClass = s_EngineAssembly.GetType("Prism.Core");
        initClass.InvokeStaticMethod("Init");
    }

    void CSharpScriptEngine::LoadAppAssembly(const std::string& assemblyPath)
    {
        PR_PROFILE_FUNCTION();
        auto path = std::filesystem::absolute(assemblyPath).string();
        s_AppAssembly = s_LoadContext->LoadAssembly(path);
        CSharpScriptMetaRegistry::BuildCache();
    }

    void CSharpScriptEngine::ReloadAssembly(const std::string& assemblyPath)
    {
        PR_PROFILE_FUNCTION();
        auto path = std::filesystem::absolute(assemblyPath).string();
        s_AppAssembly = s_LoadContext->LoadAssembly(path);
    }

    void CSharpScriptEngine::SetSceneContext(const WeakRef<Scene>& scene)
    {
        s_SceneContext = scene;
    }

    const WeakRef<Scene>& CSharpScriptEngine::GetCurrentSceneContext()
    {
        return s_SceneContext;
    }

    bool CSharpScriptEngine::ModuleExists(const std::string& moduleName)
    {
        return s_AppAssembly.GetType(moduleName) ? true : false;
    }

    Rolky::ManagedAssembly& CSharpScriptEngine::GetEngineAssembly()
    {
        return s_EngineAssembly;
    }

    Rolky::ManagedAssembly& CSharpScriptEngine::GetAppAssembly()
    {
        return s_AppAssembly;
    }

    void CSharpScriptEngine::OnImGuiRender()
    {
        ImGui::Begin("Script Engine Debug");
        ImGui::Text("C# Engine Initialized: %s", s_Initialized ? "Yes" : "No");
        ImGui::End();
    }

}
