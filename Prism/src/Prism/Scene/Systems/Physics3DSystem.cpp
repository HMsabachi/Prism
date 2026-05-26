#include "prpch.h"
#include "Physics3DSystem.h"
#include "../Scene.h"
#include "../Entity.h"
#include "../Components.h"

#include "Prism/Physics/Physics3D.h"
#include "ScriptSystem.h"
#include <PhysX/PxPhysicsAPI.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Prism {

Physics3DSystem::Physics3DSystem(Scene* scene)
    : m_Scene(scene)
{
    physx::PxSceneDesc sceneDesc = Physics3D::CreateSceneDesc();
    sceneDesc.gravity = physx::PxVec3(0.0F, -9.8F, 0.0F);
    m_PhysxScene = Physics3D::CreateScene(sceneDesc);
}

Physics3DSystem::~Physics3DSystem()
{
    if (m_PhysxScene)
    {
        m_PhysxScene->release();
        m_PhysxScene = nullptr;
    }
}

void Physics3DSystem::OnFixedUpdate(float ts)
{
    if (!m_PhysxScene)
        return;

    m_PhysxScene->simulate(ts);
    m_PhysxScene->fetchResults(true);

    auto view = m_Scene->GetAllEntitiesWith<RigidBodyComponent>();
    for (auto entity : view)
    {
        auto& rb = m_Scene->GetRegistry().get<RigidBodyComponent>(entity);
        if (rb.BodyType == RigidBodyComponent::Type::Dynamic && rb.RuntimeActor)
        {
            physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
            physx::PxTransform pxTransform = actor->getGlobalPose();

            Entity e = { entity, m_Scene };
            auto& tc = e.Transform();
            tc.Position = glm::vec3(pxTransform.p.x, pxTransform.p.y, pxTransform.p.z);
            tc.Rotation = glm::quat(pxTransform.q.w, pxTransform.q.x, pxTransform.q.y, pxTransform.q.z);
        }
    }
}

