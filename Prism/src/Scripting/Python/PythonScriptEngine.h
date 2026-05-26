#pragma once
#include "Scripting/Python/Interop/PythonScriptCore.h"
#include "Prism/Core/Core.h"
#include "Prism/Core/UUID.h"
#include "Prism/Core/Ref.h"
#include "Prism/Core/Log.h"
#include "Prism/Scene/Entity.h"
#include "PythonScriptStorage.h"
#include <unordered_map>
#include <memory>
#include <functional>

namespace Prism
{
    class Scene;
    template<typename T> class Ref;

    extern std::unordered_map<std::string, std::function<void(Entity&)>> s_PythonCreateComponentFuncs;
    extern std::unordered_map<std::string, std::function<bool(Entity&)>> s_PythonHasComponentFuncs;

    class PRISM_API PythonScriptEngine
    {
    public:
        PythonScriptEngine() = delete;

        static void Initialize();
        static void Shutdown();

        static std::unordered_map<UUID, std::unordered_map<UUID, Python::ScriptObject>> s_PythonScriptObjects;

        template<typename... TArgs>
        static UUID Instantiate(UUID scriptID, std::string_view className, PythonScriptStorage& storage, TArgs&&... args);

        static PythonEntityScriptStorage& GetEntityScriptStorage(PythonScriptStorage& storage, UUID scriptID);

        static UUID AddBehaviour(Entity& entity, PythonBehaviourBinding& binding);
        static void RemoveBehaviour(Entity& entity, UUID behaviourID);

        static Python::ScriptObject* GetScriptObject(UUID sceneID, UUID scriptID);
        static void RemoveScriptObject(PythonScriptStorage& storage, UUID scriptID);
        static void ReleaseAll();

        // Scene context
        static void SetSceneContext(const WeakRef<Scene>& scene);
        static const WeakRef<Scene>& GetCurrentSceneContext();

    private:
        static WeakRef<Scene> s_SceneContext;
        static bool s_Initialized;
    };

    template<typename... TArgs>
    UUID PythonScriptEngine::Instantiate(UUID scriptID, std::string_view className, PythonScriptStorage& storage, TArgs&&... /*args*/)
    {
        // className format: "ModuleName.ClassName" or just "ClassName"
        std::string_view moduleName = className;
        std::string_view clsName = className;
        auto dotPos = className.rfind('.');
        if (dotPos != std::string_view::npos)
        {
            moduleName = className.substr(0, dotPos);
            clsName = className.substr(dotPos + 1);
        }

        Python::ScriptModule mod = Python::ScriptModule::Import(moduleName.data());
        PR_CORE_ASSERT(mod.IsValid(), "Python module not found!");

        Python::ScriptClass cls = Python::ScriptClass::From(mod, clsName.data());
        PR_CORE_ASSERT(cls.IsValid(), "Python class not found!");

        Python::ScriptObject obj = cls.CreateInstance();
        UUID sceneID = s_SceneContext ? s_SceneContext->GetUUID() : UUID(0);
        auto& sceneMap = s_PythonScriptObjects[sceneID];
        auto [it, inserted] = sceneMap.emplace(scriptID, std::move(obj));
        PR_CORE_ASSERT(inserted, "ScriptID collision in s_PythonScriptObjects!");
        it->second.SetField<uint64_t>("_id", (uint64_t)scriptID);
        storage.Store(scriptID, &it->second);
        return scriptID;
    }
}


