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
    enum class KeyCode : uint16_t;
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
        bool Prism_Input_IsKeyPressed(KeyCode key);
        // Entity
        void Prism_Entity_GetTransform(uint64_t entityID, glm::mat4* outTransform);
        void Prism_Entity_SetTransform(uint64_t entityID, glm::mat4* inTransform);
        void Prism_Entity_CreateComponent(uint64_t entityID, Rolky::ReflectionType type);
        bool Prism_Entity_HasComponent(uint64_t entityID, Rolky::ReflectionType type);

        // Mesh
        void* Prism_MeshComponent_GetMesh(uint64_t entityID);
        void Prism_MeshComponent_SetMesh(uint64_t entityID, Ref<Mesh>* inMesh);
        Ref<Mesh>* Prism_Mesh_Constructor(Rolky::String filepath);
        void Prism_Mesh_Destructor(Ref<Mesh>* _this);
        Ref<Material>* Prism_Mesh_GetMaterial(Ref<Mesh>* inMesh);
        Ref<MaterialInstance>* Prism_Mesh_GetMaterialByIndex(Ref<Mesh>* inMesh, int32_t index);
        int32_t Prism_Mesh_GetMaterialCount(Ref<Mesh>* inMesh);
        void* Prism_MeshFactory_CreatePlane(float width, float height);

        // MaterialComponent
        Ref<MaterialInstance>* Prism_MaterialComponent_GetMaterial(uint64_t entityID);
        void Prism_MaterialComponent_SetMaterial(uint64_t entityID, Ref<MaterialInstance>* materialInstance);

        // Renderer
        // Texture2D
        void* Prism_Texture2D_Constructor(uint32_t width, uint32_t height);
        void Prism_Texture2D_Destructor(Ref<Texture2D>* _this);
        void Prism_Texture2D_SetData(Ref<Texture2D>* _this, Rolky::Array<glm::vec4> inData, int32_t count);

        // Material
        Ref<Material>* Prism_Material_Constructor(Rolky::String shaderName);
        void Prism_Material_Destructor(Ref<Material>* _this);
        void Prism_Material_SetFloat(Ref<Material>* _this, Rolky::String uniform, float value);
        void Prism_Material_SetTexture(Ref<Material>* _this, Rolky::String uniform, Ref<Texture2D>* texture);

        Ref<MaterialInstance>* Prism_MaterialInstance_Constructor(Ref<Material>* parent);
        void Prism_MaterialInstance_Destructor(Ref<MaterialInstance>* _this);
        void Prism_MaterialInstance_SetFloat(Ref<MaterialInstance>* _this, Rolky::String uniform, float value);
        void Prism_MaterialInstance_SetVector3(Ref<MaterialInstance>* _this, Rolky::String uniform, glm::vec3* value);
        void Prism_MaterialInstance_SetTexture(Ref<MaterialInstance>* _this, Rolky::String uniform, Ref<Texture2D>* texture);
    }
}
