#pragma once

#include "ISystem.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <unordered_map>
#include <entt/entt.hpp>

namespace Prism {

class Scene;
class Entity;

class PRISM_API InterpolationSystem : public ISystem
{
public:
    explicit InterpolationSystem(Scene* scene);

    void OnFixedUpdate(float dt) override;

    glm::mat4 GetInterpolatedWorldMatrix(Entity entity);

private:
    struct PrevTransform
    {
        glm::vec3 Position;
        glm::vec3 Rotation;
        glm::vec3 Scale;
    };

    glm::mat4 ComputeInterpolatedWorldMatrix(Entity entity, float alpha);

    Scene* m_Scene;
    std::unordered_map<entt::entity, PrevTransform> m_PrevStates;
};

}
