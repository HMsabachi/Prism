#include "prpch.h"
#include "CSharpScriptEngine.h"
#include "CSharpScriptStorage.h"
#include "CSharpScriptEngineRegistry.h"
#include "CSharpScriptMetaRegistry.h"
#include "Prism/Scene/Scene.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/Components.h"
#include <filesystem>
#include <algorithm>
#include <imgui.h>

#include <Rolky/HostInstance.hpp>
#include <Rolky/GC.hpp>
#include <Rolky/TypeCache.hpp>

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
    Scope<AssemblyData> CSharpScriptEngine::s_EngineAssemblyData;
    Scope<AssemblyData> CSharpScriptEngine::s_AppAssemblyData;
    bool CSharpScriptEngine::s_Initialized = false;
    std::unordered_map<UUID, std::unordered_map<UUID, Rolky::ManagedObject>> CSharpScriptEngine::s_ManagedObjects;
    CSharpScriptEngine::ReloadDelegate CSharpScriptEngine::s_PreUnloadCallbacks;
    CSharpScriptEngine::ReloadDelegate CSharpScriptEngine::s_PostReloadCallbacks;
    std::string CSharpScriptEngine::s_EngineAssemblyPath;

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
        UnloadCurrentContext();
        if (s_Host)
        {
            s_Host->Shutdown();
            s_Host.reset();
        }
        s_SceneContext = nullptr;
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
            PR_CORE_WARN("[CSharp] Attempted to remove managed object with scriptID {0} but no objects found for sceneID {1}", (uint64_t)scriptID, (uint64_t)sceneID);
        
    }

    UUID CSharpScriptEngine::AddBehaviour(Entity& entity, CSharpBehaviourBinding& binding)
    {
        UUID sceneID = s_SceneContext ? s_SceneContext->GetUUID() : UUID(0);
        PR_CORE_ASSERT(sceneID, "没有场景上下文");

        UUID entityID = entity.GetUUID();

        auto* entityObj = GetManagedObject(sceneID, entityID);
        if (!entityObj)
        {
            PR_CORE_ERROR("[CSharp] Cannot add behaviour: Entity managed object not found for {0}", (uint64_t)entityID);
            return 0;
        }

        auto* meta = CSharpScriptMetaRegistry::GetClassMetadata(binding.ClassID);
        if (!meta)
        {
            PR_CORE_ERROR("[CSharp] Cannot add behaviour: class metadata not found for ClassID {0}", (uint64_t)binding.ClassID);
            return 0;
        }

        auto type = GetAppAssembly().GetType(meta->FullName);
        if (!type)
        {
            PR_CORE_ERROR("[CSharp] Class not found in app assembly: {0}", meta->FullName);
            return 0;
        }

        auto instance = type.CreateInstance();
        if (!instance.IsValid())
        {
            PR_CORE_ERROR("[CSharp] Failed to create instance of {0}", meta->FullName);
            return 0;
        }

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

        for (auto& [hash, field] : binding.Fields)
            field.SetInstance(&it->second);

        PR_CORE_INFO("[CSharp] Added behaviour {0} ({1}) to entity {2}", meta->ClassName, (uint64_t)behaviourID, (uint64_t)entityID);
        return behaviourID;
    }

    void CSharpScriptEngine::RemoveBehaviour(Entity& entity, UUID behaviourID)
    {
        UUID sceneID = s_SceneContext ? s_SceneContext->GetUUID() : UUID(0);
        PR_CORE_ASSERT(sceneID, "没有场景上下文");


        auto* obj = GetManagedObject(sceneID, behaviourID);

        auto& comp = entity.GetComponent<CSharpScriptComponent>();
        auto it = comp.Behaviours.find(behaviourID);
        if (it != comp.Behaviours.end())
        {
            for (auto& [hash, field] : it->second.Fields)
                field.ClearInstance();
            comp.Behaviours.erase(it);
        }

        auto sceneIt = s_ManagedObjects.find(sceneID);
        if (sceneIt != s_ManagedObjects.end())
            sceneIt->second.erase(behaviourID);

        PR_CORE_INFO("[CSharp] Removed behaviour {0} from entity {1}", (uint64_t)behaviourID, (uint64_t)entity.GetUUID());
    }

    void CSharpScriptEngine::ReleaseAll()
    {
        s_ManagedObjects.clear();
    }

    void CSharpScriptEngine::LoadEngineAssembly(const std::string& assemblyPath)
    {
        PR_PROFILE_FUNCTION();
        s_EngineAssemblyPath = std::filesystem::absolute(assemblyPath).string();
        s_EngineAssemblyData = CreateScope<AssemblyData>();
        s_EngineAssemblyData->Assembly = &s_LoadContext->LoadAssembly(s_EngineAssemblyPath);

        if (s_EngineAssemblyData->Assembly->GetLoadStatus() != Rolky::AssemblyLoadStatus::Success)
        {
            PR_CORE_ERROR("[CSharp] Failed to load engine assembly: {0}", s_EngineAssemblyPath);
            return;
        }

        CSharpScriptEngineRegistry::RegisterAll();
        s_EngineAssemblyData->Assembly->UploadInternalCalls();

        auto& initClass = s_EngineAssemblyData->Assembly->GetLocalType("Prism.Core");
        initClass.InvokeStaticMethod("Init");
    }

    void CSharpScriptEngine::LoadAppAssembly(const std::string& assemblyPath)
    {
        PR_PROFILE_FUNCTION();
        auto path = std::filesystem::absolute(assemblyPath).string();
        s_AppAssemblyData = CreateScope<AssemblyData>();
        s_AppAssemblyData->Assembly = &s_LoadContext->LoadAssembly(path);

        if (s_AppAssemblyData->Assembly->GetLoadStatus() != Rolky::AssemblyLoadStatus::Success)
        {
            PR_CORE_ERROR("[CSharp] Failed to load app assembly: {0}", path);
            return;
        }

        CSharpScriptMetaRegistry::Init();
        CSharpScriptMetaRegistry::BuildCache();
    }

    void CSharpScriptEngine::ReloadAppAssembly(const std::string& appAssemblyPath)
    {
        PR_PROFILE_FUNCTION();
        PR_CORE_INFO("[CSharp] Reloading assemblies...");

        s_PreUnloadCallbacks();

        UnloadCurrentContext();

        ReloadContextAndEngineAssembly();
        LoadEngineAssembly(s_EngineAssemblyPath);
        LoadAppAssembly(appAssemblyPath);

        s_PostReloadCallbacks();

        PR_CORE_INFO("[CSharp] Assembly reload complete.");
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
        return s_AppAssemblyData && s_AppAssemblyData->Assembly
            && s_AppAssemblyData->Assembly->GetLocalType(moduleName);
    }

    Rolky::ManagedAssembly& CSharpScriptEngine::GetEngineAssembly()
    {
        PR_CORE_ASSERT(s_EngineAssemblyData && s_EngineAssemblyData->Assembly,
                       "Engine assembly not loaded!");
        return *s_EngineAssemblyData->Assembly;
    }

    Rolky::ManagedAssembly& CSharpScriptEngine::GetAppAssembly()
    {
        PR_CORE_ASSERT(s_AppAssemblyData && s_AppAssemblyData->Assembly,
                       "App assembly not loaded!");
        return *s_AppAssemblyData->Assembly;
    }

    void CSharpScriptEngine::OnImGuiRender()
    {
        ImGui::Begin("Script Engine Debug");
        ImGui::Text("C# Engine Initialized: %s", s_Initialized ? "Yes" : "No");
        ImGui::End();
    }

    CSharpScriptEngine::ReloadCallbackToken CSharpScriptEngine::RegisterPreUnloadCallback(ReloadDelegate::FuncType cb)
    {
        return s_PreUnloadCallbacks.Add(std::move(cb));
    }

    void CSharpScriptEngine::UnregisterPreUnloadCallback(ReloadCallbackToken token)
    {
        s_PreUnloadCallbacks.Remove(token);
    }

    CSharpScriptEngine::ReloadCallbackToken CSharpScriptEngine::RegisterPostReloadCallback(ReloadDelegate::FuncType cb)
    {
        return s_PostReloadCallbacks.Add(std::move(cb));
    }

    void CSharpScriptEngine::UnregisterPostReloadCallback(ReloadCallbackToken token)
    {
        s_PostReloadCallbacks.Remove(token);
    }

    // ── ALC 生命周期 ──
    void CSharpScriptEngine::UnloadCurrentContext()
    {
        s_ManagedObjects.clear();
        CSharpScriptMetaRegistry::Shutdown();
        s_EngineAssemblyData.reset();
        s_AppAssemblyData.reset();

        if (s_Host && s_LoadContext)
        {
            s_Host->UnloadAssemblyLoadContext(*s_LoadContext);
            s_LoadContext.reset();
        }

        Rolky::TypeCache::Get().Clear();

        Rolky::GC::Collect();
        Rolky::GC::WaitForPendingFinalizers();
        Rolky::GC::Collect();
        Rolky::GC::WaitForPendingFinalizers();
        Rolky::GC::Collect();
    }

    void CSharpScriptEngine::ReloadContextAndEngineAssembly()
    {
        PR_CORE_ASSERT(s_Host, "Host must remain alive during reload!");
        s_LoadContext = std::make_unique<Rolky::AssemblyLoadContext>(
            std::move(s_Host->CreateAssemblyLoadContext("PrismLoadContext")));
    }

}
