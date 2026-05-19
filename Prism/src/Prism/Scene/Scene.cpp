#include "prpch.h"
#include "Scene.h"

#include "Prism/Events/Event.h"
#include "Entity.h"
#include "Components.h"

#include "Scripting/ScriptEngine.h"

#include "Prism/Renderer/Renderer2D.h"
#include "Prism/Renderer/SceneRenderer.h"
#include "Prism/Renderer/Renderer.h"

#include "Prism/Editor/EditorCamera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

// Box2D
#include <box2d/box2d.h>

// PhysX
#include "Prism/Physics/Physics3D.h"

namespace Prism
{

    static const std::string DefaultEntityName = "Entity";

    std::unordered_map<UUID, Scene*> s_ActiveScenes;

    struct SceneComponent
    {
        UUID SceneID;
    };

    struct Box2DWorldComponent
    {
        std::unique_ptr<b2World> World;
    };

    struct PhysXSceneComponent
    {
        physx::PxScene* World;
    };

    class ContactListener : public b2ContactListener
    {
    public:
        virtual void BeginContact(b2Contact* contact) override
        {
            Entity& a = *(Entity*)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
            Entity& b = *(Entity*)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

            // TODO: improve these if checks
            if (a.HasComponent<ScriptComponent>() && ScriptEngine::ModuleExists(a.GetComponent<ScriptComponent>().ModuleName))
                ScriptEngine::OnCollision2DBegin(a);

            if (b.HasComponent<ScriptComponent>() && ScriptEngine::ModuleExists(b.GetComponent<ScriptComponent>().ModuleName))
                ScriptEngine::OnCollision2DBegin(b);
        }

        /// Called when two fixtures cease to touch.
        virtual void EndContact(b2Contact* contact) override
        {
            Entity& a = *(Entity*)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
            Entity& b = *(Entity*)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

            // TODO: improve these if checks
            if (a.HasComponent<ScriptComponent>() && ScriptEngine::ModuleExists(a.GetComponent<ScriptComponent>().ModuleName))
                ScriptEngine::OnCollision2DEnd(a);

            if (b.HasComponent<ScriptComponent>() && ScriptEngine::ModuleExists(b.GetComponent<ScriptComponent>().ModuleName))
                ScriptEngine::OnCollision2DEnd(b);
        }
    };

    static ContactListener s_Box2DContactListener;

    static void OnScriptComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        auto sceneView = registry.view<SceneComponent>();
        UUID sceneID = registry.get<SceneComponent>(sceneView.front()).SceneID;
        Scene* scene = s_ActiveScenes[sceneID];

