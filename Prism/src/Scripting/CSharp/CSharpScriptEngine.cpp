#include "prpch.h"
#include "CSharpScriptEngine.h"
#include "CSharpScriptStorage.h"
#include "ScriptEngineRegistry.h"
#include "Prism/Scene/Scene.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/Components.h"
#include <filesystem>
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
    Ref<Scene> CSharpScriptEngine::s_SceneContext;
    Rolky::ManagedAssembly CSharpScriptEngine::s_EngineAssembly;
    Rolky::ManagedAssembly CSharpScriptEngine::s_AppAssembly;
    bool CSharpScriptEngine::s_Initialized = false;

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
    }

    void CSharpScriptEngine::ReloadAssembly(const std::string& assemblyPath)
    {
        PR_PROFILE_FUNCTION();
        auto path = std::filesystem::absolute(assemblyPath).string();
        s_AppAssembly = s_LoadContext->LoadAssembly(path);
    }

    void CSharpScriptEngine::SetSceneContext(const Ref<Scene>& scene)
    {
        s_SceneContext = scene;
    }

    const Ref<Scene>& CSharpScriptEngine::GetCurrentSceneContext()
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
