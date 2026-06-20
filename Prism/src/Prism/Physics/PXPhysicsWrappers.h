#pragma once

#include <array>

#include "Prism/Physics/PhysicsUtil.h"
#include "Prism/Scene/Components.h"

#define OVERLAP_MAX_COLLIDERS 10

namespace Prism
{
    struct RaycastHit;

    class PhysicsErrorCallback : public physx::PxErrorCallback
    {
    public:
        virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override;
    };

    class ContactListener : public physx::PxSimulationEventCallback
    {
    public:
        virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override;
        virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override;
        virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override;
        virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
        virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
        virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override;
    };

    class PXPhysicsWrappers
    {
    public:
        static physx::PxScene* CreateScene();
        static physx::PxRigidActor* CreateActor(const RigidBodyComponent& rigidbody, const Transform& transform);
        static void SetCollisionFilters(const physx::PxRigidActor& actor, uint32_t physicsLayer);

        static void AddBoxCollider(physx::PxRigidActor& actor, const physx::PxMaterial& material, const BoxColliderComponent& collider, const glm::vec3& size = glm::vec3(0.0F));
        static void AddSphereCollider(physx::PxRigidActor& actor, const physx::PxMaterial& material, const SphereColliderComponent& collider, const glm::vec3& size = glm::vec3(0.0F));
        static void AddCapsuleCollider(physx::PxRigidActor& actor, const physx::PxMaterial& material, const CapsuleColliderComponent& collider, const glm::vec3& size = glm::vec3(0.0F));
        static void AddMeshCollider(physx::PxRigidActor& actor, const physx::PxMaterial& material, MeshColliderComponent& collider, const glm::vec3& size = glm::vec3(0.0F));

        static physx::PxConvexMesh* CreateConvexMesh(MeshColliderComponent& collider);

        static physx::PxMaterial* CreateMaterial(const PhysicsMaterialComponent& material);

        static bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit* hit);
        static bool OverlapBox(const glm::vec3& origin, const glm::vec3& halfSize, std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t* count);
        static bool OverlapCapsule(const glm::vec3& origin, float radius, float halfHeight, std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t* count);
        static bool OverlapSphere(const glm::vec3& origin, float radius, std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t* count);

    private:
        static void Initialize();
        static void Shutdown();

    private:
        friend class Physics;
    };
}
