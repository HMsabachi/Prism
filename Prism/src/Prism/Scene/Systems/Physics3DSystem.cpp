#include "prpch.h"
#include "Physics3DSystem.h"
#include "../Scene.h"
#include "../Entity.h"
#include "../Components.h"

#include "Prism/Physics/Physics.h"
#include "ScriptSystem.h"

namespace Prism {

    Physics3DSystem::Physics3DSystem(Scene* scene)
        : m_Scene(scene)
    {
        SceneParams sceneDesc;
        sceneDesc.Gravity = glm::vec3(0.0F, -9.81F, 0.0F);
        Physics::CreateScene(sceneDesc);
    }

    Physics3DSystem::~Physics3DSystem()
    {
    }

    void Physics3DSystem::OnFixedUpdate(float /*ts*/)
    {
        Physics::Simulate();
    }

    void Physics3DSystem::OnRuntimeStart()
    {
        {
            auto* ss = m_Scene->GetSystem<ScriptSystem>();
            Physics::SetCollisionCallbacks(
                [ss](Entity e) { if (ss) ss->OnCollisionBegin(e); },
                [ss](Entity e) { if (ss) ss->OnCollisionEnd(e); }
            );
            Physics::SetTriggerCallbacks(
                [ss](Entity e) { if (ss) ss->OnTriggerBegin(e); },
                [ss](Entity e) { if (ss) ss->OnTriggerEnd(e); }
            );
        }

        {
            auto view = m_Scene->GetAllEntitiesWith<RigidBodyComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, m_Scene };
                Physics::CreateActor(e, view.size());
            }
        }
    }

    void Physics3DSystem::OnRuntimeStop()
    {
        Physics::DestroyScene();
    }

}
