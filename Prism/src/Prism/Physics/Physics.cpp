#include "prpch.h"
#include "Physics.h"
#include "PhysicsLayer.h"
#include "PhysicsUtil.h"
#include "PXPhysicsWrappers.h"

#include "Prism/Scene/Components.h"
#include <PhysX/extensions/PxBroadPhaseExt.h>

namespace Prism {

    static physx::PxScene* s_Scene = nullptr;
    static std::vector<Entity> s_SimulatedEntities;
    static Entity* s_EntityStorageBuffer = nullptr;
    static uint32_t s_EntityBufferCount = 0;
    static int s_EntityStorageBufferPosition = 0;
    static PhysicsSettings s_Settings;

    void Physics::Init()
    {
        PXPhysicsWrappers::Initialize();
        PhysicsLayerManager::AddLayer("Default");
    }

    void Physics::Shutdown()
    {
        PXPhysicsWrappers::Shutdown();
    }

    void Physics::ExpandEntityBuffer(uint32_t entityCount)
    {
        if (s_EntityStorageBuffer != nullptr)
        {
            Entity* temp = new Entity[s_EntityBufferCount + entityCount];
            memcpy(temp, s_EntityStorageBuffer, s_EntityBufferCount * sizeof(Entity));

            for (uint32_t i = 0; i < s_EntityBufferCount; i++)
            {
                Entity& e = s_EntityStorageBuffer[i];
                RigidBodyComponent& rb = e.GetComponent<RigidBodyComponent>();

                if (rb.RuntimeActor)
                {
                    physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
                    actor->userData = &temp[rb.EntityBufferIndex];
                }
            }

            delete[] s_EntityStorageBuffer;
            s_EntityStorageBuffer = temp;
            s_EntityBufferCount += entityCount;
        }
        else
        {
            s_EntityStorageBuffer = new Entity[entityCount];
            s_EntityBufferCount = entityCount;
        }
    }

    void Physics::CreateScene()
    {
        PR_CORE_ASSERT(s_Scene == nullptr, "Scene already has a Physics Scene!");
        s_Scene = PXPhysicsWrappers::CreateScene();

        if (s_Settings.BroadphaseAlgorithm != BroadphaseType::AutomaticBoxPrune)
        {
            physx::PxBounds3* regionBounds = nullptr;
            physx::PxBounds3 globalBounds(ToPhysXVector(s_Settings.WorldBoundsMin), ToPhysXVector(s_Settings.WorldBoundsMax));
            uint32_t regionCount = physx::PxBroadPhaseExt::createRegionsFromWorldBounds(regionBounds, globalBounds, s_Settings.WorldBoundsSubdivisions);

            for (uint32_t i = 0; i < regionCount; i++)
            {
                physx::PxBroadPhaseRegion region;
                region.mBounds = regionBounds[i];
                s_Scene->addBroadPhaseRegion(region);
            }
        }
    }

    void Physics::CreateActor(Entity e)
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

        // Create Actor Body
        physx::PxRigidActor* actor = PXPhysicsWrappers::CreateActor(rigidbody, e.Transform().GetTransform());

        if (rigidbody.BodyType == RigidBodyComponent::Type::Dynamic)
            s_SimulatedEntities.push_back(e);

        Entity* entityStorage = &s_EntityStorageBuffer[s_EntityStorageBufferPosition];
        *entityStorage = e;
        actor->userData = (void*)entityStorage;
        rigidbody.RuntimeActor = actor;
        rigidbody.EntityBufferIndex = s_EntityStorageBufferPosition;
        s_EntityStorageBufferPosition++;

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

        if (!PhysicsLayerManager::IsLayerValid(rigidbody.Layer))
            rigidbody.Layer = 0;

        // Set collision filters
        PXPhysicsWrappers::SetCollisionFilters(*actor, rigidbody.Layer);

        s_Scene->addActor(*actor);
    }

    void Physics::SetGravity(float gravity)
    {
        s_Settings.Gravity.y = gravity;

        if (s_Scene)
            s_Scene->setGravity({ 0.0F, gravity, 0.0F });
    }

    float Physics::GetGravity()
    {
        return s_Settings.Gravity.y;
    }

    PhysicsSettings& Physics::GetSettings()
    {
        return s_Settings;
    }

    void Physics::Step(float dt)
    {
        s_Scene->simulate(dt);
        s_Scene->fetchResults(true);
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

    void* Physics::GetPhysicsScene()
    {
        return s_Scene;
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
