#pragma once
#include <Rolky/Array.hpp>

namespace Rolky
{
    class String;
    class ReflectionType;
}

namespace Prism
{
    struct OverlapHitData;
    class Mesh;
    class Texture2D;
    class Material;
    struct RaycastHit;
    enum class KeyCode : uint16_t;
    enum class MouseButton : uint16_t;
    enum class CursorMode;

}

namespace Prism
{
    namespace Script
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
        void Prism_Log_LogMessage(LogLevel level, Rolky::String inFormattedMessage);
#pragma endregion
        // Time
        float Prism_Time_GetDeltaTime();
        float Prism_Time_GetUnscaledDeltaTime();
        float Prism_Time_GetTime();
        float Prism_Time_GetUnscaledTime();
        float Prism_Time_GetFixedDeltaTime();
        int64_t Prism_Time_GetFrameCount();
        void Prism_Time_SetTimeScale(float scale);
        float Prism_Time_GetTimeScale();
        void Prism_Time_SetFixedDeltaTime(float fixedDeltaTime);

        // Math
        float Prism_Noise_PerlinNoise(float x, float y);
        // Input
        Rolky::Bool32 Prism_Input_IsKeyPressed(KeyCode key);
        void Prism_Input_GetMousePosition(glm::vec2* outPosition);
        void Prism_Input_SetCursorMode(CursorMode mode);
        CursorMode Prism_Input_GetCursorMode();
        Rolky::Bool32 Prism_Input_IsMouseButtonPressed(MouseButton button);
        // Entity
        void Prism_Entity_GetTransform(uint64_t entityID, glm::mat4* outTransform);
        void Prism_Entity_SetTransform(uint64_t entityID, glm::mat4* inTransform);
        void Prism_Entity_CreateComponent(uint64_t entityID, Rolky::ReflectionType type);
        Rolky::Bool32 Prism_Entity_HasComponent(uint64_t entityID, Rolky::ReflectionType type);
        uint64_t Prism_Entity_FindEntityByTag(Rolky::String tag);
        void* Prism_Entity_AddBehaviour(uint64_t entityID, Rolky::String className);
        void Prism_Entity_RemoveBehaviour(uint64_t entityID, uint64_t behaviourID);
        void* Prism_Entity_GetBehaviour(uint64_t entityID, Rolky::ReflectionType type);
        Rolky::Bool32 Prism_Behaviour_GetEnabled(uint64_t behaviourID);
        void Prism_Behaviour_SetEnabled(uint64_t behaviourID, Rolky::Bool32 enabled);

        // TransformComponent
        void Prism_TransformComponent_GetPosition(uint64_t entityID, glm::vec3* outPosition);
        void Prism_TransformComponent_GetRotation(uint64_t entityID, glm::vec3* outRotation);
        void Prism_TransformComponent_GetScale(uint64_t entityID, glm::vec3* outScale);
        void Prism_TransformComponent_SetPosition(uint64_t entityID, glm::vec3 inPosition);
        void Prism_TransformComponent_SetRotation(uint64_t entityID, glm::vec3 inRotation);
        void Prism_TransformComponent_SetScale(uint64_t entityID, glm::vec3 inScale);
        // MeshRendererComponent
        void* Prism_MeshRendererComponent_GetMesh(uint64_t entityID);
        void Prism_MeshRendererComponent_SetMesh(uint64_t entityID, Ref<Mesh>* inMesh);
        void Prism_MeshRendererComponent_GetMaterial(uint64_t entityID, Ref<Material>** outMaterial, uint64_t index);
        void Prism_MeshRendererComponent_SetMaterial(uint64_t entityID, Ref<Material>* inMaterial, uint64_t index);
        uint64_t Prism_MeshRendererComponent_GetMaterialCount(uint64_t entityID);
        void Prism_MeshRendererComponent_GetMaterials(uint64_t entityID, void** outHandles);
        void Prism_MeshRendererComponent_SetMaterials(uint64_t entityID, void** inHandles, uint64_t count);
        // Mesh
        Ref<Mesh>* Prism_Mesh_Constructor(Rolky::String filepath);
        void Prism_Mesh_Destructor(Ref<Mesh>* _this);
        void* Prism_MeshFactory_CreatePlane(float width, float height);

        // Renderer
        // Texture2D
        void* Prism_Texture2D_Constructor(uint32_t width, uint32_t height);
        void Prism_Texture2D_Destructor(Ref<Texture2D>* _this);
        void Prism_Texture2D_SetData(Ref<Texture2D>* _this, Rolky::Array<glm::vec4> inData, int32_t count);

