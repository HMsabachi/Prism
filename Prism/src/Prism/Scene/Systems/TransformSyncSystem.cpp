#include "prpch.h"
#include "TransformSyncSystem.h"
#include "../Scene.h"
#include "../Entity.h"
#include "../Components.h"

#include "Prism/Physics/PhysicsUtil.h"

#include <box2d/box2d.h>
#include <PhysX/PxPhysicsAPI.h>

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
            auto view = m_Scene->GetAllEntitiesWith<RigidBodyComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, m_Scene };
                auto& rb = e.GetComponent<RigidBodyComponent>();
                auto* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
                if (!actor) continue;

                auto& tc = e.Transform();
                if (!tc.TransformDirty) continue;

                actor->setGlobalPose(ToPhysXTransform(tc.GetTransform()));
                tc.TransformDirty = false;
            }
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

                auto& tc = e.Transform();
                if (!tc.TransformDirty) continue;

                float angle = glm::eulerAngles(tc.Rotation).z;
                body->SetTransform(b2Vec2(tc.Position.x, tc.Position.y), angle);
                tc.TransformDirty = false;
            }
        }
    }

    void TransformSyncSystem::OnPreUpdate(float dt)
    {
        PR_PROFILE_FUNCTION();

        // 3D: PhysX -> ECS
        {
            auto view = m_Scene->GetAllEntitiesWith<RigidBodyComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, m_Scene };
                auto& rb = e.GetComponent<RigidBodyComponent>();
                if (rb.BodyType != RigidBodyComponent::Type::Dynamic) continue;

                auto* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
                if (!actor) continue;

                auto& tc = e.Transform();
                physx::PxTransform pxTransform = actor->getGlobalPose();
                tc.Position = glm::vec3(pxTransform.p.x, pxTransform.p.y, pxTransform.p.z);
                tc.Rotation = glm::quat(pxTransform.q.w, pxTransform.q.x, pxTransform.q.y, pxTransform.q.z);
                tc.TransformDirty = false;
            }
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

                auto& tc = e.Transform();
                const auto& position = body->GetPosition();
                tc.Position.x = position.x;
                tc.Position.y = position.y;
                tc.Rotation = glm::quat(glm::vec3(0.0f, 0.0f, body->GetAngle()));
                tc.TransformDirty = false;
            }
        }
    }

}
