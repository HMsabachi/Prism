#pragma once
#include <unordered_map>
#include <memory>
#include "Prism/Core/Core.h"
#include "Prism/Core/UUID.h"
#include "Prism/Core/Ref.h"
#include "Prism/Core/Log.h"
#include "Prism/Scene/Entity.h"
#include "CSharpScriptStorage.h"

#include <Rolky/HostInstance.hpp>


namespace Prism
{
    class Scene;

    class PRISM_API CSharpScriptEngine
    {
    public:
        CSharpScriptEngine() = delete;

        static void Initialize();
        static void Shutdown();

        // Template: generate ScriptID, create object, store, return copy
        template<typename... TArgs>
        static Rolky::ManagedObject Instantiate(std::string_view className, CSharpScriptStorage& storage, TArgs&&... args);

        // Storage lookup (takes storage reference)
        static CSharpEntityScriptStorage& GetEntityScriptStorage(CSharpScriptStorage& storage, UUID scriptID);

        // Assembly management
        static void LoadEngineAssembly(const std::string& path);
        static void LoadAppAssembly(const std::string& path);
        static void ReloadAssembly(const std::string& path);

        // Scene context
        static void SetSceneContext(const Ref<Scene>& scene);
        static const Ref<Scene>& GetCurrentSceneContext();

        static bool ModuleExists(const std::string& moduleName);
        static void OnImGuiRender();


        static Rolky::ManagedAssembly& GetEngineAssembly();
        static Rolky::ManagedAssembly& GetAppAssembly();

    private:
        static std::unique_ptr<Rolky::HostInstance> s_Host;
        static std::unique_ptr<Rolky::AssemblyLoadContext> s_LoadContext;
        static Ref<Scene> s_SceneContext;
        static Rolky::ManagedAssembly s_EngineAssembly;
        static Rolky::ManagedAssembly s_AppAssembly;
        static bool s_Initialized;
    };
    template<typename... TArgs>
    Rolky::ManagedObject CSharpScriptEngine::Instantiate(std::string_view className, CSharpScriptStorage& storage, TArgs&&... args)
    {
        auto type = GetAppAssembly().GetType(className);
        PR_CORE_ASSERT(type, "Class not found in app assembly!");
        auto instance = type->CreateInstance(std::forward<TArgs>(args)...);
        UUID scriptID = UUID();
        storage.Store(scriptID, instance);
        return instance;
    }

}