        auto entityID = registry.get<IDComponent>(entity).ID;
        PR_CORE_ASSERT(scene->m_EntityIDMap.find(entityID) != scene->m_EntityIDMap.end());
        ScriptEngine::InitScriptEntity(scene->m_EntityIDMap.at(entityID));
    }

    static void OnScriptComponentDestroy(entt::registry& registry, entt::entity entity)
    {
        auto sceneView = registry.view<SceneComponent>();
        UUID sceneID = registry.get<SceneComponent>(sceneView.front()).SceneID;

        Scene* scene = s_ActiveScenes[sceneID];

        auto entityID = registry.get<IDComponent>(entity).ID;
        ScriptEngine::OnScriptComponentDestroyed(sceneID, entityID);
    }

    Scene::Scene(const std::string& debugName)
        : m_DebugName(debugName)
    {
        m_Registry.on_construct<ScriptComponent>().connect<&OnScriptComponentConstruct>();
        m_Registry.on_destroy<ScriptComponent>().connect<&OnScriptComponentDestroy>();

        m_SceneEntity = m_Registry.create();
        m_Registry.emplace<SceneComponent>(m_SceneEntity, m_SceneID);

        // TODO: Obviously not necessary in all cases
        m_Registry.emplace<Box2DWorldComponent>(m_SceneEntity, std::make_unique<b2World>(b2Vec2{ 0.0f, -9.8f }));
        m_Registry.get<Box2DWorldComponent>(m_SceneEntity).World->SetContactListener(&s_Box2DContactListener);

        // PhysX
        {
            physx::PxSceneDesc sceneDesc = Physics3D::CreateSceneDesc();
            sceneDesc.gravity = physx::PxVec3(0.0F, -9.8F, 0.0F);
            PhysXSceneComponent& physxWorld = m_Registry.emplace<PhysXSceneComponent>(m_SceneEntity, Physics3D::CreateScene(sceneDesc));
            PR_CORE_ASSERT(physxWorld.World);
        }

        s_ActiveScenes[m_SceneID] = this;
        Init();
    }

    Scene::~Scene()
    {
        m_Registry.on_destroy<ScriptComponent>().disconnect();
        m_Registry.on_construct<ScriptComponent>().disconnect<&OnScriptComponentConstruct>();
        OnShutdown();
        delete[] m_PhysicsBodyEntityBuffer;
        m_PhysicsBodyEntityBuffer = nullptr;
        delete[] m_Physics3DBodyEntityBuffer;
        m_Physics3DBodyEntityBuffer = nullptr;
        m_Registry.clear();
        s_ActiveScenes.erase(m_SceneID);
        ScriptEngine::OnSceneDestruct(m_SceneID);
    }

    void Scene::Init()
    {
#if 1
        auto skyboxShader = Renderer::GetShaderLibrary()->Get("Custom/Skybox");
        m_SkyboxMaterial = MaterialInstance::Create(Material::Create(skyboxShader));
#endif
    }

    void Scene::OnUpdate()
    {
        // Script OnUpdate (per-frame)
        {
            float ts = Time::GetDeltaTime();
            auto view = m_Registry.view<ScriptComponent>();
            for (auto entity : view)
            {
                UUID entityID = m_Registry.get<IDComponent>(entity).ID;
                Entity e = { entity, this };
                if (ScriptEngine::ModuleExists(e.GetComponent<ScriptComponent>().ModuleName))
                    ScriptEngine::OnUpdateEntity(m_SceneID, entityID, ts);
            }
        }
    }

    void Scene::OnFixedUpdate()
    {
        if (!Time::ShouldFixedUpdate())
            return;

        float ts = Time::GetFixedDeltaTime();

        // Box2D physics step
        {
            auto view = m_Registry.view<Box2DWorldComponent>();
            if (!view.empty())
            {
                auto& box2DWorld = m_Registry.get<Box2DWorldComponent>(view.front()).World;
                int32_t velocityIterations = 6;
                int32_t positionIterations = 2;
                box2DWorld->Step(ts, velocityIterations, positionIterations);
            }
        }

        // Sync Box2D positions back to entity transforms
        {
            auto view = m_Registry.view<RigidBody2DComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, this };
                auto& tc = e.Transform();
                auto& rb2d = e.GetComponent<RigidBody2DComponent>();
                b2Body* body = static_cast<b2Body*>(rb2d.RuntimeBody);

                auto& position = body->GetPosition();
                tc.Position.x = position.x;
                tc.Position.y = position.y;
                tc.Rotation = glm::quat({ 0.0f, 0.0f, body->GetAngle() });
            }
        }

        // PhysX fixed-step simulation
        {
            auto physxView = m_Registry.view<PhysXSceneComponent>();
            if (!physxView.empty())
            {
                physx::PxScene* physxScene = m_Registry.get<PhysXSceneComponent>(physxView.front()).World;
                physxScene->simulate(ts);
                physxScene->fetchResults(true);
            }
        }

        // Sync PhysX dynamic body transforms back to entities
        {
            auto view = m_Registry.view<RigidBodyComponent>();
            for (auto entity : view)
            {
                auto& rb = m_Registry.get<RigidBodyComponent>(entity);
                if (rb.BodyType == RigidBodyComponent::Type::Dynamic && rb.RuntimeActor)
                {
                    physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
                    physx::PxTransform pxTransform = actor->getGlobalPose();

                    Entity e = { entity, this };
                    auto& tc = e.Transform();
                    tc.Position = glm::vec3(pxTransform.p.x, pxTransform.p.y, pxTransform.p.z);
                    tc.Rotation = glm::quat(pxTransform.q.w, pxTransform.q.x, pxTransform.q.y, pxTransform.q.z);
                }
            }
        }

        // Script OnFixedUpdate
        {
            auto view = m_Registry.view<ScriptComponent>();
            for (auto entity : view)
            {
                UUID entityID = m_Registry.get<IDComponent>(entity).ID;
                Entity e = { entity, this };
                if (ScriptEngine::ModuleExists(e.GetComponent<ScriptComponent>().ModuleName))
                    ScriptEngine::OnFixedUpdateEntity(m_SceneID, entityID);
            }
        }
    }

    void Scene::OnRenderRuntime()
    {
        /////////////////////////////////////////////////////////////////////
        // RENDER 3D SCENE
        /////////////////////////////////////////////////////////////////////
        float ts = Time::GetDeltaTime();
        Entity cameraEntity = GetMainCameraEntity();
        if (!cameraEntity)
            return;
        glm::mat4 cameraViewMatrix = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());
        PR_CORE_ASSERT(cameraEntity, "Scene does not contain any cameras!");
        SceneCamera& camera = cameraEntity.GetComponent<CameraComponent>();
        camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);

        m_SkyboxMaterial->Set("u_TextureLod", m_SkyboxLod);

        auto group = m_Registry.group<MeshComponent>(entt::get<TransformComponent>);
        SceneRenderer::BeginScene(this, { camera, cameraViewMatrix });
        for (auto entity : group)
        {
            auto [transformComponent, meshComponent] = group.get<TransformComponent, MeshComponent>(entity);
            if (meshComponent.Mesh)
            {
                meshComponent.Mesh->OnUpdate(ts);

                // TODO: Should we render (logically)
                Ref<MaterialInstance> overrideMaterial = m_Registry.any_of<MaterialComponent>(entity) ? m_Registry.get<MaterialComponent>(entity).Material : nullptr;
                SceneRenderer::SubmitMesh(meshComponent, transformComponent.GetTransform(), overrideMaterial);
            }
        }
        SceneRenderer::EndScene();
        /////////////////////////////////////////////////////////////////////

