#include "prpch.h"
#include "Physics.h"
#include "PXPhysicsWrappers.h"
#include "PhysicsUtil.h"

#include "Prism/Scene/Components.h"

PRISM_API void PrismConnectPhysXDebugger()
{
    Prism::Physics::ConnectVisualDebugger();
}

namespace Prism {

    static physx::PxScene* s_Scene = nullptr;
    static std::vector<Entity> s_SimulatedEntities;
    static Entity* s_EntityStorageBuffer = nullptr;
    static int s_EntityStorageBufferPosition = 0;

    void Physics::Init()
    {
        PXPhysicsWrappers::Initialize();
    }

    void Physics::Shutdown()
    {
        PXPhysicsWrappers::Shutdown();
    }

    void Physics::CreateScene(const SceneParams& params)
    {
        PR_CORE_ASSERT(s_Scene == nullptr, "Scene already has a Physics Scene!");
        s_Scene = PXPhysicsWrappers::CreateScene(params);
    }

    void Physics::CreateActor(Entity e, int entityCount)
    {
        if (!e.HasComponent<RigidBodyComponent>())
        {
            PR_CORE_WARN("Trying to create PhysX actor from a non-rigidbody actor!");
            return;
        }

        if (!e.HasComponent<PhysicsMaterialComponent>())
        {
            PR_CORE_WARN("Trying to create PhysX actor without a PhysicsMaterialComponent!");
            return;
        }

        RigidBodyComponent& rigidbody = e.GetComponent<RigidBodyComponent>();

        if (s_EntityStorageBuffer == nullptr)
            s_EntityStorageBuffer = new Entity[entityCount];

        // Create Actor Body
        physx::PxRigidActor* actor = PXPhysicsWrappers::CreateActor(rigidbody, e.Transform().GetTransform());
        s_SimulatedEntities.push_back(e);
        Entity* entityStorage = &s_EntityStorageBuffer[s_EntityStorageBufferPosition++];
        *entityStorage = e;
        actor->userData = (void*)entityStorage;
        rigidbody.RuntimeActor = actor;

        // Physics Material
        physx::PxMaterial* material = PXPhysicsWrappers::CreateMaterial(e.GetComponent<PhysicsMaterialComponent>());

        auto& transform = e.Transform();
        glm::vec3 scale = transform.Scale;

        // Add all colliders
        if (e.HasComponent<BoxColliderComponent>())
        {
            BoxColliderComponent& collider = e.GetComponent<BoxColliderComponent>();
            PXPhysicsWrappers::AddBoxCollider(*actor, *material, collider, scale);
        }

        if (e.HasComponent<SphereColliderComponent>())
        {
            SphereColliderComponent& collider = e.GetComponent<SphereColliderComponent>();
            PXPhysicsWrappers::AddSphereCollider(*actor, *material, collider, scale);
        }

        if (e.HasComponent<CapsuleColliderComponent>())
        {
            CapsuleColliderComponent& collider = e.GetComponent<CapsuleColliderComponent>();
            PXPhysicsWrappers::AddCapsuleCollider(*actor, *material, collider, scale);
        }

        if (e.HasComponent<MeshColliderComponent>())
        {
            MeshColliderComponent& collider = e.GetComponent<MeshColliderComponent>();
            PXPhysicsWrappers::AddMeshCollider(*actor, *material, collider, scale);
        }

        // Set collision filters
        if (rigidbody.BodyType == RigidBodyComponent::Type::Static)
        {
            PXPhysicsWrappers::SetCollisionFilters(*actor, (uint32_t)FilterGroup::Static, (uint32_t)FilterGroup::All);
        }
        else if (rigidbody.BodyType == RigidBodyComponent::Type::Dynamic)
        {
            PXPhysicsWrappers::SetCollisionFilters(*actor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All);
        }

        s_Scene->addActor(*actor);
    }

    void Physics::Simulate()
    {
        constexpr float stepSize = 0.016666660f;
        s_Scene->simulate(stepSize);
        s_Scene->fetchResults(true);

        for (Entity& e : s_SimulatedEntities)
        {
            auto& tc = e.Transform();
            RigidBodyComponent& rb = e.GetComponent<RigidBodyComponent>();
            physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);

            if (rb.BodyType == RigidBodyComponent::Type::Dynamic)
            {
                physx::PxTransform pxTransform = actor->getGlobalPose();
                tc.Position = glm::vec3(pxTransform.p.x, pxTransform.p.y, pxTransform.p.z);
                tc.Rotation = glm::quat(pxTransform.q.w, pxTransform.q.x, pxTransform.q.y, pxTransform.q.z);
            }
            else if (rb.BodyType == RigidBodyComponent::Type::Static)
            {
                actor->setGlobalPose(ToPhysXTransform(tc.GetTransform()));
            }
        }
    }

    void Physics::DestroyScene()
    {
        if (s_EntityStorageBuffer)
        {
            delete[] s_EntityStorageBuffer;
            s_EntityStorageBuffer = nullptr;
        }
        s_EntityStorageBufferPosition = 0;
        s_SimulatedEntities.clear();

        if (s_Scene)
        {
            s_Scene->release();
            s_Scene = nullptr;
        }
    }

    void Physics::ConnectVisualDebugger()
    {
        PXPhysicsWrappers::ConnectVisualDebugger();
    }

    void Physics::DisconnectVisualDebugger()
    {
        PXPhysicsWrappers::DisconnectVisualDebugger();
    }

    void Physics::SetCollisionCallbacks(CollisionCallback begin, CollisionCallback end)
    {
        SetContactCallbacks(begin, end);
    }

    void Physics::SetTriggerCallbacks(TriggerCallback begin, TriggerCallback end)
    {
        SetContactTriggerCallbacks(begin, end);
    }

}
