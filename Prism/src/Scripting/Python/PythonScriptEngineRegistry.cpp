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

    void PythonScriptEngineRegistry::RegisterAll()
    {
        InitComponentTypes();
    }
}

PYBIND11_MODULE(PrismNative, m)
{
    using namespace Prism;
    using namespace Prism::PythonScript;

    py::class_<RaycastHit>(m, "RaycastHit")
        .def_readwrite("EntityID", &RaycastHit::EntityID)
        .def_readwrite("Position", &RaycastHit::Position)
        .def_readwrite("Normal", &RaycastHit::Normal)
        .def_readwrite("Distance", &RaycastHit::Distance);

    py::class_<OverlapHitData>(m, "OverlapHitData")
        .def_readwrite("EntityID", &OverlapHitData::EntityID)
        .def_readwrite("ColliderType", &OverlapHitData::ColliderType)
        .def_readwrite("IsTrigger", &OverlapHitData::IsTrigger)
        .def_property_readonly("ShapeData", [](const OverlapHitData& d) {
            py::list shapeData;
            for (int i = 0; i < 6; ++i)
                shapeData.append(PyFloat_FromDouble(d.ShapeData[i]));
            return shapeData;
        })
        .def_property_readonly("MeshHandle", [](const OverlapHitData& d) -> uint64_t {
            return reinterpret_cast<uintptr_t>(d.MeshHandle);
        });

    py::class_<ScriptTransform>(m, "ScriptTransform")
        .def_readwrite("Position", &ScriptTransform::Position)
        .def_readwrite("Rotation", &ScriptTransform::Rotation)
        .def_readwrite("Scale", &ScriptTransform::Scale)
        .def_readwrite("Up", &ScriptTransform::Up)
        .def_readwrite("Right", &ScriptTransform::Right)
        .def_readwrite("Forward", &ScriptTransform::Forward);

#define BIND_MODULE_FUNCTION(name) m.def(#name, &Prism::PythonScript::name)

    // Log
    BIND_MODULE_FUNCTION(Prism_Log_LogMessage);
    // Time
    BIND_MODULE_FUNCTION(Prism_Time_GetDeltaTime);
    BIND_MODULE_FUNCTION(Prism_Time_GetUnscaledDeltaTime);
    BIND_MODULE_FUNCTION(Prism_Time_GetTime);
    BIND_MODULE_FUNCTION(Prism_Time_GetUnscaledTime);
    BIND_MODULE_FUNCTION(Prism_Time_GetFixedDeltaTime);
    BIND_MODULE_FUNCTION(Prism_Time_GetFrameCount);
    BIND_MODULE_FUNCTION(Prism_Time_SetTimeScale);
    BIND_MODULE_FUNCTION(Prism_Time_GetTimeScale);
    BIND_MODULE_FUNCTION(Prism_Time_SetFixedDeltaTime);
    // Math
    BIND_MODULE_FUNCTION(Prism_Noise_PerlinNoise);
    // Input
    BIND_MODULE_FUNCTION(Prism_Input_IsKeyPressed);
    BIND_MODULE_FUNCTION(Prism_Input_GetMousePosition);
    BIND_MODULE_FUNCTION(Prism_Input_SetCursorMode);
    BIND_MODULE_FUNCTION(Prism_Input_GetCursorMode);
    BIND_MODULE_FUNCTION(Prism_Input_IsMouseButtonPressed);
    // Entity
    BIND_MODULE_FUNCTION(Prism_Entity_CreateComponent);
    BIND_MODULE_FUNCTION(Prism_Entity_HasComponent);
    BIND_MODULE_FUNCTION(Prism_Entity_FindEntityByTag);
    BIND_MODULE_FUNCTION(Prism_Entity_AddBehaviour);
    BIND_MODULE_FUNCTION(Prism_Entity_RemoveBehaviour);
    BIND_MODULE_FUNCTION(Prism_Entity_GetBehaviour);
    BIND_MODULE_FUNCTION(Prism_Behaviour_GetEnabled);
    BIND_MODULE_FUNCTION(Prism_Behaviour_SetEnabled);
    // TransformComponent
    BIND_MODULE_FUNCTION(Prism_TransformComponent_GetPosition);
    BIND_MODULE_FUNCTION(Prism_TransformComponent_GetRotation);
    BIND_MODULE_FUNCTION(Prism_TransformComponent_GetScale);
    BIND_MODULE_FUNCTION(Prism_TransformComponent_GetUp);
    BIND_MODULE_FUNCTION(Prism_TransformComponent_GetRight);
    BIND_MODULE_FUNCTION(Prism_TransformComponent_GetForward);
    BIND_MODULE_FUNCTION(Prism_TransformComponent_SetPosition);
    BIND_MODULE_FUNCTION(Prism_TransformComponent_SetRotation);
    BIND_MODULE_FUNCTION(Prism_TransformComponent_SetScale);
    BIND_MODULE_FUNCTION(Prism_TransformComponent_GetLocalPosition);
    BIND_MODULE_FUNCTION(Prism_TransformComponent_SetLocalPosition);
    BIND_MODULE_FUNCTION(Prism_TransformComponent_GetLocalRotation);
    BIND_MODULE_FUNCTION(Prism_TransformComponent_SetLocalRotation);
    BIND_MODULE_FUNCTION(Prism_TransformComponent_GetLocalScale);
    BIND_MODULE_FUNCTION(Prism_TransformComponent_SetLocalScale);
    BIND_MODULE_FUNCTION(Prism_TransformComponent_GetTransform);
    BIND_MODULE_FUNCTION(Prism_TransformComponent_SetTransform);
    // MeshRendererComponent
    BIND_MODULE_FUNCTION(Prism_MeshRendererComponent_GetMesh);
    BIND_MODULE_FUNCTION(Prism_MeshRendererComponent_SetMesh);
    BIND_MODULE_FUNCTION(Prism_MeshRendererComponent_GetMaterial);
    BIND_MODULE_FUNCTION(Prism_MeshRendererComponent_SetMaterial);
    BIND_MODULE_FUNCTION(Prism_MeshRendererComponent_GetMaterialCount);
    BIND_MODULE_FUNCTION(Prism_MeshRendererComponent_GetMaterials);
    BIND_MODULE_FUNCTION(Prism_MeshRendererComponent_SetMaterials);
    // Mesh
    BIND_MODULE_FUNCTION(Prism_Mesh_Constructor);
    BIND_MODULE_FUNCTION(Prism_Mesh_Destructor);
    BIND_MODULE_FUNCTION(Prism_MeshFactory_CreatePlane);
    // Texture2D
    BIND_MODULE_FUNCTION(Prism_Texture2D_Constructor);
    BIND_MODULE_FUNCTION(Prism_Texture2D_Destructor);
    // RigidBody2DComponent
    BIND_MODULE_FUNCTION(Prism_RigidBody2DComponent_ApplyLinearImpulse);
    BIND_MODULE_FUNCTION(Prism_RigidBody2DComponent_GetLinearVelocity);
    BIND_MODULE_FUNCTION(Prism_RigidBody2DComponent_SetLinearVelocity);
    // RigidBodyComponent
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_AddForce);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_AddTorque);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_GetLinearVelocity);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_SetLinearVelocity);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_Rotate);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_GetLayer);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_GetMass);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_SetMass);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_GetBodyType);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_GetAngularVelocity);
    BIND_MODULE_FUNCTION(Prism_RigidBodyComponent_SetAngularVelocity);
    // Physics
    BIND_MODULE_FUNCTION(Prism_Physics_Raycast);
    BIND_MODULE_FUNCTION(Prism_Physics_OverlapBox);
    BIND_MODULE_FUNCTION(Prism_Physics_OverlapCapsule);
    BIND_MODULE_FUNCTION(Prism_Physics_OverlapSphere);
    BIND_MODULE_FUNCTION(Prism_Physics_GetGravity);
    BIND_MODULE_FUNCTION(Prism_Physics_SetGravity);
    // Material
    BIND_MODULE_FUNCTION(Prism_Material_Constructor);
    BIND_MODULE_FUNCTION(Prism_Material_Destructor);
    BIND_MODULE_FUNCTION(Prism_Material_SetFloat);
    BIND_MODULE_FUNCTION(Prism_Material_SetInt);
    BIND_MODULE_FUNCTION(Prism_Material_SetBool);
    BIND_MODULE_FUNCTION(Prism_Material_SetVector2);
    BIND_MODULE_FUNCTION(Prism_Material_SetVector3);
    BIND_MODULE_FUNCTION(Prism_Material_SetVector4);
    BIND_MODULE_FUNCTION(Prism_Material_SetColor3);
    BIND_MODULE_FUNCTION(Prism_Material_SetColor);
    BIND_MODULE_FUNCTION(Prism_Material_SetMatrix4);
    BIND_MODULE_FUNCTION(Prism_Material_SetTexture);
    BIND_MODULE_FUNCTION(Prism_Material_SetKeyword);
    BIND_MODULE_FUNCTION(Prism_Material_IsKeywordEnabled);
}
