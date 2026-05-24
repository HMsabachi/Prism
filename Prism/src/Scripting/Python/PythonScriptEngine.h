#pragma once
#include "Scripting/Python/PythonScriptCore.h"
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

        // Template: generate ScriptID, create object, store, return copy
        template<typename... TArgs>
        static Python::ScriptObject Instantiate(std::string_view className, PythonScriptStorage& storage, TArgs&&... args);

        // Storage lookup (takes storage reference)
        static PythonEntityScriptStorage& GetEntityScriptStorage(PythonScriptStorage& storage, UUID scriptID);

        // Scene context
        static void SetSceneContext(const Ref<Scene>& scene);
        static const Ref<Scene>& GetCurrentSceneContext();

    private:
        static Ref<Scene> s_SceneContext;
        static bool s_Initialized;
    };

    // Template definition: import module, get class, create instance, store, return copy
    template<typename... TArgs>
    Python::ScriptObject PythonScriptEngine::Instantiate(std::string_view className, PythonScriptStorage& storage, TArgs&&... /*args*/)
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
        UUID scriptID = UUID();
        storage.Store(scriptID, obj);
        return obj;
    }
}


