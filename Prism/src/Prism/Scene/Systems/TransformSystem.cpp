#include "prpch.h"
#include "TransformSystem.h"
#include "../Scene.h"
#include "../Entity.h"
#include "../Components.h"

#include "Prism/Physics/Physics.h"
#include "Prism/Physics/PhysicsActor.h"
#include "Prism/Physics/PhysicsUtil.h"

#include <box2d/box2d.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace Prism {
    TransformSystem::TransformSystem(Scene* scene)
        : m_Scene(scene)
    {
    }
    void TransformSystem::OnFixedUpdate(float /*dt*/)
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

                auto& tc = e.Transformation();
                if (!tc.IsPhysicsDirty()) continue;

                const auto pos = tc.GetPosition();
                float angle = glm::radians(tc.GetRotation().z);
                body->SetTransform(b2Vec2(pos.x, pos.y), angle);
                tc.MarkPhysicsClean();
            }
        }
    }

    void TransformSystem::OnPreUpdate(float dt)
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

                auto& tc = e.Transformation();
                const auto& position = body->GetPosition();
                const auto prevPos = tc.GetPosition();
                tc.SetPosition(glm::vec3(position.x, position.y, prevPos.z));
                tc.SetRotation(glm::vec3(0.0f, 0.0f, glm::degrees(body->GetAngle())));
                tc.MarkPhysicsClean();
            }
        }
    }

    void TransformSystem::OnRender(float /*dt*/)
    {
        auto view = m_Scene->GetRegistry().view<TransformComponent>();
        for (auto entity : view)
        {
            auto& transformComponent = view.get<TransformComponent>(entity);
            WorldTRS world = ComputeWorldTRS(m_Scene, Entity(entity, m_Scene));

            transformComponent.Up = glm::normalize(glm::rotate(world.Rotation, glm::vec3(0.0F, 1.0F, 0.0F)));
            transformComponent.Right = glm::normalize(glm::rotate(world.Rotation, glm::vec3(1.0F, 0.0F, 0.0F)));
            transformComponent.Forward = glm::normalize(glm::rotate(world.Rotation, glm::vec3(0.0F, 0.0F, -1.0F)));
        }
    }

    struct WorldTRS
    {
        glm::vec3 Position;
        glm::quat Rotation;
        glm::vec3 Scale;
    };

    static WorldTRS ComputeWorldTRS(Scene* scene, Entity entity)
    {
        auto& tc = entity.Transformation();
        WorldTRS result;
        result.Position = tc.GetPosition();
        result.Rotation = glm::quat(glm::radians(tc.GetRotation()));
        result.Scale    = tc.GetScale();

        Entity parent = scene->FindEntityByUUID(entity.GetParentUUID());
        if (parent)
        {
            WorldTRS p = ComputeWorldTRS(scene, parent);
            result.Position = p.Position + p.Rotation * (p.Scale * result.Position);
            result.Rotation = p.Rotation * result.Rotation;
            result.Scale    = p.Scale * result.Scale;
        }
        return result;
    }

    DecomposedTransform TransformSystem::GetWorldDecomposed(Entity entity)
    {
        WorldTRS w = ComputeWorldTRS(m_Scene, entity);
        DecomposedTransform r;
        r.Position = w.Position;
        r.Rotation = glm::degrees(glm::eulerAngles(w.Rotation));
        r.Scale    = w.Scale;
        return r;
    }

    glm::mat4 TransformSystem::GetWorldTransformMatrix(Entity entity)
    {
        WorldTRS w = ComputeWorldTRS(m_Scene, entity);
        return glm::translate(glm::mat4(1.0F), w.Position)
             * glm::toMat4(w.Rotation)
             * glm::scale(glm::mat4(1.0F), w.Scale);
    }

    glm::vec3 TransformSystem::GetWorldPosition(Entity entity)
    {
        return GetWorldDecomposed(entity).Position;
    }

    glm::vec3 TransformSystem::GetWorldRotation(Entity entity)
    {
        return GetWorldDecomposed(entity).Rotation;
    }

    glm::vec3 TransformSystem::GetWorldScale(Entity entity)
    {
        return GetWorldDecomposed(entity).Scale;
    }

    glm::vec3 TransformSystem::GetLocalPosition(Entity entity)
    {
        return entity.Transformation().GetPosition();
    }

    glm::vec3 TransformSystem::GetLocalRotation(Entity entity)
    {
        return entity.Transformation().GetRotation();
    }

    glm::vec3 TransformSystem::GetLocalScale(Entity entity)
    {
        return entity.Transformation().GetScale();
    }

    void TransformSystem::SetWorldPosition(Entity entity, const glm::vec3& position)
    {
        Entity parent = m_Scene->FindEntityByUUID(entity.GetParentUUID());
        if (parent)
        {
            WorldTRS p = ComputeWorldTRS(m_Scene, parent);
            glm::vec3 localPos = glm::inverse(p.Rotation) * ((position - p.Position) / p.Scale);
            entity.Transformation().SetPosition(localPos);
        }
        else
        {
            entity.Transformation().SetPosition(position);
        }
    }

    void TransformSystem::SetWorldRotation(Entity entity, const glm::vec3& rotation)
    {
        glm::quat worldQuat = glm::quat(glm::radians(rotation));

        Entity parent = m_Scene->FindEntityByUUID(entity.GetParentUUID());
        if (parent)
        {
            WorldTRS p = ComputeWorldTRS(m_Scene, parent);
            glm::quat localQuat = glm::inverse(p.Rotation) * worldQuat;
            entity.Transformation().SetRotation(glm::degrees(glm::eulerAngles(localQuat)));
        }
        else
        {
            entity.Transformation().SetRotation(rotation);
        }
    }

    void TransformSystem::SetWorldScale(Entity entity, const glm::vec3& scale)
    {
        Entity parent = m_Scene->FindEntityByUUID(entity.GetParentUUID());
        if (parent)
        {
            WorldTRS p = ComputeWorldTRS(m_Scene, parent);
            entity.Transformation().SetScale(scale / p.Scale);
        }
        else
        {
            entity.Transformation().SetScale(scale);
        }
    }

    void TransformSystem::SetLocalPosition(Entity entity, const glm::vec3& position)
    {
        entity.Transformation().SetPosition(position);
    }

    void TransformSystem::SetLocalRotation(Entity entity, const glm::vec3& rotation)
    {
        entity.Transformation().SetRotation(rotation);
    }

    void TransformSystem::SetLocalScale(Entity entity, const glm::vec3& scale)
    {
        entity.Transformation().SetScale(scale);
    }

}
