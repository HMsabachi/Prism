#include "prpch.h"
#include "PythonScriptEngine.h"
#include "PythonScriptEngineRegistry.h"
#include "PythonScriptMetaRegistry.h"
#include "PythonField.inl"

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "Prism/Scene/Scene.h"

#include <filesystem>

namespace py = pybind11;
extern "C" PyObject* PyInit_PrismEngine();
namespace Prism
{
    WeakRef<Scene> PythonScriptEngine::s_SceneContext;
    bool PythonScriptEngine::s_Initialized = false;
    std::unordered_map<UUID, std::unordered_map<UUID, py::object>> PythonScriptEngine::s_PythonScriptObjects;
    PythonScriptEngine::ReloadDelegate PythonScriptEngine::s_PreUnloadCallbacks;
    PythonScriptEngine::ReloadDelegate PythonScriptEngine::s_PostReloadCallbacks;

    static bool IsSubClassOf(const py::object& obj, const py::object& baseClass)
    {
        try
        {
            py::module_ inspect = py::module::import("builtins");
            return py::bool_(inspect.attr("issubclass")(obj, baseClass));
        }
        catch (py::error_already_set& e)
        {
            PR_CORE_ERROR("[Python] IsSubClassOf({0}): {1}", (std::string)py::str(baseClass), e.what());
            return false;
        }
    }
    template<typename... Args>
    static py::object instantiateClass(const py::object& cls, Args&&... args)
    {
        try
        {
            return cls(std::forward<Args>(args)...);
        }
        catch (py::error_already_set& e)
        {
            PR_CORE_ERROR("[Python] instantiateClass({0}): {1}", (std::string)py::str(cls), e.what());
            return py::none();
        }
    }

    void PythonScriptEngine::Initialize()
    {
        PR_PROFILE_FUNCTION();

        if (s_Initialized)
            return;

        PyImport_AppendInittab("PrismEngine", PyInit_PrismEngine);
        py::initialize_interpreter();
        py::exec("import sys; sys.path.append('Assets/scripts/Python')");

        PythonScriptEngineRegistry::RegisterAll();

        s_Initialized = true;
        PR_CORE_TRACE("[Python] Python 运行时已初始化 (pybind11)");

        PythonScriptMetaRegistry::BuildCache();
    }

    void PythonScriptEngine::Shutdown()
    {
        PR_PROFILE_FUNCTION();

        if (!s_Initialized)
            return;

        PythonScriptMetaRegistry::Shutdown();
        ReleaseAll();
        py::finalize_interpreter();
        s_Initialized = false;
        s_SceneContext = nullptr;
        PR_CORE_INFO("[Python] Python 解释器已关闭");
    }

    void PythonScriptEngine::SetSceneContext(const WeakRef<Scene>& scene)
    {
        s_SceneContext = scene;
    }

    const WeakRef<Scene>& PythonScriptEngine::GetCurrentSceneContext()
    {
        return s_SceneContext;
    }

    py::object* PythonScriptEngine::GetScriptObject(UUID sceneID, UUID scriptID)
    {
        auto sceneIt = s_PythonScriptObjects.find(sceneID);
        if (sceneIt == s_PythonScriptObjects.end())
            return nullptr;
        auto objIt = sceneIt->second.find(scriptID);
        return objIt != sceneIt->second.end() ? &objIt->second : nullptr;
    }