#if 0
        // Render all sprites
        Renderer2D::BeginScene(*camera);
        {
            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRenderer>);
            for (auto entity : group)
            {
                auto [transformComponent, spriteRendererComponent] = group.get<TransformComponent, SpriteRenderer>(entity);
                if (spriteRendererComponent.Texture)
                    Renderer2D::DrawQuad(transformComponent.GetTransform(), spriteRendererComponent.Texture, spriteRendererComponent.TilingFactor);
                else
                    Renderer2D::DrawQuad(transformComponent.GetTransform(), spriteRendererComponent.Color);
            }
        }

        Renderer2D::EndScene();
#endif
    }

    void Scene::OnRenderEditor(const EditorCamera& editorCamera)
    {
        /////////////////////////////////////////////////////////////////////
        // RENDER 3D SCENE
        /////////////////////////////////////////////////////////////////////
        float ts = Time::GetDeltaTime();
        m_SkyboxMaterial->Set("u_TextureLod", m_SkyboxLod);

        auto group = m_Registry.group<MeshComponent>(entt::get<TransformComponent>);
        SceneRenderer::BeginScene(this, { editorCamera, editorCamera.GetViewMatrix() });
        for (auto entity : group)
        {
            auto [transformComponent, meshComponent] = group.get<TransformComponent, MeshComponent>(entity);
            if (meshComponent.Mesh)
            {
                meshComponent.Mesh->OnUpdate(ts);

                // TODO: Should we render (logically)

                Ref<MaterialInstance> overrideMaterial = m_Registry.any_of<MaterialComponent>(entity) ? m_Registry.get<MaterialComponent>(entity).Material : nullptr;
                if (m_SelectedEntity == entity)
                    SceneRenderer::SubmitSelectedMesh(meshComponent, transformComponent.GetTransform());
                else
                    SceneRenderer::SubmitMesh(meshComponent, transformComponent.GetTransform(), overrideMaterial);
            }
        }
        SceneRenderer::EndScene();
        /////////////////////////////////////////////////////////////////////

#if 0
        // Render all sprites
        Renderer2D::BeginScene(*camera);
        {
            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRenderer>);
            for (auto entity : group)
            {
                auto [transformComponent, spriteRendererComponent] = group.get<TransformComponent, SpriteRenderer>(entity);
                if (spriteRendererComponent.Texture)
                    Renderer2D::DrawQuad(transformComponent.GetTransform(), spriteRendererComponent.Texture, spriteRendererComponent.TilingFactor);
                else
                    Renderer2D::DrawQuad(transformComponent.GetTransform(), spriteRendererComponent.Color);
            }
        }

        Renderer2D::EndScene();
