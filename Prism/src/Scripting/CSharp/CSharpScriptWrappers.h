#pragma once
#include <Rolky/Array.hpp>

namespace Rolky
{
    struct String;
    struct ReflectionType;
}

namespace Prism
{
    class Mesh;
    class Texture2D;
    class Material;
    class MaterialInstance;
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

        // Mesh
        void* Prism_MeshComponent_GetMesh(uint64_t entityID);
        void Prism_MeshComponent_SetMesh(uint64_t entityID, Ref<Mesh>* inMesh);
        Ref<Mesh>* Prism_Mesh_Constructor(Rolky::String filepath);
        void Prism_Mesh_Destructor(Ref<Mesh>* _this);
        Ref<Material>* Prism_Mesh_GetMaterial(Ref<Mesh>* inMesh);
        Ref<MaterialInstance>* Prism_Mesh_GetMaterialByIndex(Ref<Mesh>* inMesh, int32_t index);
        int32_t Prism_Mesh_GetMaterialCount(Ref<Mesh>* inMesh);
        void Prism_Mesh_SetMaterialByIndex(Ref<Mesh>* inMesh, int32_t index, Ref<MaterialInstance>* material);
        void Prism_Mesh_SetOverrideMaterial(Ref<Mesh>* inMesh, Ref<MaterialInstance>* material);
        Ref<MaterialInstance>* Prism_Mesh_GetOverrideMaterial(Ref<Mesh>* inMesh);
        void* Prism_MeshFactory_CreatePlane(float width, float height);

        // MaterialComponent
        Ref<MaterialInstance>* Prism_MaterialComponent_GetMaterial(uint64_t entityID);
        void Prism_MaterialComponent_SetMaterial(uint64_t entityID, Ref<MaterialInstance>* materialInstance);

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

        // Physics
        Rolky::Bool32 Prism_Physics_Raycast(glm::vec3* origin, glm::vec3* direction, float maxDistance, RaycastHit* hit);

        // Material
        Ref<Material>* Prism_Material_Constructor(Rolky::String shaderName);
        void Prism_Material_Destructor(Ref<Material>* _this);
        void Prism_Material_SetFloat(Ref<Material>* _this, Rolky::String uniform, float value);
        void Prism_Material_SetTexture(Ref<Material>* _this, Rolky::String uniform, Ref<Texture2D>* texture);
        void Prism_Material_SetKeyword(Ref<Material>* _this, Rolky::String name, Rolky::Bool32 enabled);
        Rolky::Bool32 Prism_Material_IsKeywordEnabled(Ref<Material>* _this, Rolky::String name);

        Ref<MaterialInstance>* Prism_MaterialInstance_Constructor(Ref<Material>* parent);
        void Prism_MaterialInstance_Destructor(Ref<MaterialInstance>* _this);
        void Prism_MaterialInstance_SetFloat(Ref<MaterialInstance>* _this, Rolky::String uniform, float value);
        void Prism_MaterialInstance_SetVector3(Ref<MaterialInstance>* _this, Rolky::String uniform, glm::vec3* value);
        void Prism_MaterialInstance_SetVector4(Ref<MaterialInstance>* _this, Rolky::String uniform, glm::vec4* value);
        void Prism_MaterialInstance_SetTexture(Ref<MaterialInstance>* _this, Rolky::String uniform, Ref<Texture2D>* texture);
        void Prism_MaterialInstance_SetKeyword(Ref<MaterialInstance>* _this, Rolky::String name, Rolky::Bool32 enabled);
        Rolky::Bool32 Prism_MaterialInstance_IsKeywordEnabled(Ref<MaterialInstance>* _this, Rolky::String name);
    }
}
