#pragma once
#include "Prism/Core/Core.h"
#include "Prism/Physics/Physics.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <optional>
#include <vector>

namespace pybind11 { class object; }

namespace Prism
{
    class Mesh;
    class Texture2D;
    class Material;
    struct RaycastHit;
    struct OverlapHitData;
    enum class KeyCode : uint16_t;
    enum class MouseButton : uint16_t;
    enum class CursorMode;
}

namespace Prism::PythonScript
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
    void Prism_Log_LogMessage(int32_t level, const char* message);
#pragma endregion

#pragma region Time
    float Prism_Time_GetDeltaTime();
    float Prism_Time_GetUnscaledDeltaTime();
    float Prism_Time_GetTime();
    float Prism_Time_GetUnscaledTime();
    float Prism_Time_GetFixedDeltaTime();
    int64_t Prism_Time_GetFrameCount();
    void Prism_Time_SetTimeScale(float scale);
    float Prism_Time_GetTimeScale();
    void Prism_Time_SetFixedDeltaTime(float fixedDeltaTime);
#pragma endregion

#pragma region Math
    float Prism_Noise_PerlinNoise(float x, float y);
#pragma endregion

#pragma region Input
    bool Prism_Input_IsKeyPressed(uint16_t key);
    glm::vec2 Prism_Input_GetMousePosition();
    void Prism_Input_SetCursorMode(int mode);
    int Prism_Input_GetCursorMode();
    bool Prism_Input_IsMouseButtonPressed(uint16_t button);
#pragma endregion

#pragma region Entity
    void Prism_Entity_CreateComponent(uint64_t entityID, pybind11::object cls);
    bool Prism_Entity_HasComponent(uint64_t entityID, pybind11::object cls);
    uint64_t Prism_Entity_FindEntityByTag(const char* tag);
    uint64_t Prism_Entity_AddBehaviour(uint64_t entityID, pybind11::object cls);
    void Prism_Entity_RemoveBehaviour(uint64_t entityID, uint64_t behaviourID);
    uint64_t Prism_Entity_GetBehaviour(uint64_t entityID, pybind11::object cls);
    bool Prism_Behaviour_GetEnabled(uint64_t behaviourID);
    void Prism_Behaviour_SetEnabled(uint64_t behaviourID, bool enabled);
#pragma endregion

#pragma region TransformComponent
    struct ScriptTransform { glm::vec3 Position; glm::vec3 Rotation; glm::vec3 Scale; glm::vec3 Up; glm::vec3 Right; glm::vec3 Forward; };
    glm::vec3 Prism_TransformComponent_GetPosition(uint64_t entityID);
    glm::vec3 Prism_TransformComponent_GetRotation(uint64_t entityID);
    glm::vec3 Prism_TransformComponent_GetScale(uint64_t entityID);
    glm::vec3 Prism_TransformComponent_GetUp(uint64_t entityID);
    glm::vec3 Prism_TransformComponent_GetRight(uint64_t entityID);
    glm::vec3 Prism_TransformComponent_GetForward(uint64_t entityID);
    void Prism_TransformComponent_SetPosition(uint64_t entityID, const glm::vec3& position);
    void Prism_TransformComponent_SetRotation(uint64_t entityID, const glm::vec3& rotation);
    void Prism_TransformComponent_SetScale(uint64_t entityID, const glm::vec3& scale);
    glm::vec3 Prism_TransformComponent_GetLocalPosition(uint64_t entityID);
    void Prism_TransformComponent_SetLocalPosition(uint64_t entityID, const glm::vec3& position);
    glm::vec3 Prism_TransformComponent_GetLocalRotation(uint64_t entityID);
    void Prism_TransformComponent_SetLocalRotation(uint64_t entityID, const glm::vec3& rotation);
    glm::vec3 Prism_TransformComponent_GetLocalScale(uint64_t entityID);
    void Prism_TransformComponent_SetLocalScale(uint64_t entityID, const glm::vec3& scale);
    ScriptTransform Prism_TransformComponent_GetTransform(uint64_t entityID);
    void Prism_TransformComponent_SetTransform(uint64_t entityID, const ScriptTransform& transform);
#pragma endregion

