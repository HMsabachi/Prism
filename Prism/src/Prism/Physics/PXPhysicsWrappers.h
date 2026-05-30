#pragma once

#include "Prism/Physics/PhysicsUtil.h"
#include "Prism/Scene/Components.h"

namespace Prism
{
    struct SceneParams;

    class PXPhysicsWrappers
    {
    public:
        static physx::PxScene* CreateScene(const SceneParams& sceneParams);
        static physx::PxRigidActor* CreateActor(const RigidBodyComponent& rigidbody, const glm::mat4& transform);
        static void SetCollisionFilters(const physx::PxRigidActor& actor, uint32_t actorGroup, uint32_t filters);

        static void AddBoxCollider(physx::PxRigidActor& actor, const physx::PxMaterial& material, const BoxColliderComponent& collider, const glm::vec3& size = glm::vec3(0.0F));
        static void AddSphereCollider(physx::PxRigidActor& actor, const physx::PxMaterial& material, const SphereColliderComponent& collider, const glm::vec3& size = glm::vec3(0.0F));
        static void AddCapsuleCollider(physx::PxRigidActor& actor, const physx::PxMaterial& material, const CapsuleColliderComponent& collider, const glm::vec3& size = glm::vec3(0.0F));
        static void AddMeshCollider(physx::PxRigidActor& actor, const physx::PxMaterial& material, MeshColliderComponent& collider, const glm::vec3& size = glm::vec3(0.0F));

        static physx::PxConvexMesh* CreateConvexMesh(MeshColliderComponent& collider);

        static physx::PxMaterial* CreateMaterial(const PhysicsMaterialComponent& material);

    private:
        static void Initialize();
        static void Shutdown();

        static void ConnectVisualDebugger();
        static void DisconnectVisualDebugger();

    private:
        friend class Physics;
    };
}
