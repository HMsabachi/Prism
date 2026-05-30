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
        static std::unordered_map<UUID, std::unordered_map<UUID, Rolky::ManagedObject>> s_ManagedObjects;

        CSharpScriptEngine() = delete;

        static void Initialize();
        static void Shutdown();

        template<typename... TArgs>
        static UUID InstantiateEngine(UUID scriptID, std::string_view className, CSharpScriptStorage& storage, TArgs&&... args);

        static Rolky::ManagedObject* GetManagedObject(UUID sceneID, UUID scriptID);
        static void RemoveManagedObject(CSharpScriptStorage& storage, UUID scriptID);
        static void ReleaseAll();

        static UUID AddBehaviour(Entity& entity, CSharpBehaviourBinding& binding);
        static void RemoveBehaviour(Entity& entity, UUID behaviourID);

        // Storage lookup (takes storage reference)
        static CSharpEntityScriptStorage& GetEntityScriptStorage(CSharpScriptStorage& storage, UUID scriptID);

        // Assembly management
        static void LoadEngineAssembly(const std::string& path);
        static void LoadAppAssembly(const std::string& path);
        static void ReloadAssembly(const std::string& path);

        // Scene context
        static void SetSceneContext(const WeakRef<Scene>& scene);
        static const WeakRef<Scene>& GetCurrentSceneContext();

        static bool ModuleExists(const std::string& moduleName);
        static void OnImGuiRender();

        static Rolky::ManagedAssembly& GetEngineAssembly();
        static Rolky::ManagedAssembly& GetAppAssembly();

    private:
        static std::unique_ptr<Rolky::HostInstance> s_Host;
        static std::unique_ptr<Rolky::AssemblyLoadContext> s_LoadContext;
        static WeakRef<Scene> s_SceneContext;
        static Rolky::ManagedAssembly s_EngineAssembly;
        static Rolky::ManagedAssembly s_AppAssembly;
        static bool s_Initialized;
    };


    // 从 engine assembly 创建框架类 (如 Prism.Entity), 使用外部传入的 ScriptID
    template<typename... TArgs>
    UUID CSharpScriptEngine::InstantiateEngine(UUID scriptID, std::string_view className, CSharpScriptStorage& storage, TArgs&&... args)
    {
        auto type = GetEngineAssembly().GetType(className);
        PR_CORE_ASSERT(type, "Class not found in engine assembly!");
        auto instance = type.CreateInstance(std::forward<TArgs>(args)...);
        UUID sceneID = s_SceneContext ? s_SceneContext->GetUUID() : UUID(0);
        auto& sceneMap = s_ManagedObjects[sceneID];
        auto [it, inserted] = sceneMap.emplace(scriptID, std::move(instance));
        PR_CORE_ASSERT(inserted, "ScriptID collision in s_ManagedObjects!");
        it->second.SetPropertyValue<uint64_t>("ID", (uint64_t)scriptID);
        storage.Store(scriptID, &it->second);
        return scriptID;
    }

}


