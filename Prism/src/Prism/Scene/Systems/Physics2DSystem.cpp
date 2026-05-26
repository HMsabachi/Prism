#include "prpch.h"
#include "Physics2DSystem.h"
#include "../Scene.h"
#include "../Entity.h"
#include "../Components.h"

#include <box2d/box2d.h>

namespace Prism {

    class Physics2DSystem::ContactListener : public b2ContactListener {
    public:
        Physics2DSystem* System = nullptr;

        void BeginContact(b2Contact* contact) override
        {
            uint64_t aID = (uint64_t)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
            uint64_t bID = (uint64_t)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

            if (System)
            {
                if (System->OnCollisionBegin)
                {
                    System->OnCollisionBegin(aID);
                    System->OnCollisionBegin(bID);
                }
            }
        }

        void EndContact(b2Contact* contact) override
        {
            uint64_t aID = (uint64_t)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
            uint64_t bID = (uint64_t)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

            if (System)
            {
                if (System->OnCollisionEnd)
                {
                    System->OnCollisionEnd(aID);
                    System->OnCollisionEnd(bID);
                }
            }
        }
    };

    Physics2DSystem::Physics2DSystem(Scene* scene)
        : m_Scene(scene)
        , m_World(std::make_unique<b2World>(b2Vec2{ 0.0f, -9.8f }))
        , m_Listener(std::make_unique<ContactListener>())
    {
        m_Listener->System = this;
        m_World->SetContactListener(m_Listener.get());
    }

    Physics2DSystem::~Physics2DSystem() = default;

    void Physics2DSystem::OnFixedUpdate(float ts)
    {
        int32_t velocityIterations = 6;
        int32_t positionIterations = 2;
        m_World->Step(ts, velocityIterations, positionIterations);

        auto view = m_Scene->GetAllEntitiesWith<RigidBody2DComponent>();
        for (auto entity : view)
        {
            Entity e = { entity, m_Scene };
            auto& tc = e.Transform();
            auto& rb2d = e.GetComponent<RigidBody2DComponent>();
            b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);
            if (!body) continue;

            auto& position = body->GetPosition();
            tc.Position.x = position.x;
            tc.Position.y = position.y;
            tc.Rotation = glm::quat({ 0.0f, 0.0f, body->GetAngle() });
        }
    }

    void Physics2DSystem::OnRuntimeStart()
    {
        auto& registry = m_Scene->GetRegistry();
        registry.on_construct<RigidBody2DComponent>().connect<&Physics2DSystem::OnRigidBody2DConstruct>(this);
        registry.on_destroy<RigidBody2DComponent>().connect<&Physics2DSystem::OnRigidBody2DDestroy>(this);
        registry.on_construct<BoxCollider2DComponent>().connect<&Physics2DSystem::OnBoxCollider2DConstruct>(this);
        registry.on_construct<CircleCollider2DComponent>().connect<&Physics2DSystem::OnCircleCollider2DConstruct>(this);

        {
            auto view = registry.view<RigidBody2DComponent>();
            for (auto entity : view)
                OnRigidBody2DConstruct(registry, entity);
        }
        {
            auto view = registry.view<BoxCollider2DComponent>();
            for (auto entity : view)
                OnBoxCollider2DConstruct(registry, entity);
        }
        {
            auto view = registry.view<CircleCollider2DComponent>();
            for (auto entity : view)
                OnCircleCollider2DConstruct(registry, entity);
        }
    }

    void Physics2DSystem::OnRuntimeStop()
    {
        auto& registry = m_Scene->GetRegistry();
        registry.on_construct<RigidBody2DComponent>().disconnect(this);
        registry.on_destroy<RigidBody2DComponent>().disconnect(this);
        registry.on_construct<BoxCollider2DComponent>().disconnect(this);
        registry.on_construct<CircleCollider2DComponent>().disconnect(this);
    }

    float Physics2DSystem::GetGravity() const
    {
        return m_World->GetGravity().y;
    }

    void Physics2DSystem::SetGravity(float gravity)
    {
        m_World->SetGravity({ 0.0f, gravity });
    }

    void Physics2DSystem::OnRigidBody2DConstruct(entt::registry& registry, entt::entity entity)
    {
        Entity e = { entity, m_Scene };
        auto& tc = e.Transform();
        auto& rigidBody2D = registry.get<RigidBody2DComponent>(entity);

        b2BodyDef bodyDef;
        if (rigidBody2D.BodyType == RigidBody2DComponent::Type::Static)
            bodyDef.type = b2_staticBody;
        else if (rigidBody2D.BodyType == RigidBody2DComponent::Type::Dynamic)
            bodyDef.type = b2_dynamicBody;
        else if (rigidBody2D.BodyType == RigidBody2DComponent::Type::Kinematic)
            bodyDef.type = b2_kinematicBody;

        bodyDef.position.Set(tc.Position.x, tc.Position.y);
        bodyDef.angle = glm::eulerAngles(tc.Rotation).z;

        b2Body* body = m_World->CreateBody(&bodyDef);
        body->SetFixedRotation(rigidBody2D.FixedRotation);
        body->GetUserData().pointer = (uintptr_t)e.GetUUID();
        rigidBody2D.RuntimeBody = body;
    }

    void Physics2DSystem::OnRigidBody2DDestroy(entt::registry& registry, entt::entity entity)
    {
        auto& rigidBody2D = registry.get<RigidBody2DComponent>(entity);
        if (rigidBody2D.RuntimeBody)
        {
            m_World->DestroyBody(static_cast<b2Body*>(rigidBody2D.RuntimeBody));
            rigidBody2D.RuntimeBody = nullptr;
        }
    }

    void Physics2DSystem::OnBoxCollider2DConstruct(entt::registry& registry, entt::entity entity)
    {
        Entity e = { entity, m_Scene };
        auto& boxCollider2D = registry.get<BoxCollider2DComponent>(entity);
        if (e.HasComponent<RigidBody2DComponent>())
        {
            auto& rigidBody2D = e.GetComponent<RigidBody2DComponent>();
            PR_CORE_ASSERT(rigidBody2D.RuntimeBody);
            b2Body* body = static_cast<b2Body*>(rigidBody2D.RuntimeBody);

            b2PolygonShape polygonShape;
            polygonShape.SetAsBox(boxCollider2D.Size.x, boxCollider2D.Size.y);

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &polygonShape;
            fixtureDef.density = boxCollider2D.Density;
            fixtureDef.friction = boxCollider2D.Friction;
            body->CreateFixture(&fixtureDef);
        }
    }

    void Physics2DSystem::OnCircleCollider2DConstruct(entt::registry& registry, entt::entity entity)
    {
        Entity e = { entity, m_Scene };
        auto& circleCollider2D = registry.get<CircleCollider2DComponent>(entity);
        if (e.HasComponent<RigidBody2DComponent>())
        {
            auto& rigidBody2D = e.GetComponent<RigidBody2DComponent>();
            PR_CORE_ASSERT(rigidBody2D.RuntimeBody);
            b2Body* body = static_cast<b2Body*>(rigidBody2D.RuntimeBody);

            b2CircleShape circleShape;
            circleShape.m_radius = circleCollider2D.Radius;

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &circleShape;
            fixtureDef.density = circleCollider2D.Density;
            fixtureDef.friction = circleCollider2D.Friction;
            body->CreateFixture(&fixtureDef);
        }
    }

}
