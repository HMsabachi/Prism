#pragma once
#include <unordered_map>
#include <memory>
#include "Prism/Core/Core.h"
#include "Prism/Core/UUID.h"
#include "Prism/Core/Ref.h"
#include "Prism/Core/Log.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Utilities/Delegate.h"
#include "CSharpScriptStorage.h"

#include <Rolky/HostInstance.hpp>


namespace Prism
{
    class Scene;

    // ── AssemblyData 指针封装（避免 ManagedAssembly 值拷贝导致的悬空指针）──
    struct AssemblyData
    {
        Rolky::ManagedAssembly* Assembly = nullptr;
    };


    class PRISM_API CSharpScriptEngine
    {
    public:
        // ── 热重载回调 ──
        using ReloadDelegate = Delegate<>;
        using ReloadCallbackToken = ReloadDelegate::Token;

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

        // Storage lookup
        static CSharpEntityScriptStorage& GetEntityScriptStorage(CSharpScriptStorage& storage, UUID scriptID);

        // Assembly management
        static void LoadEngineAssembly(const std::string& path);
        static void LoadAppAssembly(const std::string& path);
        static void ReloadAppAssembly(const std::string& appAssemblyPath);

        // Callback registration
        static ReloadCallbackToken RegisterPreUnloadCallback(ReloadDelegate::FuncType callback);
        static void               UnregisterPreUnloadCallback(ReloadCallbackToken token);
        static ReloadCallbackToken RegisterPostReloadCallback(ReloadDelegate::FuncType callback);
        static void               UnregisterPostReloadCallback(ReloadCallbackToken token);

        // Scene context
        static void SetSceneContext(const WeakRef<Scene>& scene);
        static const WeakRef<Scene>& GetCurrentSceneContext();

        static bool ModuleExists(const std::string& moduleName);
        static void OnImGuiRender();

        static Rolky::ManagedAssembly& GetEngineAssembly();
        static Rolky::ManagedAssembly& GetAppAssembly();

    private:
        static void UnloadCurrentContext();
        static void ReloadContextAndEngineAssembly();

        static std::unique_ptr<Rolky::HostInstance> s_Host;
        static std::unique_ptr<Rolky::AssemblyLoadContext> s_LoadContext;
        static WeakRef<Scene> s_SceneContext;
        static Scope<AssemblyData> s_EngineAssemblyData;
        static Scope<AssemblyData> s_AppAssemblyData;
        static bool s_Initialized;

        static ReloadDelegate s_PreUnloadCallbacks;
        static ReloadDelegate s_PostReloadCallbacks;

        static std::string s_EngineAssemblyPath;
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
