#include "prpch.h"
#include "Physics3DSystem.h"
#include "../Scene.h"
#include "../Entity.h"
#include "../Components.h"

#include "Prism/Physics/Physics.h"
#include "RenderSystem.h"
#include "ScriptSystem.h"

namespace Prism {

    Physics3DSystem::Physics3DSystem(Scene* scene)
        : m_Scene(scene)
    {
    }

    Physics3DSystem::~Physics3DSystem()
    {
        Physics::DestroyScene();
    }

    void Physics3DSystem::OnFixedUpdate(float dt)
    {
        PR_PROFILE_FUNCTION();
        Physics::Step(dt);
    }

    void Physics3DSystem::OnRuntimeStart()
    {
        {
            SceneParams sceneDesc;
            Physics::CreateScene(sceneDesc);
        }

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
            Physics::ExpandEntityBuffer((uint32_t)view.size());
            for (auto entity : view)
            {
                Entity e = { entity, m_Scene };
                Physics::CreateActor(e);
            }
        }
    }

    void Physics3DSystem::OnRuntimeStop()
    {
        Physics::DestroyScene();
    }

    void Physics3DSystem::SubmitColliderMeshes()
    {
        auto* rs = m_Scene->GetSystem<RenderSystem>();
        if (!rs) return;

        {
            auto view = m_Scene->GetAllEntitiesWith<BoxColliderComponent>();
            for (auto entity : view)
            {
                if (m_Scene->GetSelectedEntity() == entity)
                {
                    Entity e = { entity, m_Scene };
                    auto& collider = e.GetComponent<BoxColliderComponent>();
                    rs->SubmitDebugMesh(collider.DebugMesh,
                        glm::translate(e.GetComponent<TransformComponent>().GetTransform(), collider.Offset));
                }
            }
        }

        {
            auto view = m_Scene->GetAllEntitiesWith<SphereColliderComponent>();
            for (auto entity : view)
            {
                if (m_Scene->GetSelectedEntity() == entity)
                {
                    Entity e = { entity, m_Scene };
                    auto& collider = e.GetComponent<SphereColliderComponent>();
                    rs->SubmitDebugMesh(collider.DebugMesh,
                        e.GetComponent<TransformComponent>().GetTransform());
                }
            }
        }

        {
            auto view = m_Scene->GetAllEntitiesWith<CapsuleColliderComponent>();
            for (auto entity : view)
            {
                if (m_Scene->GetSelectedEntity() == entity)
                {
                    Entity e = { entity, m_Scene };
                    auto& collider = e.GetComponent<CapsuleColliderComponent>();
                    rs->SubmitDebugMesh(collider.DebugMesh,
                        e.GetComponent<TransformComponent>().GetTransform());
                }
            }
        }

        {
            auto view = m_Scene->GetAllEntitiesWith<MeshColliderComponent>();
            for (auto entity : view)
            {
                if (m_Scene->GetSelectedEntity() == entity)
                {
                    Entity e = { entity, m_Scene };
                    auto& collider = e.GetComponent<MeshColliderComponent>();
                    rs->SubmitDebugMesh(collider.ProcessedMesh,
                        e.GetComponent<TransformComponent>().GetTransform());
                }
            }
        }
    }

}
