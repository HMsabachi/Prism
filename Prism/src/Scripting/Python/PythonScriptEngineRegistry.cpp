#include "prpch.h"
#include "PythonScriptEngineRegistry.h"
#include "PythonScriptWrappers.h"
#include "Scripting/Python/Interop/PythonScriptCore.h"

#include "Prism/Scene/Components.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Utilities/TypeInfo.h"

namespace Prism
{
    std::unordered_map<uint64_t, std::function<void(Entity&)>> s_PythonCreateComponentFuncs;
    std::unordered_map<uint64_t, std::function<bool(Entity&)>> s_PythonHasComponentFuncs;

    template<typename TComponent>
    static void RegisterPythonComponent()
    {
        const TypeNameString& name = TypeInfo<TComponent, true>().Name();

        // 获取 Python 组件类对象
        Python::ScriptModule mod = Python::ScriptModule::Import("Prism.Component");
        PR_CORE_ASSERT(mod.IsValid(), "Python module Prism.Component not found!");
        Python::ScriptClass cls = Python::ScriptClass::From(mod, name.data());
        PR_CORE_ASSERT(cls.IsValid(), "Python class {} not found in Prism.Component!", name);

        // 用 Python 类型对象地址做 key（与 id(cls) 等效）
        uint64_t typeId = cls.GetTypeId();
        s_PythonCreateComponentFuncs[typeId] = [](Entity& e) { e.AddComponent<TComponent>(); };
        s_PythonHasComponentFuncs[typeId] = [](Entity& e) { return e.HasComponent<TComponent>(); };
    }

    static void InitComponentTypes()
    {
        RegisterPythonComponent<TagComponent>();
        RegisterPythonComponent<TransformComponent>();
        RegisterPythonComponent<MeshRendererComponent>();
        RegisterPythonComponent<CameraComponent>();
        RegisterPythonComponent<SpriteRendererComponent>();
        RegisterPythonComponent<RigidBody2DComponent>();
        RegisterPythonComponent<BoxCollider2DComponent>();
        RegisterPythonComponent<CircleCollider2DComponent>();
        RegisterPythonComponent<RigidBodyComponent>();
        RegisterPythonComponent<BoxColliderComponent>();
        RegisterPythonComponent<SphereColliderComponent>();
        RegisterPythonComponent<CapsuleColliderComponent>();
    }


