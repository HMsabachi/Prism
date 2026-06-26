#include "prpch.h"
#include "TransformSyncSystem.h"
#include "../Scene.h"
#include "../Entity.h"
#include "../Components.h"

#include "Prism/Physics/Physics.h"
#include "Prism/Physics/PhysicsActor.h"
#include "Prism/Physics/PhysicsUtil.h"

#include <box2d/box2d.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Prism {

    TransformSyncSystem::TransformSyncSystem(Scene* scene)
        : m_Scene(scene)
    {
    }

    void TransformSyncSystem::OnFixedUpdate(float /*dt*/)
    {
        PR_PROFILE_FUNCTION();

        // 3D: ECS -> PhysX
        {
            for (auto& actor : Physics::GetActors())
                actor->SyncToPhysX();
        }

        // 2D: ECS -> Box2D
        {
            auto view = m_Scene->GetAllEntitiesWith<RigidBody2DComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, m_Scene };
                auto& rb2d = e.GetComponent<RigidBody2DComponent>();
                auto* body = static_cast<b2Body*>(rb2d.RuntimeBody);
                if (!body) continue;

                auto& transform = e.Transformation();
                if (!transform.IsPhysicsDirty()) continue;

                const auto& pos = transform.GetPosition();
                float angle = glm::radians(transform.GetRotation().z);
                body->SetTransform(b2Vec2(pos.x, pos.y), angle);
                transform.MarkPhysicsClean();
            }
        }
    }

    void TransformSyncSystem::OnPreUpdate(float dt)
    {
        PR_PROFILE_FUNCTION();

        // 3D: PhysX -> ECS
        {
            for (auto& actor : Physics::GetActors())
                actor->SyncFromPhysX();
        }

        // 2D: Box2D -> ECS
        {
            auto view = m_Scene->GetAllEntitiesWith<RigidBody2DComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, m_Scene };
                auto& rb2d = e.GetComponent<RigidBody2DComponent>();
                auto* body = static_cast<b2Body*>(rb2d.RuntimeBody);
                if (!body) continue;

                auto& transform = e.Transformation();
                const auto& position = body->GetPosition();
                const auto& prevPos = transform.GetPosition();
                transform.SetPosition(glm::vec3(position.x, position.y, prevPos.z));
                transform.SetRotation(glm::vec3(0.0f, 0.0f, glm::degrees(body->GetAngle())));
                transform.MarkPhysicsClean();
            }
        }
    }

}