void Physics3DSystem::OnRuntimeStart()
{
    if (!m_PhysxScene)
        return;

    auto& registry = m_Scene->GetRegistry();
    registry.on_construct<RigidBodyComponent>().connect<&Physics3DSystem::OnRigidBodyConstruct>(this);
    registry.on_destroy<RigidBodyComponent>().connect<&Physics3DSystem::OnRigidBodyDestroy>(this);

    Physics3D::SetCollisionScene(m_Scene);

    {
        auto* ss = m_Scene->GetSystem<ScriptSystem>();
        Physics3D::SetCollisionCallbacks(
            [ss](Entity e) { if (ss) ss->OnCollisionBegin(e); },
            [ss](Entity e) { if (ss) ss->OnCollisionEnd(e); }
        );
    }

    auto rigidBodyView = registry.view<RigidBodyComponent>();
    for (auto entity : rigidBodyView)
        OnRigidBodyConstruct(registry, entity);

    auto boxView = registry.view<BoxColliderComponent>();
    for (auto entity : boxView)
    {
        Entity e = { entity, m_Scene };
        if (!e.HasComponent<RigidBodyComponent>())
            continue;

        auto& rb = e.GetComponent<RigidBodyComponent>();
        physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
        auto& bc = e.GetComponent<BoxColliderComponent>();

        physx::PxMaterial* material;
        if (e.HasComponent<PhysicsMaterialComponent>())
        {
            auto& pm = e.GetComponent<PhysicsMaterialComponent>();
            material = Physics3D::CreateMaterial(pm.StaticFriction, pm.DynamicFriction, pm.Bounciness);
        }
        else
            material = Physics3D::CreateMaterial(1.0f, 1.0f, 1.0f);

        auto* physics = Physics3D::GetFactory();
        physx::PxShape* shape = physics->createShape(physx::PxBoxGeometry(bc.Size.x * 0.5f, bc.Size.y * 0.5f, bc.Size.z * 0.5f), *material, true);
        shape->setLocalPose(physx::PxTransform(physx::PxVec3(bc.Offset.x, bc.Offset.y, bc.Offset.z)));
        actor->attachShape(*shape);
        shape->release();

        Physics3D::SetCollisionFilters(actor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All);
    }

    auto sphereView = registry.view<SphereColliderComponent>();
    for (auto entity : sphereView)
    {
        Entity e = { entity, m_Scene };
        if (!e.HasComponent<RigidBodyComponent>())
            continue;

        auto& rb = e.GetComponent<RigidBodyComponent>();
        physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
        auto& sc = e.GetComponent<SphereColliderComponent>();

        physx::PxMaterial* material;
        if (e.HasComponent<PhysicsMaterialComponent>())
        {
            auto& pm = e.GetComponent<PhysicsMaterialComponent>();
            material = Physics3D::CreateMaterial(pm.StaticFriction, pm.DynamicFriction, pm.Bounciness);
        }
        else
            material = Physics3D::CreateMaterial(1.0f, 1.0f, 1.0f);

        auto* physics = Physics3D::GetFactory();
        physx::PxShape* shape = physics->createShape(physx::PxSphereGeometry(sc.Radius), *material, true);
        shape->setLocalPose(physx::PxTransform(physx::PxIdentity));
        actor->attachShape(*shape);
        shape->release();

        Physics3D::SetCollisionFilters(actor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All);
    }

    auto capsuleView = registry.view<CapsuleColliderComponent>();
    for (auto entity : capsuleView)
    {
        Entity e = { entity, m_Scene };
        if (!e.HasComponent<RigidBodyComponent>())
            continue;

        auto& rb = e.GetComponent<RigidBodyComponent>();
        physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
        auto& cc = e.GetComponent<CapsuleColliderComponent>();

        physx::PxMaterial* material;
        if (e.HasComponent<PhysicsMaterialComponent>())
        {
            auto& pm = e.GetComponent<PhysicsMaterialComponent>();
            material = Physics3D::CreateMaterial(pm.StaticFriction, pm.DynamicFriction, pm.Bounciness);
        }
        else
            material = Physics3D::CreateMaterial(1.0f, 1.0f, 1.0f);

        auto* physics = Physics3D::GetFactory();
        physx::PxShape* shape = physics->createShape(physx::PxCapsuleGeometry(cc.Radius, cc.Height * 0.5f), *material, true);
        shape->setLocalPose(physx::PxTransform(physx::PxIdentity));
        actor->attachShape(*shape);
        shape->release();

        Physics3D::SetCollisionFilters(actor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All);
    }

    auto meshColliderView = registry.view<MeshColliderComponent>();
    for (auto entity : meshColliderView)
    {
        Entity e = { entity, m_Scene };
        if (!e.HasComponent<RigidBodyComponent>())
            continue;

        auto& rb = e.GetComponent<RigidBodyComponent>();
        physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
        auto& mc = e.GetComponent<MeshColliderComponent>();

        if (!mc.CollisionMesh)
        {
            PR_CORE_ERROR("MeshColliderComponent has no CollisionMesh assigned!");
            continue;
        }

        physx::PxMaterial* material;
        if (e.HasComponent<PhysicsMaterialComponent>())
        {
            auto& pm = e.GetComponent<PhysicsMaterialComponent>();
            material = Physics3D::CreateMaterial(pm.StaticFriction, pm.DynamicFriction, pm.Bounciness);
        }
        else
            material = Physics3D::CreateMaterial(1.0f, 1.0f, 1.0f);

        physx::PxShape* shape = nullptr;
        if (rb.BodyType == RigidBodyComponent::Type::Dynamic)
        {
            physx::PxConvexMesh* convexMesh = Physics3D::CreateConvexMeshCollider(mc.CollisionMesh);
            if (!convexMesh) continue;

            physx::PxConvexMeshGeometry geometry(convexMesh);
            auto* physics = Physics3D::GetFactory();
            shape = physics->createShape(geometry, *material, true);
            convexMesh->release();
        }
        else
        {
            physx::PxTriangleMesh* triangleMesh = Physics3D::CreateTriangleMeshCollider(mc.CollisionMesh);
            if (!triangleMesh) continue;

            physx::PxTriangleMeshGeometry geometry(triangleMesh);
            auto* physics = Physics3D::GetFactory();
            shape = physics->createShape(geometry, *material, true);
            triangleMesh->release();
        }

        shape->setLocalPose(physx::PxTransform(physx::PxIdentity));
        actor->attachShape(*shape);
        shape->release();

        Physics3D::SetCollisionFilters(actor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All);
    }
}

void Physics3DSystem::OnRuntimeStop()
{
    auto& registry = m_Scene->GetRegistry();
    registry.on_construct<RigidBodyComponent>().disconnect(this);
    registry.on_destroy<RigidBodyComponent>().disconnect(this);

    if (m_PhysxScene)
    {
        m_PhysxScene->release();
        m_PhysxScene = nullptr;
    }

    auto view = registry.view<RigidBodyComponent>();
    for (auto entity : view)
        registry.get<RigidBodyComponent>(entity).RuntimeActor = nullptr;
}

void Physics3DSystem::OnRigidBodyConstruct(entt::registry& registry, entt::entity entity)
{
    if (!m_PhysxScene)
        return;

    Entity e = { entity, m_Scene };
    auto& rb = registry.get<RigidBodyComponent>(entity);

    physx::PxRigidActor* actor = Physics3D::CreateAndAddActor(m_PhysxScene, rb, e.Transform().GetTransform());
    if (actor)
    {
        actor->userData = (void*)(uintptr_t)e.GetUUID();
        rb.RuntimeActor = actor;
    }
}

void Physics3DSystem::OnRigidBodyDestroy(entt::registry& registry, entt::entity entity)
{
    auto& rb = registry.get<RigidBodyComponent>(entity);
    if (!rb.RuntimeActor)
        return;

    if (m_PhysxScene)
        m_PhysxScene->removeActor(*static_cast<physx::PxRigidActor*>(rb.RuntimeActor));

    static_cast<physx::PxRigidActor*>(rb.RuntimeActor)->release();
    rb.RuntimeActor = nullptr;
}

}