        // RigidBody2DComponent
        void Prism_RigidBody2DComponent_ApplyLinearImpulse(uint64_t entityID, glm::vec2* impulse, glm::vec2* offset, Rolky::Bool32 wake);
        void Prism_RigidBody2DComponent_GetLinearVelocity(uint64_t entityID, glm::vec2* outVelocity);
        void Prism_RigidBody2DComponent_SetLinearVelocity(uint64_t entityID, glm::vec2* velocity);

        // RigidBodyComponent
        void Prism_RigidBodyComponent_AddForce(uint64_t entityID, glm::vec3* force, int32_t forceMode);
        void Prism_RigidBodyComponent_AddTorque(uint64_t entityID, glm::vec3* torque, int32_t forceMode);
        void Prism_RigidBodyComponent_GetLinearVelocity(uint64_t entityID, glm::vec3* outVelocity);
        void Prism_RigidBodyComponent_SetLinearVelocity(uint64_t entityID, glm::vec3* velocity);
        void Prism_RigidBodyComponent_Rotate(uint64_t entityID, glm::vec3* rotation);
        uint32_t Prism_RigidBodyComponent_GetLayer(uint64_t entityID);
        float Prism_RigidBodyComponent_GetMass(uint64_t entityID);
        void Prism_RigidBodyComponent_SetMass(uint64_t entityID, float mass);
        uint32_t Prism_RigidBodyComponent_GetBodyType(uint64_t entityID);
        void Prism_RigidBodyComponent_GetAngularVelocity(uint64_t entityID, glm::vec3* outVelocity);
        void Prism_RigidBodyComponent_SetAngularVelocity(uint64_t entityID, glm::vec3* velocity);

        // Physics
        Rolky::Bool32 Prism_Physics_Raycast(glm::vec3* origin, glm::vec3* direction, float maxDistance, RaycastHit* hit);
        void Prism_Physics_OverlapBox(glm::vec3* origin, glm::vec3* halfSize, Rolky::Array<OverlapHitData>* outResults);
        void Prism_Physics_OverlapCapsule(glm::vec3* origin, float radius, float halfHeight, Rolky::Array<OverlapHitData>* outResults);
        void Prism_Physics_OverlapSphere(glm::vec3* origin, float radius, Rolky::Array<OverlapHitData>* outResults);
        void Prism_Physics_OverlapBoxNonAlloc(glm::vec3* origin, glm::vec3* halfSize, OverlapHitData* outBuffer, int32_t bufferSize, int32_t* outCount);
        void Prism_Physics_OverlapCapsuleNonAlloc(glm::vec3* origin, float radius, float halfHeight, OverlapHitData* outBuffer, int32_t bufferSize, int32_t* outCount);
        void Prism_Physics_OverlapSphereNonAlloc(glm::vec3* origin, float radius, OverlapHitData* outBuffer, int32_t bufferSize, int32_t* outCount);
        float Prism_Physics_GetGravity();
        void Prism_Physics_SetGravity(float gravity);

        // Material
        Ref<Material>* Prism_Material_Constructor(Rolky::String shaderName);
        void Prism_Material_GetDefaultMaterial(Ref<Material>** outMaterial);
        void Prism_Material_Destructor(Ref<Material>* _this);
        void Prism_Material_SetFloat(Ref<Material>* _this, Rolky::String uniform, float value);
        void Prism_Material_SetInt(Ref<Material>* _this, Rolky::String uniform, int value);
        void Prism_Material_SetBool(Ref<Material>* _this, Rolky::String uniform, Rolky::Bool32 value);
        void Prism_Material_SetVector2(Ref<Material>* _this, Rolky::String uniform, glm::vec2* value);
        void Prism_Material_SetColor3(Ref<Material>* _this, Rolky::String uniform, glm::vec3* value);
        void Prism_Material_SetColor(Ref<Material>* _this, Rolky::String uniform, glm::vec4* value);
        void Prism_Material_SetMatrix4(Ref<Material>* _this, Rolky::String uniform, glm::mat4* value);
        void Prism_Material_SetVector3(Ref<Material>* _this, Rolky::String uniform, glm::vec3* value);
        void Prism_Material_SetVector4(Ref<Material>* _this, Rolky::String uniform, glm::vec4* value);
        void Prism_Material_SetTexture(Ref<Material>* _this, Rolky::String uniform, Ref<Texture2D>* texture);
        void Prism_Material_SetKeyword(Ref<Material>* _this, Rolky::String name, Rolky::Bool32 enabled);
        Rolky::Bool32 Prism_Material_IsKeywordEnabled(Ref<Material>* _this, Rolky::String name);
    }
}
