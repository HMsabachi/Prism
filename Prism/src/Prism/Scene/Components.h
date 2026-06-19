#pragma once

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "Prism/Core/UUID.h"

#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Renderer/Material.h"
#include "Prism/Scene/SceneCamera.h"
#include "glm/gtx/quaternion.hpp"
#include "Scripting/CSharp/CSharpField.h"
#include "Scripting/Python/PythonField.h"


namespace Prism {

    struct IDComponent
    {
        UUID ID = 0;
    };

    struct TagComponent
    {
        std::string Tag;
        TagComponent() = default;
        TagComponent(const TagComponent& other)
            : Tag(other.Tag) {
        }
        TagComponent(const std::string& tag)
            : Tag(tag) {
        }
        operator std::string& () { return Tag; }
        operator const std::string& () const { return Tag; }
    };

    struct TransformComponent
    {
        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
        glm::quat Rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3 Scale    = { 1.0f, 1.0f, 1.0f };
        bool TransformDirty = false;

        TransformComponent() = default;
        TransformComponent(const TransformComponent& other) = default;

        void SetPosition(const glm::vec3& pos) { Position = pos; TransformDirty = true; }
        void SetRotation(const glm::quat& rot) { Rotation = rot; TransformDirty = true; }
        void SetScale(const glm::vec3& scl)    { Scale = scl;    TransformDirty = true; }

        glm::mat4 GetTransform() const
        {
            return glm::translate(glm::mat4(1.0f), Position)
                 * glm::toMat4(Rotation)
                 * glm::scale(glm::mat4(1.0f), Scale);
        }

        void SetTransform(const glm::mat4& mat)
        {
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(mat, Scale, Rotation, Position, skew, perspective);
        }
    };

    struct MeshRendererComponent
    {
        Ref<Prism::Mesh> Mesh;
        std::vector<Ref<Material>> Materials;

        MeshRendererComponent() = default;
        MeshRendererComponent(const MeshRendererComponent&) = default;
        MeshRendererComponent(const Ref<Prism::Mesh>& mesh) { SetMesh(mesh); }

        void SetMesh(const Ref<Prism::Mesh>& mesh)
        {
            Mesh = mesh;
            if (mesh)
                Materials.resize(mesh->GetSubmeshes().size(), nullptr);
            else
                Materials.clear();
        }

        void SetMaterial(uint32_t index, Ref<Material> material)
        {
            if (index < Materials.size())
                Materials[index] = material;
        }

        void SetMaterials(const std::vector<Ref<Material>>& materials)
        {
            Materials = materials;
        }
    };

    struct CSharpBehaviourBinding
    {
        UUID BehaviourID = 0;
        UUID ClassID = 0;
        uint16_t LifecycleMask = 0;
        bool Enabled = true;
        std::unordered_map<uint32_t, CSharpField> Fields;
    };

    struct PythonBehaviourBinding
    {
        UUID BehaviourID = 0;
        UUID ClassID = 0;
        uint16_t LifecycleMask = 0;
        bool Enabled = true;
        std::unordered_map<uint32_t, PythonField> Fields;
    };

    struct CSharpScriptComponent
    {
        UUID ScriptID = 0;
        std::unordered_map<UUID, CSharpBehaviourBinding> Behaviours;
    };

    struct PythonScriptComponent
    {
        UUID ScriptID = 0;
        std::unordered_map<UUID, PythonBehaviourBinding> Behaviours;
    };

    struct CameraComponent
    {
        //OrthographicCamera Camera;
        SceneCamera Camera;
        bool Primary = true;

        CameraComponent() = default;
        CameraComponent(const CameraComponent& other)
            : Camera(other.Camera), Primary(other.Primary) {
        }

        operator SceneCamera& () { return Camera; }
        operator const SceneCamera& () const { return Camera; }
    };

