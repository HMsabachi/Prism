#include "prpch.h"
#include "PythonScriptEngineRegistry.h"
#include "PythonScriptWrappers.h"
#include "PythonScriptTypeCasters.h"
#include <pybind11/pybind11.h>

#include "Prism/Scene/Components.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Utilities/TypeInfo.h"

namespace py = pybind11;

namespace Prism
{
    std::unordered_map<uint64_t, std::function<void(Entity&)>> s_PythonCreateComponentFuncs;
    std::unordered_map<uint64_t, std::function<bool(Entity&)>> s_PythonHasComponentFuncs;

    template<typename TComponent>
    static void RegisterPythonComponent(py::module_& pyComponentModule)
    {
        const TypeNameString& name = TypeInfo<TComponent, true>().Name();
        py::object cls = pyComponentModule.attr(name.data());
        uint64_t typeId = reinterpret_cast<uint64_t>(cls.ptr());
        s_PythonCreateComponentFuncs[typeId] = [](Entity& e) { e.AddComponent<TComponent>(); };
        s_PythonHasComponentFuncs[typeId]    = [](Entity& e) { return e.HasComponent<TComponent>(); };
    }

    static void InitComponentTypes()
    {
        try
        {
            s_PythonCreateComponentFuncs.clear();
            s_PythonHasComponentFuncs.clear();

            py::module_ compMod = py::module::import("Prism.Component");

            RegisterPythonComponent<TagComponent>(compMod);
            RegisterPythonComponent<TransformComponent>(compMod);
            RegisterPythonComponent<MeshRendererComponent>(compMod);
            RegisterPythonComponent<CameraComponent>(compMod);
            RegisterPythonComponent<SpriteRendererComponent>(compMod);
            RegisterPythonComponent<RigidBody2DComponent>(compMod);
            RegisterPythonComponent<BoxCollider2DComponent>(compMod);
            RegisterPythonComponent<CircleCollider2DComponent>(compMod);
            RegisterPythonComponent<RigidBodyComponent>(compMod);
            RegisterPythonComponent<BoxColliderComponent>(compMod);
            RegisterPythonComponent<SphereColliderComponent>(compMod);
            RegisterPythonComponent<CapsuleColliderComponent>(compMod);
        }
        catch (const py::error_already_set& e)
        {
            PR_CORE_ERROR("Failed to import Prism.Component module: {0}", e.what());
            return;
        }
    }

    void PythonScriptEngineRegistry::RegisterAll()
    {
        InitComponentTypes();
    }
}