    void PythonScriptEngineRegistry::RegisterAll()
    {
        s_PythonCreateComponentFuncs.clear();
        s_PythonHasComponentFuncs.clear();

        using namespace Prism::Script;
        Python::NativeModule mod("PrismNative");

#define PR_PYTHON_FUNCTION(func, doc) mod.AddFunction(#func, func, doc)
        // Log
        PR_PYTHON_FUNCTION(Prism_Log_LogMessage, "Log(level, message)");

        // Time
        PR_PYTHON_FUNCTION(Prism_Time_GetDeltaTime, "GetDeltaTime() -> float");
        PR_PYTHON_FUNCTION(Prism_Time_GetTime, "GetTime() -> float");
        PR_PYTHON_FUNCTION(Prism_Time_GetUnscaledDeltaTime, "GetUnscaledDeltaTime() -> float");
        PR_PYTHON_FUNCTION(Prism_Time_GetUnscaledTime, "GetUnscaledTime() -> float");
        PR_PYTHON_FUNCTION(Prism_Time_GetFixedDeltaTime, "GetFixedDeltaTime() -> float");
        PR_PYTHON_FUNCTION(Prism_Time_GetFrameCount, "GetFrameCount() -> uint64");
        PR_PYTHON_FUNCTION(Prism_Time_GetTimeScale, "GetTimeScale() -> float");
        PR_PYTHON_FUNCTION(Prism_Time_SetTimeScale, "SetTimeScale(scale)");

        // Math
        PR_PYTHON_FUNCTION(Prism_Noise_PerlinNoise, "PerlinNoise(x, y) -> float");

        // Input
        PR_PYTHON_FUNCTION(Prism_Input_IsKeyPressed, "IsKeyPressed(key) -> bool");
        PR_PYTHON_FUNCTION(Prism_Input_GetMousePosition, "GetMousePosition() -> vec2");
        PR_PYTHON_FUNCTION(Prism_Input_SetCursorMode, "SetCursorMode(mode)");
        PR_PYTHON_FUNCTION(Prism_Input_GetCursorMode, "GetCursorMode() -> int");
        PR_PYTHON_FUNCTION(Prism_Input_IsMouseButtonPressed, "IsMouseButtonPressed(button) -> bool");

        // Entity
        PR_PYTHON_FUNCTION(Prism_Entity_GetTransform, "GetTransform(entityID) -> mat4");
        PR_PYTHON_FUNCTION(Prism_Entity_SetTransform, "SetTransform(entityID, mat4)");
        PR_PYTHON_FUNCTION(Prism_Entity_CreateComponent, "CreateComponent(entityID, typeName)");
        PR_PYTHON_FUNCTION(Prism_Entity_HasComponent, "HasComponent(entityID, typeName) -> bool");
        PR_PYTHON_FUNCTION(Prism_Entity_AddBehaviour, "AddBehaviour(entityID, cls) -> object");
        PR_PYTHON_FUNCTION(Prism_Entity_FindEntityByTag, "FindEntityByTag(tag) -> uint64");
        PR_PYTHON_FUNCTION(Prism_Entity_RemoveBehaviour, "RemoveBehaviour(entityID, behaviourID)");
        PR_PYTHON_FUNCTION(Prism_Entity_GetBehaviour, "GetBehaviour(entityID, cls) -> object");
        PR_PYTHON_FUNCTION(Prism_Behaviour_GetEnabled, "GetEnabled(behaviourID) -> bool");
        PR_PYTHON_FUNCTION(Prism_Behaviour_SetEnabled, "SetEnabled(behaviourID, enabled)");

        // TagComponent
        PR_PYTHON_FUNCTION(Prism_TagComponent_GetTag, "GetTag(entityID) -> string");
        PR_PYTHON_FUNCTION(Prism_TagComponent_SetTag, "SetTag(entityID, tag)");

        // TransformComponent
        PR_PYTHON_FUNCTION(Prism_TransformComponent_GetPosition, "GetPosition(entityID) -> vec3");
        PR_PYTHON_FUNCTION(Prism_TransformComponent_SetPosition, "SetPosition(entityID, vec3)");
        PR_PYTHON_FUNCTION(Prism_TransformComponent_GetRotation, "GetRotation(entityID) -> vec3 radians");
        PR_PYTHON_FUNCTION(Prism_TransformComponent_SetRotation, "SetRotation(entityID, vec3) degrees");
        PR_PYTHON_FUNCTION(Prism_TransformComponent_GetScale, "GetScale(entityID) -> vec3");
        PR_PYTHON_FUNCTION(Prism_TransformComponent_SetScale, "SetScale(entityID, vec3)");

        // RigidBody2DComponent
        PR_PYTHON_FUNCTION(Prism_RigidBody2DComponent_ApplyLinearImpulse, "ApplyLinearImpulse(entityID, impulse, offset, wake)");
        PR_PYTHON_FUNCTION(Prism_RigidBody2DComponent_GetLinearVelocity, "GetLinearVelocity(entityID) -> vec2");
        PR_PYTHON_FUNCTION(Prism_RigidBody2DComponent_SetLinearVelocity, "SetLinearVelocity(entityID, velocity)");

        // RigidBodyComponent (3D)
        PR_PYTHON_FUNCTION(Prism_RigidBodyComponent_AddForce, "AddForce(entityID, force, forceMode)");
        PR_PYTHON_FUNCTION(Prism_RigidBodyComponent_AddTorque, "AddTorque(entityID, torque, forceMode)");
        PR_PYTHON_FUNCTION(Prism_RigidBodyComponent_GetLinearVelocity, "GetLinearVelocity(entityID) -> vec3");
        PR_PYTHON_FUNCTION(Prism_RigidBodyComponent_SetLinearVelocity, "SetLinearVelocity(entityID, velocity)");
        PR_PYTHON_FUNCTION(Prism_RigidBodyComponent_Rotate, "Rotate(entityID, rotation)");
        PR_PYTHON_FUNCTION(Prism_RigidBodyComponent_GetLayer, "GetLayer(entityID) -> uint");
        PR_PYTHON_FUNCTION(Prism_RigidBodyComponent_GetMass, "GetMass(entityID) -> float");
        PR_PYTHON_FUNCTION(Prism_RigidBodyComponent_SetMass, "SetMass(entityID, mass)");
        // Physics
        PR_PYTHON_FUNCTION(Prism_Physics_Raycast, "Raycast(origin, direction, maxDistance) -> (entityID, pos, normal, distance) or None");
        PR_PYTHON_FUNCTION(Prism_Physics_OverlapBox, "OverlapBox(origin, halfSize) -> tuple of Collider or None");
        PR_PYTHON_FUNCTION(Prism_Physics_OverlapSphere, "OverlapSphere(origin, radius) -> tuple of Collider or None");

        // Mesh
        PR_PYTHON_FUNCTION(Prism_Mesh_Constructor, "Mesh(cpp_handle, filepath)");
        PR_PYTHON_FUNCTION(Prism_Mesh_Destructor, "~Mesh(cpp_handle)");

        // MeshFactory
        PR_PYTHON_FUNCTION(Prism_MeshFactory_CreatePlane, "CreatePlane(width, height) -> handle");

        // MeshRendererComponent
        PR_PYTHON_FUNCTION(Prism_MeshRendererComponent_GetMesh, "GetMesh(entityID) -> handle");
        PR_PYTHON_FUNCTION(Prism_MeshRendererComponent_SetMesh, "SetMesh(entityID, handle)");

        // Texture2D
        PR_PYTHON_FUNCTION(Prism_Texture2D_Constructor, "Texture2D(cpp_handle, width, height)");
        PR_PYTHON_FUNCTION(Prism_Texture2D_Destructor, "~Texture2D(cpp_handle)");
        PR_PYTHON_FUNCTION(Prism_Texture2D_SetData, "SetData(cpp_handle, data)");

        // Material
        PR_PYTHON_FUNCTION(Prism_Material_Constructor, "Material(cpp_handle, shaderName)");
        PR_PYTHON_FUNCTION(Prism_Material_GetDefaultMaterial, "GetDefaultMaterial() -> handle");
        PR_PYTHON_FUNCTION(Prism_Material_Destructor, "~Material(cpp_handle)");
        PR_PYTHON_FUNCTION(Prism_Material_SetFloat, "SetFloat(cpp_handle, uniform, value)");
        PR_PYTHON_FUNCTION(Prism_Material_SetVector3, "SetVector3(cpp_handle, uniform, vec3)");
        PR_PYTHON_FUNCTION(Prism_Material_SetVector4, "SetVector4(cpp_handle, uniform, vec4)");
        PR_PYTHON_FUNCTION(Prism_Material_SetTexture, "SetTexture(cpp_handle, uniform, texHandle)");
        PR_PYTHON_FUNCTION(Prism_Material_SetKeyword, "SetKeyword(cpp_handle, name, enabled)");
        PR_PYTHON_FUNCTION(Prism_Material_IsKeywordEnabled, "IsKeywordEnabled(cpp_handle, name) -> bool");


        mod.Register();
        InitComponentTypes();
        PR_CORE_TRACE("[Python] PrismNative 模块已注册");
    }
}
