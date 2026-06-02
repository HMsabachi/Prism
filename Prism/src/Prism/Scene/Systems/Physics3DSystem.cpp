#include "prpch.h"
#include "Physics3DSystem.h"
#include "../Scene.h"
#include "../Entity.h"
#include "../Components.h"

#include "Prism/Physics/Physics.h"
#include "Prism/Renderer/SceneRenderer.h"
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
        Physics::Step(dt);
    }

    void Physics3DSystem::OnRuntimeStart()
    {
        {
            SceneParams sceneDesc;
            sceneDesc.Gravity = glm::vec3(0.0F, -9.81F, 0.0F);
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

    void Physics3DSystem::SubmitColliderMeshes()
    {
        {
            auto view = m_Scene->GetAllEntitiesWith<BoxColliderComponent>();
            for (auto entity : view)
            {
                if (m_Scene->GetSelectedEntity() == entity)
                {
                    Entity e = { entity, m_Scene };
                    auto& collider = e.GetComponent<BoxColliderComponent>();
                    SceneRenderer::SubmitColliderMesh(collider, e.GetComponent<TransformComponent>().GetTransform());
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
                    SceneRenderer::SubmitColliderMesh(collider, e.GetComponent<TransformComponent>().GetTransform());
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
                    SceneRenderer::SubmitColliderMesh(collider, e.GetComponent<TransformComponent>().GetTransform());
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
                    SceneRenderer::SubmitColliderMesh(collider, e.GetComponent<TransformComponent>().GetTransform());
                }
            }
        }
    }

}
