#pragma once

#include "ISystem.h"
#include <entt/entt.hpp>

namespace physx {
class PxScene;
}

namespace Prism {

class Scene;

class PRISM_API Physics3DSystem : public ISystem {
public:
    explicit Physics3DSystem(Scene* scene);
    ~Physics3DSystem() override;

    void OnFixedUpdate(float ts) override;
    void OnRuntimeStart() override;
    void OnRuntimeStop() override;

private:
    void OnRigidBodyConstruct(entt::registry& registry, entt::entity entity);
    void OnRigidBodyDestroy(entt::registry& registry, entt::entity entity);

    Scene* m_Scene;
    physx::PxScene* m_PhysxScene = nullptr;
};

}
