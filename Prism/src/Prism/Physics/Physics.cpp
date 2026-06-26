#include "prpch.h"
#include "Physics.h"
#include "PhysicsActor.h"
#include "PhysicsLayer.h"
#include "PhysicsUtil.h"
#include "PXPhysicsWrappers.h"

#include "Prism/Scene/Components.h"
#include <PhysX/extensions/PxBroadPhaseExt.h>

namespace Prism {

    static physx::PxScene* s_Scene = nullptr;
    static std::vector<Ref<PhysicsActor>> s_Actors;
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

    void Physics::CreateScene()
    {
        PR_CORE_ASSERT(s_Scene == nullptr, "Scene already has a Physics Scene!");
        s_Scene = PXPhysicsWrappers::CreateScene();

        if (s_Settings.BroadphaseAlgorithm != BroadphaseType::AutomaticBoxPrune)
        {
            physx::PxBounds3* regionBounds = nullptr;
            physx::PxBounds3 globalBounds(ToPhysXVector(s_Settings.WorldBoundsMin), ToPhysXVector(s_Settings.WorldBoundsMax));
            uint32_t regionCount = physx::PxBroadPhaseExt::createRegionsFromWorldBounds(regionBounds, globalBounds, s_Settings.WorldBoundsSubdivisions);

            if (regionCount > 0 && regionBounds != nullptr)
            {
                for (uint32_t i = 0; i < regionCount; i++)
                {
                    physx::PxBroadPhaseRegion region;
                    region.mBounds = regionBounds[i];
                    s_Scene->addBroadPhaseRegion(region);
                }
                delete[] regionBounds;
            }
        }
    }

    Ref<PhysicsActor> Physics::CreateActor(Entity e)
    {
        Ref<PhysicsActor> actor = Ref<PhysicsActor>::Create(e);
        s_Actors.push_back(actor);
        actor->Spawn();
        return actor;
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

    Ref<PhysicsActor> Physics::GetActorForEntity(Entity entity)
    {
        for (auto& actor : s_Actors)
        {
            if (actor->GetEntity() == entity)
                return actor;
        }

        return nullptr;
    }

    PhysicsSettings& Physics::GetSettings()
    {
        return s_Settings;
    }

    // TODO: Physics Thread
    void Physics::Step(float dt)
    {
        s_Scene->simulate(dt);
        s_Scene->fetchResults(true);
    }

    void Physics::DestroyScene()
    {
        s_Actors.clear();

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

    std::vector<Ref<PhysicsActor>>& Physics::GetActors()
    {
        return s_Actors;
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
