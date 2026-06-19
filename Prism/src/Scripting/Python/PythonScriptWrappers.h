#pragma once
#include "Scripting/Python/Interop/PythonScriptCore.h"

namespace Prism::Script
{

#pragma region Log
    enum class LogLevel : int32_t
    {
        Trace = BIT(0),
        Debug = BIT(1),
        Info = BIT(2),
        Warn = BIT(3),
        Error = BIT(4),
        Critical = BIT(5)
    };
    Python::ScriptValue* Prism_Log_LogMessage(Python::ScriptValue* self, Python::ScriptValue* args);
#pragma endregion

    // Time
    Python::ScriptValue* Prism_Time_GetDeltaTime(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Time_GetTime(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Time_GetUnscaledDeltaTime(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Time_GetUnscaledTime(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Time_GetFixedDeltaTime(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Time_GetFrameCount(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Time_GetTimeScale(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Time_SetTimeScale(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Time_SetFixedDeltaTime(Python::ScriptValue* self, Python::ScriptValue* args);

    // Math
    Python::ScriptValue* Prism_Noise_PerlinNoise(Python::ScriptValue* self, Python::ScriptValue* args);

    // Input
    Python::ScriptValue* Prism_Input_IsKeyPressed(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Input_GetMousePosition(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Input_SetCursorMode(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Input_GetCursorMode(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Input_IsMouseButtonPressed(Python::ScriptValue* self, Python::ScriptValue* args);

    // Entity
    Python::ScriptValue* Prism_Entity_GetTransform(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Entity_SetTransform(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Entity_CreateComponent(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Entity_HasComponent(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Entity_AddBehaviour(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Entity_FindEntityByTag(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Entity_RemoveBehaviour(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Entity_GetBehaviour(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Behaviour_GetEnabled(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Behaviour_SetEnabled(Python::ScriptValue* self, Python::ScriptValue* args);

    // TagComponent
    Python::ScriptValue* Prism_TagComponent_GetTag(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_TagComponent_SetTag(Python::ScriptValue* self, Python::ScriptValue* args);

    // TransformComponent
    Python::ScriptValue* Prism_TransformComponent_GetPosition(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_TransformComponent_SetPosition(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_TransformComponent_GetRotation(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_TransformComponent_SetRotation(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_TransformComponent_GetScale(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_TransformComponent_SetScale(Python::ScriptValue* self, Python::ScriptValue* args);

    // RigidBody2DComponent
    Python::ScriptValue* Prism_RigidBody2DComponent_ApplyLinearImpulse(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_RigidBody2DComponent_GetLinearVelocity(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_RigidBody2DComponent_SetLinearVelocity(Python::ScriptValue* self, Python::ScriptValue* args);

    // RigidBodyComponent (3D)
    Python::ScriptValue* Prism_RigidBodyComponent_AddForce(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_RigidBodyComponent_AddTorque(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_RigidBodyComponent_GetLinearVelocity(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_RigidBodyComponent_SetLinearVelocity(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_RigidBodyComponent_Rotate(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_RigidBodyComponent_GetLayer(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_RigidBodyComponent_GetMass(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_RigidBodyComponent_SetMass(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_RigidBodyComponent_GetBodyType(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_RigidBodyComponent_GetAngularVelocity(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_RigidBodyComponent_SetAngularVelocity(Python::ScriptValue* self, Python::ScriptValue* args);

    // Physics
    Python::ScriptValue* Prism_Physics_Raycast(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Physics_OverlapBox(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Physics_OverlapCapsule(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Physics_OverlapSphere(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Physics_GetGravity(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Physics_SetGravity(Python::ScriptValue* self, Python::ScriptValue* args);

    // Mesh
    Python::ScriptValue* Prism_Mesh_Constructor(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Mesh_Destructor(Python::ScriptValue* self, Python::ScriptValue* args);

    // MeshFactory
    Python::ScriptValue* Prism_MeshFactory_CreatePlane(Python::ScriptValue* self, Python::ScriptValue* args);

    // MeshComponent
    Python::ScriptValue* Prism_MeshRendererComponent_GetMesh(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_MeshRendererComponent_SetMesh(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_MeshRendererComponent_GetMaterial(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_MeshRendererComponent_SetMaterial(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_MeshRendererComponent_GetMaterialCount(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_MeshRendererComponent_GetMaterials(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_MeshRendererComponent_SetMaterials(Python::ScriptValue* self, Python::ScriptValue* args);

    // Texture2D
    Python::ScriptValue* Prism_Texture2D_Constructor(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Texture2D_Destructor(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Texture2D_SetData(Python::ScriptValue* self, Python::ScriptValue* args);

    // Material
    Python::ScriptValue* Prism_Material_Constructor(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Material_GetDefaultMaterial(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Material_Destructor(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Material_SetFloat(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Material_SetInt(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Material_SetBool(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Material_SetVector2(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Material_SetColor3(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Material_SetColor(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Material_SetMatrix4(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Material_SetVector3(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Material_SetVector4(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Material_SetTexture(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Material_SetKeyword(Python::ScriptValue* self, Python::ScriptValue* args);
    Python::ScriptValue* Prism_Material_IsKeywordEnabled(Python::ScriptValue* self, Python::ScriptValue* args);


} // namespace Prism::Script
