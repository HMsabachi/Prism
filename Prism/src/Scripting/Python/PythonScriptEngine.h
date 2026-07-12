#pragma once
#include "Prism/Core/Core.h"
#include "Prism/Core/UUID.h"
#include "Prism/Core/Ref.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Utilities/Delegate.h"
#include "PythonScriptStorage.h"
#include <unordered_map>

namespace pybind11
{
    class object;
}

namespace Prism
{
    class Scene;

    class PRISM_API PythonScriptEngine
    {
    public:
        using ReloadDelegate = Delegate<>;
        using ReloadCallbackToken = ReloadDelegate::Token;

        PythonScriptEngine() = delete;

        static void Initialize();
        static void Shutdown();

        static std::unordered_map<UUID, std::unordered_map<UUID, pybind11::object>> s_PythonScriptObjects;

        static UUID Instantiate(UUID scriptID, const std::string& className, PythonScriptStorage& storage);

        static PythonEntityScriptStorage& GetEntityScriptStorage(PythonScriptStorage& storage, UUID scriptID);

        static UUID AddBehaviour(Entity& entity, PythonBehaviourBinding& binding);
        static void RemoveBehaviour(Entity& entity, UUID behaviourID);

        static pybind11::object* GetScriptObject(UUID sceneID, UUID scriptID);
        static void RemoveScriptObject(PythonScriptStorage& storage, UUID scriptID);
        static void ReleaseAll();

        static void ReloadPythonScripts();

        static ReloadCallbackToken RegisterPreUnloadCallback(ReloadDelegate::FuncType callback);
        static void UnregisterPreUnloadCallback(ReloadCallbackToken token);
        static ReloadCallbackToken RegisterPostReloadCallback(ReloadDelegate::FuncType callback);
        static void UnregisterPostReloadCallback(ReloadCallbackToken token);

        static void SetSceneContext(const WeakRef<Scene>& scene);
        static const WeakRef<Scene>& GetCurrentSceneContext();

    private:
        static WeakRef<Scene> s_SceneContext;
        static bool s_Initialized;

        static ReloadDelegate s_PreUnloadCallbacks;
        static ReloadDelegate s_PostReloadCallbacks;
    };
}
