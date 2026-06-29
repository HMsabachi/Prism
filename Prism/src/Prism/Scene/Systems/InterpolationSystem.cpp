#include "prpch.h"
#include "InterpolationSystem.h"
#include "../Scene.h"
#include "../Entity.h"
#include "../Components.h"

#include "Prism/Core/Time.h"
#include "Prism/Physics/Physics.h"
#include "Prism/Physics/PhysicsActor.h"

#include <box2d/box2d.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Prism {

InterpolationSystem::InterpolationSystem(Scene* scene)
    : m_Scene(scene)
{
}

void InterpolationSystem::OnFixedUpdate(float /*dt*/)
{
    for (auto& actor : Physics::GetActors())
    {
        auto& tc = actor->GetEntity().Transformation();
        m_PrevStates[(entt::entity)actor->GetEntity()] = {
            tc.GetPosition(), tc.GetRotation(), tc.GetScale()
        };
    }

    auto view = m_Scene->GetAllEntitiesWith<RigidBody2DComponent>();
    for (auto entity : view)
    {
        Entity e = { entity, m_Scene };
        auto& rb2d = e.GetComponent<RigidBody2DComponent>();
        if (!rb2d.RuntimeBody) continue;

        auto& tc = e.Transformation();
        m_PrevStates[entity] = {
            tc.GetPosition(), tc.GetRotation(), tc.GetScale()
        };
    }
}

glm::mat4 InterpolationSystem::GetInterpolatedWorldMatrix(Entity entity)
{
    float alpha = Time::GetInterpolationAlpha();
    return ComputeInterpolatedWorldMatrix(entity, alpha);
}

glm::mat4 InterpolationSystem::ComputeInterpolatedWorldMatrix(Entity entity, float alpha)
{
    auto& tc = entity.Transformation();
    glm::vec3 pos, scale;
    glm::quat rot;

    auto it = m_PrevStates.find((entt::entity)entity);
    if (it != m_PrevStates.end())
    {
        glm::quat prevQuat = glm::quat(glm::radians(it->second.Rotation));
        glm::quat currQuat = glm::quat(glm::radians(tc.GetRotation()));
        pos   = glm::mix(it->second.Position, tc.GetPosition(), alpha);
        rot   = glm::slerp(prevQuat, currQuat, alpha);
        scale = glm::mix(it->second.Scale, tc.GetScale(), alpha);
    }
    else
    {
        pos   = tc.GetPosition();
        rot   = glm::quat(glm::radians(tc.GetRotation()));
        scale = tc.GetScale();
    }

    glm::mat4 local = glm::translate(glm::mat4(1.0f), pos)
                    * glm::toMat4(rot)
                    * glm::scale(glm::mat4(1.0f), scale);

    Entity parent = m_Scene->FindEntityByUUID(entity.GetParentUUID());
    if (parent)
    {
        glm::mat4 parentWorld = ComputeInterpolatedWorldMatrix(parent, alpha);
        return parentWorld * local;
    }

    return local;
}

}
