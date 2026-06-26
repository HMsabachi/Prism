#pragma once

#include <array>

#include "Prism/Physics/PhysicsUtil.h"
#include "Prism/Scene/Components.h"

#define OVERLAP_MAX_COLLIDERS 10

namespace Prism
{
    class PhysicsActor;
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

        static void AddBoxCollider(PhysicsActor& actor, const physx::PxMaterial& material);
        static void AddSphereCollider(PhysicsActor& actor, const physx::PxMaterial& material);
        static void AddCapsuleCollider(PhysicsActor& actor, const physx::PxMaterial& material);
        static void AddMeshCollider(PhysicsActor& actor, const physx::PxMaterial& material);

        static std::vector<physx::PxTriangleMesh*> CreateTriangleMesh(MeshColliderComponent& collider, bool invalidateOld = false);
        static std::vector<physx::PxConvexMesh*> CreateConvexMesh(MeshColliderComponent& collider, bool invalidateOld = false);

        static bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit* hit);
        static bool OverlapBox(const glm::vec3& origin, const glm::vec3& halfSize, std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t* count);
        static bool OverlapCapsule(const glm::vec3& origin, float radius, float halfHeight, std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t* count);
        static bool OverlapSphere(const glm::vec3& origin, float radius, std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t* count);

    private:
        static void Initialize();
        static void Shutdown();
        static physx::PxPhysics& GetPhysics();
        static physx::PxAllocatorCallback& GetAllocator();

    private:
        friend class Physics;
        friend class PhysicsActor;
    };
}