#pragma region MeshRendererComponent
    uint64_t Prism_MeshRendererComponent_GetMesh(uint64_t entityID);
    void Prism_MeshRendererComponent_SetMesh(uint64_t entityID, uint64_t meshHandle);
    uint64_t Prism_MeshRendererComponent_GetMaterial(uint64_t entityID, uint64_t index);
    void Prism_MeshRendererComponent_SetMaterial(uint64_t entityID, uint64_t materialHandle, uint64_t index);
    uint64_t Prism_MeshRendererComponent_GetMaterialCount(uint64_t entityID);
    std::vector<uint64_t> Prism_MeshRendererComponent_GetMaterials(uint64_t entityID);
    void Prism_MeshRendererComponent_SetMaterials(uint64_t entityID, const std::vector<uint64_t>& handles);
#pragma endregion

#pragma region Mesh
    uint64_t Prism_Mesh_Constructor(const char* filepath);
    void Prism_Mesh_Destructor(uint64_t handle);
    uint64_t Prism_MeshFactory_CreatePlane(float width, float height);
#pragma endregion

#pragma region Texture2D
    uint64_t Prism_Texture2D_Constructor(uint32_t width, uint32_t height);
    void Prism_Texture2D_Destructor(uint64_t handle);
#pragma endregion

#pragma region RigidBody2DComponent
    void Prism_RigidBody2DComponent_ApplyLinearImpulse(uint64_t entityID, const glm::vec2& impulse, const glm::vec2& offset, bool wake);
    glm::vec2 Prism_RigidBody2DComponent_GetLinearVelocity(uint64_t entityID);
    void Prism_RigidBody2DComponent_SetLinearVelocity(uint64_t entityID, const glm::vec2& velocity);
#pragma endregion

#pragma region RigidBodyComponent
    void Prism_RigidBodyComponent_AddForce(uint64_t entityID, const glm::vec3& force, int32_t forceMode);
    void Prism_RigidBodyComponent_AddTorque(uint64_t entityID, const glm::vec3& torque, int32_t forceMode);
    glm::vec3 Prism_RigidBodyComponent_GetLinearVelocity(uint64_t entityID);
    void Prism_RigidBodyComponent_SetLinearVelocity(uint64_t entityID, const glm::vec3& velocity);
    void Prism_RigidBodyComponent_Rotate(uint64_t entityID, const glm::vec3& rotation);
    uint32_t Prism_RigidBodyComponent_GetLayer(uint64_t entityID);
    float Prism_RigidBodyComponent_GetMass(uint64_t entityID);
    void Prism_RigidBodyComponent_SetMass(uint64_t entityID, float mass);
    uint32_t Prism_RigidBodyComponent_GetBodyType(uint64_t entityID);
    glm::vec3 Prism_RigidBodyComponent_GetAngularVelocity(uint64_t entityID);
    void Prism_RigidBodyComponent_SetAngularVelocity(uint64_t entityID, const glm::vec3& velocity);
#pragma endregion

#pragma region Physics
    std::optional<RaycastHit> Prism_Physics_Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance);
    std::vector<OverlapHitData> Prism_Physics_OverlapBox(const glm::vec3& origin, const glm::vec3& halfSize);
    std::vector<OverlapHitData> Prism_Physics_OverlapCapsule(const glm::vec3& origin, float radius, float halfHeight);
    std::vector<OverlapHitData> Prism_Physics_OverlapSphere(const glm::vec3& origin, float radius);
    float Prism_Physics_GetGravity();
    void Prism_Physics_SetGravity(float gravity);
#pragma endregion

#pragma region Material
    uint64_t Prism_Material_Constructor(const char* shaderName);
    void Prism_Material_Destructor(uint64_t handle);
    void Prism_Material_SetFloat(uint64_t handle, const char* uniform, float value);
    void Prism_Material_SetInt(uint64_t handle, const char* uniform, int value);
    void Prism_Material_SetBool(uint64_t handle, const char* uniform, bool value);
    void Prism_Material_SetVector2(uint64_t handle, const char* uniform, const glm::vec2& value);
    void Prism_Material_SetVector3(uint64_t handle, const char* uniform, const glm::vec3& value);
    void Prism_Material_SetVector4(uint64_t handle, const char* uniform, const glm::vec4& value);
    void Prism_Material_SetColor3(uint64_t handle, const char* uniform, const glm::vec3& value);
    void Prism_Material_SetColor(uint64_t handle, const char* uniform, const glm::vec4& value);
    void Prism_Material_SetMatrix4(uint64_t handle, const char* uniform, const glm::mat4& value);
    void Prism_Material_SetTexture(uint64_t handle, const char* uniform, uint64_t textureHandle);
    void Prism_Material_SetKeyword(uint64_t handle, const char* name, bool enabled);
    bool Prism_Material_IsKeywordEnabled(uint64_t handle, const char* name);
#pragma endregion

} // namespace Prism::PythonScript