    struct SpriteRendererComponent
    {
        glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        Ref<Texture2D> Texture;
        float TilingFactor = 1.0f;

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent& other)
            : Color(other.Color), Texture(other.Texture), TilingFactor(other.TilingFactor) {
        }
    };

    // MaterialComponent removed — materials now live in MeshComponent.Materials[]
    struct RigidBody2DComponent
    {
        enum class Type { Static, Dynamic, Kinematic };
        Type BodyType = Type::Static;
        bool FixedRotation = false;

        // Storage for runtime
        void* RuntimeBody = nullptr;

        RigidBody2DComponent() = default;
        RigidBody2DComponent(const RigidBody2DComponent& other)
            : BodyType(other.BodyType), FixedRotation(other.FixedRotation), RuntimeBody(nullptr) {
        }
    };

    struct BoxCollider2DComponent
    {
        glm::vec2 Offset = { 0.0f, 0.0f };
        glm::vec2 Size = { 1.0f, 1.0f };

        float Density = 1.0f;
        float Friction = 1.0f;

        // Storage for runtime
        void* RuntimeFixture = nullptr;

        BoxCollider2DComponent() = default;
        BoxCollider2DComponent(const BoxCollider2DComponent& other)
            : Offset(other.Offset), Size(other.Size), Density(other.Density), Friction(other.Friction), RuntimeFixture(nullptr) {
        }
    };

    struct CircleCollider2DComponent
    {
        glm::vec2 Offset = { 0.0f, 0.0f };
        float Radius = 1.0f;

        float Density = 1.0f;
        float Friction = 1.0f;

        // Storage for runtime
        void* RuntimeFixture = nullptr;

        CircleCollider2DComponent() = default;
        CircleCollider2DComponent(const CircleCollider2DComponent& other)
            : Offset(other.Offset), Radius(other.Radius), Density(other.Density), Friction(other.Friction), RuntimeFixture(nullptr) {
        }
    };

    struct RigidBodyComponent
    {
        enum class Type { Static, Dynamic };
        Type BodyType = Type::Static;
        float Mass = 1.0F;
        bool IsKinematic = false;
        uint32_t Layer = 0;

        bool LockPositionX = false;
        bool LockPositionY = false;
        bool LockPositionZ = false;
        bool LockRotationX = false;
        bool LockRotationY = false;
        bool LockRotationZ = false;

        void* RuntimeActor = nullptr;
        int32_t EntityBufferIndex = -1;

        RigidBodyComponent() = default;
        RigidBodyComponent(const RigidBodyComponent& other) = default;
    };

    // TODO: This will eventually be a resource, but that requires object referencing through the editor
    struct PhysicsMaterialComponent
    {
        float StaticFriction = 1.0F;
        float DynamicFriction = 1.0F;
        float Bounciness = 1.0F;

        PhysicsMaterialComponent() = default;
        PhysicsMaterialComponent(const PhysicsMaterialComponent& other) = default;
    };

    struct BoxColliderComponent
    {
        glm::vec3 Size = { 1.0F, 1.0F, 1.0F };
        glm::vec3 Offset = { 0.0F, 0.0F, 0.0F };

        bool IsTrigger = false;

        // The mesh that will be drawn in the editor to show the collision bounds
        Ref<Mesh> DebugMesh;

        BoxColliderComponent() = default;
        BoxColliderComponent(const BoxColliderComponent& other) = default;
    };

    struct SphereColliderComponent
    {
        float Radius = 0.5f;
        bool IsTrigger = false;

        // The mesh that will be drawn in the editor to show the collision bounds
        Ref<Mesh> DebugMesh;

        SphereColliderComponent() = default;
        SphereColliderComponent(const SphereColliderComponent& other) = default;
    };

    struct CapsuleColliderComponent
    {
        float Radius = 0.5F;
        float Height = 1.0F;
        bool IsTrigger = false;

        Ref<Mesh> DebugMesh;

        CapsuleColliderComponent() = default;
        CapsuleColliderComponent(const CapsuleColliderComponent& other) = default;
    };

    struct MeshColliderComponent
    {
        Ref<Mesh> CollisionMesh;
        Ref<Mesh> ProcessedMesh;
        bool IsTrigger = false;

        MeshColliderComponent() = default;
        MeshColliderComponent(const MeshColliderComponent& other) = default;
        MeshColliderComponent(const Ref<Mesh>& mesh)
            : CollisionMesh(mesh)
        {
        }

        operator Ref<Mesh>() { return CollisionMesh; }
    };

}