#endif
    }

    void Scene::OnEvent(Event& e)
    {
    }
    void Scene::OnRuntimeStart()
    {
        ScriptEngine::SetSceneContext(this);

        {
            auto view = m_Registry.view<ScriptComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, this };
                if (ScriptEngine::ModuleExists(e.GetComponent<ScriptComponent>().ModuleName))
                    ScriptEngine::InstantiateEntityClass(e);
            }
        }

        // Box2D physics
        {
            auto view = m_Registry.view<RigidBody2DComponent>();
            m_PhysicsBodyEntityBuffer = new Entity[view.size()];
            uint32_t physicsBodyEntityBufferIndex = 0;
            for (auto entity : view)
            {
                Entity e = { entity, this };
                auto& tc = e.Transform();
                auto& rigidBody2D = m_Registry.get<RigidBody2DComponent>(entity);

                b2BodyDef bodyDef;
                if (rigidBody2D.BodyType == RigidBody2DComponent::Type::Static)
                    bodyDef.type = b2_staticBody;
                else if (rigidBody2D.BodyType == RigidBody2DComponent::Type::Dynamic)
                    bodyDef.type = b2_dynamicBody;
                else if (rigidBody2D.BodyType == RigidBody2DComponent::Type::Kinematic)
                    bodyDef.type = b2_kinematicBody;
                bodyDef.position.Set(tc.Position.x, tc.Position.y);
                bodyDef.angle = glm::eulerAngles(tc.Rotation).z;

                b2Body* body = m_Registry.get<Box2DWorldComponent>(m_SceneEntity).World->CreateBody(&bodyDef);
                body->SetFixedRotation(rigidBody2D.FixedRotation);
                Entity* entityStorage = &m_PhysicsBodyEntityBuffer[physicsBodyEntityBufferIndex++];
                *entityStorage = e;
                body->GetUserData().pointer = (uintptr_t)entityStorage;
                rigidBody2D.RuntimeBody = body;
            }
        }

        {
            auto view = m_Registry.view<BoxCollider2DComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, this };

                auto& boxCollider2D = m_Registry.get<BoxCollider2DComponent>(entity);
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
        }

        {
            auto view = m_Registry.view<CircleCollider2DComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, this };

                auto& circleCollider2D = m_Registry.get<CircleCollider2DComponent>(entity);
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

        // PhysX 3D physics
        {
            auto physxView = m_Registry.view<PhysXSceneComponent>();
            if (!physxView.empty())
            {
                physx::PxScene* physxScene = m_Registry.get<PhysXSceneComponent>(physxView.front()).World;

                // Create rigid body actors
                auto rigidBodyView = m_Registry.view<RigidBodyComponent>();
                m_Physics3DBodyEntityBuffer = new Entity[rigidBodyView.size()];
                uint32_t physics3DBodyEntityBufferIndex = 0;

                for (auto entity : rigidBodyView)
                {
                    Entity e = { entity, this };
                    auto& rb = m_Registry.get<RigidBodyComponent>(entity);

                    physx::PxRigidActor* actor = Physics3D::CreateAndAddActor(physxScene, rb, e.Transform().GetTransform());
                    if (actor)
                    {
                        Entity* entityStorage = &m_Physics3DBodyEntityBuffer[physics3DBodyEntityBufferIndex++];
                        *entityStorage = e;
                        actor->userData = entityStorage;
                        rb.RuntimeActor = actor;
                    }
                }

                // Box colliders
                auto boxView = m_Registry.view<BoxColliderComponent>();
                for (auto entity : boxView)
                {
                    Entity e = { entity, this };
                    if (!e.HasComponent<RigidBodyComponent>())
                        continue;

                    auto& rb = e.GetComponent<RigidBodyComponent>();
                    PR_CORE_ASSERT(rb.RuntimeActor);
                    physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
                    auto& bc = e.GetComponent<BoxColliderComponent>();

                    physx::PxMaterial* material;
                    if (e.HasComponent<PhysicsMaterialComponent>())
                    {
                        auto& pm = e.GetComponent<PhysicsMaterialComponent>();
                        material = Physics3D::CreateMaterial(pm.StaticFriction, pm.DynamicFriction, pm.Bounciness);
                    }
                    else
                    {
                        material = Physics3D::CreateMaterial(1.0f, 1.0f, 1.0f);
                    }

                    auto* physics = Physics3D::GetFactory();
                    physx::PxShape* shape = physics->createShape(physx::PxBoxGeometry(bc.Size.x * 0.5f, bc.Size.y * 0.5f, bc.Size.z * 0.5f), *material, true);
                    shape->setLocalPose(physx::PxTransform(physx::PxVec3(bc.Offset.x, bc.Offset.y, bc.Offset.z)));
                    actor->attachShape(*shape);
                    shape->release();

                    Physics3D::SetCollisionFilters(actor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All);
                }

                // Sphere colliders
                auto sphereView = m_Registry.view<SphereColliderComponent>();
                for (auto entity : sphereView)
                {
                    Entity e = { entity, this };
                    if (!e.HasComponent<RigidBodyComponent>())
                        continue;

                    auto& rb = e.GetComponent<RigidBodyComponent>();
                    PR_CORE_ASSERT(rb.RuntimeActor);
                    physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
                    auto& sc = e.GetComponent<SphereColliderComponent>();

                    physx::PxMaterial* material;
                    if (e.HasComponent<PhysicsMaterialComponent>())
                    {
                        auto& pm = e.GetComponent<PhysicsMaterialComponent>();
                        material = Physics3D::CreateMaterial(pm.StaticFriction, pm.DynamicFriction, pm.Bounciness);
                    }
                    else
                    {
                        material = Physics3D::CreateMaterial(1.0f, 1.0f, 1.0f);
                    }

                    auto* physics = Physics3D::GetFactory();
                    physx::PxShape* shape = physics->createShape(physx::PxSphereGeometry(sc.Radius), *material, true);
                    shape->setLocalPose(physx::PxTransform(physx::PxIdentity));
                    actor->attachShape(*shape);
                    shape->release();

                    Physics3D::SetCollisionFilters(actor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All);
                }

                // Capsule colliders
                auto capsuleView = m_Registry.view<CapsuleColliderComponent>();
                for (auto entity : capsuleView)
                {
                    Entity e = { entity, this };
                    if (!e.HasComponent<RigidBodyComponent>())
                        continue;

                    auto& rb = e.GetComponent<RigidBodyComponent>();
                    PR_CORE_ASSERT(rb.RuntimeActor);
                    physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rb.RuntimeActor);
                    auto& cc = e.GetComponent<CapsuleColliderComponent>();

                    physx::PxMaterial* material;
                    if (e.HasComponent<PhysicsMaterialComponent>())
                    {
                        auto& pm = e.GetComponent<PhysicsMaterialComponent>();
                        material = Physics3D::CreateMaterial(pm.StaticFriction, pm.DynamicFriction, pm.Bounciness);
                    }
                    else
                    {
                        material = Physics3D::CreateMaterial(1.0f, 1.0f, 1.0f);
                    }

                    auto* physics = Physics3D::GetFactory();
                    physx::PxShape* shape = physics->createShape(physx::PxCapsuleGeometry(cc.Radius, cc.Height * 0.5f), *material, true);
                    shape->setLocalPose(physx::PxTransform(physx::PxIdentity));
                    actor->attachShape(*shape);
                    shape->release();

                    Physics3D::SetCollisionFilters(actor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All);
                }

                // Mesh colliders
                auto meshColliderView = m_Registry.view<MeshColliderComponent>();
                for (auto entity : meshColliderView)
                {
                    Entity e = { entity, this };
                    if (!e.HasComponent<RigidBodyComponent>())
                        continue;

                    auto& rb = e.GetComponent<RigidBodyComponent>();
                    PR_CORE_ASSERT(rb.RuntimeActor);
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
                    {
                        material = Physics3D::CreateMaterial(1.0f, 1.0f, 1.0f);
                    }

                    physx::PxShape* shape = nullptr;
                    if (rb.BodyType == RigidBodyComponent::Type::Dynamic)
                    {
                        physx::PxConvexMesh* convexMesh = Physics3D::CreateConvexMeshCollider(mc);
                        if (!convexMesh)
                            continue;

                        physx::PxConvexMeshGeometry geometry(convexMesh);
                        auto* physics = Physics3D::GetFactory();
                        shape = physics->createShape(geometry, *material, true);
                        convexMesh->release();
                    }
                    else
                    {
                        physx::PxTriangleMesh* triangleMesh = Physics3D::CreateTriangleMeshCollider(mc);
                        if (!triangleMesh)
                            continue;

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
        }

        m_IsPlaying = true;
    }

    void Scene::OnRuntimeStop()
    {
        // Release PhysX scene
        {
            auto physxView = m_Registry.view<PhysXSceneComponent>();
            if (!physxView.empty())
            {
                auto& physxComp = m_Registry.get<PhysXSceneComponent>(m_SceneEntity);
                if (physxComp.World)
                {
                    physxComp.World->release();
                    physxComp.World = nullptr;
                }
            }
        }

        // Clear PhysX runtime actor pointers
        {
            auto view = m_Registry.view<RigidBodyComponent>();
            for (auto entity : view)
                m_Registry.get<RigidBodyComponent>(entity).RuntimeActor = nullptr;
        }

        delete[] m_Physics3DBodyEntityBuffer;
        m_Physics3DBodyEntityBuffer = nullptr;

        delete[] m_PhysicsBodyEntityBuffer;
        m_PhysicsBodyEntityBuffer = nullptr;
        m_IsPlaying = false;
    }

    void Scene::OnShutdown()
    {
        auto physxView = m_Registry.view<PhysXSceneComponent>();
        if (!physxView.empty())
        {
            auto& physxComp = m_Registry.get<PhysXSceneComponent>(m_SceneEntity);
            if (physxComp.World)
            {
                physxComp.World->release();
                physxComp.World = nullptr;
            }
        }

    }

    void Scene::SetViewportSize(uint32_t width, uint32_t height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;
    }

    void Scene::SetEnvironment(const Environment& environment)
    {
        m_Environment = environment;
        SetSkybox(environment.RadianceMap);
    }

    void Scene::SetSkybox(const Ref<TextureCube>& skybox)
    {
        m_SkyboxTexture = skybox;
        m_SkyboxMaterial->Set("u_Texture", skybox);
    }

    Entity Scene::GetMainCameraEntity()
    {
        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view)
        {
            auto& comp = view.get<CameraComponent>(entity);
            if (comp.Primary)
                return { entity, this };
        }
        return {};
    }

    Entity Scene::CreateEntity(const std::string& name)
    {
        auto entity = Entity{ m_Registry.create(), this };
        auto& idComponent = entity.AddComponent<IDComponent>();
        idComponent.ID = {};

        entity.AddComponent<TransformComponent>();
        if (!name.empty())
            entity.AddComponent<TagComponent>(name);

        m_EntityIDMap[idComponent.ID] = entity;
        return entity;
    }
    Entity Scene::CreateEntityWithID(UUID uuid, const std::string& name, bool runtimeMap)
    {
        auto entity = Entity{ m_Registry.create(), this };
        auto& idComponent = entity.AddComponent<IDComponent>();
        idComponent.ID = uuid;

        entity.AddComponent<TransformComponent>();
        if (!name.empty())
            entity.AddComponent<TagComponent>(name);

        PR_CORE_ASSERT(m_EntityIDMap.find(uuid) == m_EntityIDMap.end());
        m_EntityIDMap[uuid] = entity;
        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        if (entity.HasComponent<ScriptComponent>())
            ScriptEngine::OnScriptComponentDestroyed(m_SceneID, entity.GetUUID());

        m_Registry.destroy(entity.m_EntityHandle);
    }

    template<typename T>
    static void CopyComponent(entt::registry& dstRegistry, entt::registry& srcRegistry, const std::unordered_map<UUID, entt::entity>& enttMap)
    {
        auto components = srcRegistry.view<T>();
        for (auto srcEntity : components)
        {
            entt::entity destEntity = enttMap.at(srcRegistry.get<IDComponent>(srcEntity).ID);

            auto& srcComponent = srcRegistry.get<T>(srcEntity);
            auto& destComponent = dstRegistry.emplace_or_replace<T>(destEntity, srcComponent);
        }
    }

    template<typename T>
    static void CopyComponentIfExists(entt::entity dst, entt::entity src, entt::registry& registry)
    {
        if (registry.any_of<T>(src))
        {
            auto& srcComponent = registry.get<T>(src);
            registry.emplace_or_replace<T>(dst, srcComponent);
        }
    }

    void Scene::DuplicateEntity(Entity entity)
    {
        Entity newEntity;
        if (entity.HasComponent<TagComponent>())
            newEntity = CreateEntity(entity.GetComponent<TagComponent>().Tag);
        else
            newEntity = CreateEntity();

        CopyComponentIfExists<TransformComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
        CopyComponentIfExists<MeshComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
        CopyComponentIfExists<MaterialComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
        CopyComponentIfExists<ScriptComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
        CopyComponentIfExists<CameraComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
        CopyComponentIfExists<SpriteRendererComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
        CopyComponentIfExists<RigidBody2DComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
        CopyComponentIfExists<BoxCollider2DComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
        CopyComponentIfExists<CircleCollider2DComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
        CopyComponentIfExists<RigidBodyComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
        CopyComponentIfExists<PhysicsMaterialComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
        CopyComponentIfExists<BoxColliderComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
        CopyComponentIfExists<SphereColliderComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
        CopyComponentIfExists<CapsuleColliderComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
        CopyComponentIfExists<MeshColliderComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
    }

    Entity Scene::FindEntityByTag(const std::string& tag)
    {
        // TODO: If this becomes used often, consider indexing by tag
        auto view = m_Registry.view<TagComponent>();
        for (auto entity : view)
        {
            const auto& candidate = view.get<TagComponent>(entity).Tag;
            if (candidate == tag)
                return Entity(entity, this);
        }

        return Entity{};
    }

    // Copy to runtime
    void Scene::CopyTo(Ref<Scene>& target)
    {
        // Environment
        target->m_Light = m_Light;
        target->m_LightMultiplier = m_LightMultiplier;

        target->m_Environment = m_Environment;
        target->m_SkyboxTexture = m_SkyboxTexture;
        target->m_SkyboxMaterial = m_SkyboxMaterial;
        target->m_SkyboxLod = m_SkyboxLod;

        std::unordered_map<UUID, entt::entity> enttMap;
        auto idComponents = m_Registry.view<IDComponent>();
        for (auto entity : idComponents)
        {
            auto uuid = m_Registry.get<IDComponent>(entity).ID;
            Entity e = target->CreateEntityWithID(uuid, "", true);
            enttMap[uuid] = e.m_EntityHandle;
        }

        CopyComponent<TagComponent>(target->m_Registry, m_Registry, enttMap);
        CopyComponent<TransformComponent>(target->m_Registry, m_Registry, enttMap);
        CopyComponent<MeshComponent>(target->m_Registry, m_Registry, enttMap);
        CopyComponent<MaterialComponent>(target->m_Registry, m_Registry, enttMap);
        CopyComponent<ScriptComponent>(target->m_Registry, m_Registry, enttMap);
        CopyComponent<CameraComponent>(target->m_Registry, m_Registry, enttMap);
        CopyComponent<SpriteRendererComponent>(target->m_Registry, m_Registry, enttMap);
        CopyComponent<RigidBody2DComponent>(target->m_Registry, m_Registry, enttMap);
        CopyComponent<BoxCollider2DComponent>(target->m_Registry, m_Registry, enttMap);
        CopyComponent<CircleCollider2DComponent>(target->m_Registry, m_Registry, enttMap);
        CopyComponent<RigidBodyComponent>(target->m_Registry, m_Registry, enttMap);
        CopyComponent<PhysicsMaterialComponent>(target->m_Registry, m_Registry, enttMap);
        CopyComponent<BoxColliderComponent>(target->m_Registry, m_Registry, enttMap);
        CopyComponent<SphereColliderComponent>(target->m_Registry, m_Registry, enttMap);
        CopyComponent<CapsuleColliderComponent>(target->m_Registry, m_Registry, enttMap);
        CopyComponent<MeshColliderComponent>(target->m_Registry, m_Registry, enttMap);

        const auto& entityInstanceMap = ScriptEngine::GetEntityInstanceMap();
        if (entityInstanceMap.find(target->GetUUID()) != entityInstanceMap.end())
            ScriptEngine::CopyEntityScriptData(target->GetUUID(), m_SceneID);

        target->SetPhysics2DGravity(GetPhysics2DGravity());
    }

    Ref<Scene> Scene::GetScene(UUID uuid)
    {
        if (s_ActiveScenes.find(uuid) != s_ActiveScenes.end())
            return s_ActiveScenes.at(uuid);

        return {};
    }

    float Scene::GetPhysics2DGravity() const
    {
        return m_Registry.get<Box2DWorldComponent>(m_SceneEntity).World->GetGravity().y;
    }

    void Scene::SetPhysics2DGravity(float gravity)
    {
        m_Registry.get<Box2DWorldComponent>(m_SceneEntity).World->SetGravity({ 0.0f, gravity });
    }

    Environment Environment::Load(const std::string& filepath)
    {
        auto [radiance, irradiance] = SceneRenderer::CreateEnvironmentMap(filepath);
        return { filepath, radiance, irradiance };
    }
}