    UUID PythonScriptEngine::AddBehaviour(Entity& entity, PythonBehaviourBinding& binding)
    {
        pybind11::gil_scoped_acquire gilAcquire;
        try
        {
            UUID sceneID = s_SceneContext ? s_SceneContext->GetUUID() : UUID(0);
            PR_CORE_ASSERT(sceneID, "没有场景上下文");

            auto* meta = PythonScriptMetaRegistry::GetClassMetadata(binding.ClassID);
            if (!meta)
            {
                PR_CORE_ERROR("[Python] Cannot add behaviour: class metadata not found for ClassID {0}", (uint64_t)binding.ClassID);
                return 0;
            }

            py::module_ mod = py::module::import(meta->ModuleName.c_str());
            py::object cls = mod.attr(meta->ClassName.c_str());
            py::object obj = cls();

            for (auto& [hash, field] : binding.Fields)
            {
                Buffer& buf = field.GetBuffer();
                if (buf.Data && buf.Size > 0)
                    field.SetInstance(&obj);
            }

            py::object entityCls = py::module::import("PrismEngine").attr("Entity");
            obj.attr("Entity") = entityCls(py::cast((uint64_t)entity.GetUUID()));

            UUID behaviourID = binding.BehaviourID;
            auto& sceneMap = s_PythonScriptObjects[sceneID];
            auto [it, inserted] = sceneMap.emplace(behaviourID, std::move(obj));
            PR_CORE_ASSERT(inserted, "BehaviourID collision in s_PythonScriptObjects!");
            auto& instance = it->second;
            for (auto& [hash, field] : binding.Fields)
            {
                py::object* fieldType = field.GetPyType();
                if (IsSubClassOf(*fieldType, *GetPythonType(PYTHON_TYPE_REF)))
                {
                    uint64_t assetPtr = field.GetBuffer().Read<uint64_t>();
                    auto object = instantiateClass(*fieldType);
                    object.attr("SetRef")(assetPtr);
                    instance.attr(field.GetName().c_str()) = object;
                }
                field.SetInstance(&instance);
            }

            PR_CORE_INFO("[Python] Added behaviour {0} ({1}) to entity {2}", meta->ClassName, (uint64_t)behaviourID, (uint64_t)entity.GetUUID());
            return behaviourID;
        }
        catch (py::error_already_set& e)
        {
            PR_CORE_ERROR("[Python] AddBehaviour: {0}", e.what());
            return 0;
        }
    }

    void PythonScriptEngine::RemoveBehaviour(Entity& entity, UUID behaviourID)
    {
        pybind11::gil_scoped_acquire gilAcquire;
        UUID sceneID = s_SceneContext ? s_SceneContext->GetUUID() : UUID(0);
        PR_CORE_ASSERT(sceneID, "没有场景上下文");
        auto& comp = entity.GetComponent<PythonScriptComponent>();
        auto it = comp.Behaviours.find(behaviourID);
        if (it != comp.Behaviours.end())
        {
            for (auto& [hash, field] : it->second.Fields)
                field.ClearInstance();
            comp.Behaviours.erase(it);
        }

        auto sceneIt = s_PythonScriptObjects.find(sceneID);
        if (sceneIt != s_PythonScriptObjects.end())
            sceneIt->second.erase(behaviourID);

        PR_CORE_INFO("[Python] Removed behaviour {0} from entity {1}", (uint64_t)behaviourID, (uint64_t)entity.GetUUID());
    }

    void PythonScriptEngine::ReleaseAll()
    {
        pybind11::gil_scoped_acquire gilAcquire;
        s_PythonScriptObjects.clear();
    }

    PythonScriptEngine::ReloadCallbackToken PythonScriptEngine::RegisterPreUnloadCallback(ReloadDelegate::FuncType cb)
    {
        return s_PreUnloadCallbacks.Add(std::move(cb));
    }
    void PythonScriptEngine::UnregisterPreUnloadCallback(ReloadCallbackToken token)
    {
        s_PreUnloadCallbacks.Remove(token);
    }
    PythonScriptEngine::ReloadCallbackToken PythonScriptEngine::RegisterPostReloadCallback(ReloadDelegate::FuncType cb)
    {
        return s_PostReloadCallbacks.Add(std::move(cb));
    }
    void PythonScriptEngine::UnregisterPostReloadCallback(ReloadCallbackToken token)
    {
        s_PostReloadCallbacks.Remove(token);
    }

    void PythonScriptEngine::ReloadPythonScripts()
    {
        pybind11::gil_scoped_acquire gilAcquire;
        PR_CORE_INFO("[Python] Reloading scripts...");

        s_PreUnloadCallbacks();

        ReleaseAll();
        PythonScriptMetaRegistry::Shutdown();

        PythonScriptMetaRegistry::BuildCache();

        s_PostReloadCallbacks();

        PR_CORE_INFO("[Python] Script reload complete.");
    }

}
