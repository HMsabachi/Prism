#pragma once

#include "ISystem.h"
#include <memory>
#include <functional>

#include <entt/entt.hpp>

class b2World;

namespace Prism {

    class Scene;
    class Entity;

    class PRISM_API Physics2DSystem : public ISystem {
    public:
        explicit Physics2DSystem(Scene* scene);
        ~Physics2DSystem() override;

        void OnFixedUpdate(float ts) override;
        void OnRuntimeStart() override;
        void OnRuntimeStop() override;

        float GetGravity() const;
        void SetGravity(float gravity);

        std::function<void(uint64_t)> OnCollisionBegin;
        std::function<void(uint64_t)> OnCollisionEnd;

    private:
        class ContactListener;
        void OnRigidBody2DConstruct(entt::registry& registry, entt::entity entity);
        void OnRigidBody2DDestroy(entt::registry& registry, entt::entity entity);
        void OnBoxCollider2DConstruct(entt::registry& registry, entt::entity entity);
        void OnCircleCollider2DConstruct(entt::registry& registry, entt::entity entity);

        Scene* m_Scene;
        std::unique_ptr<b2World> m_World;
        std::unique_ptr<ContactListener> m_Listener;
    };

}
