#include "prpch.h"
#include "Physics3DSystem.h"
#include "../Scene.h"
#include "../Entity.h"
#include "../Components.h"

#include "Prism/Physics/Physics3D.h"
#include "ScriptSystem.h"

namespace Prism {

    Physics3DSystem::Physics3DSystem(Scene* scene)
        : m_Scene(scene)
    {
        SceneParams sceneDesc;
        sceneDesc.Gravity = glm::vec3(0.0F, -9.81F, 0.0F);
        Physics3D::CreateScene(sceneDesc);
    }

    Physics3DSystem::~Physics3DSystem()
    {
    }

    void Physics3DSystem::OnFixedUpdate(float /*ts*/)
    {
        Physics3D::Simulate();
    }

    void Physics3DSystem::OnRuntimeStart()
    {
        {
            auto* ss = m_Scene->GetSystem<ScriptSystem>();
            Physics3D::SetCollisionCallbacks(
                [ss](Entity e) { if (ss) ss->OnCollisionBegin(e); },
                [ss](Entity e) { if (ss) ss->OnCollisionEnd(e); }
            );
        }

        {
            auto view = m_Scene->GetAllEntitiesWith<RigidBodyComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, m_Scene };
                Physics3D::CreateActor(e, view.size());
            }
        }
    }

    void Physics3DSystem::OnRuntimeStop()
    {
        Physics3D::DestroyScene();
    }

}
