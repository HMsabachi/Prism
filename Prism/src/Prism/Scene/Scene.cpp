#include "prpch.h"
#include "Scene.h"

#include "Prism/Events/Event.h"
#include "Entity.h"
#include "Components.h"

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
#include <PhysX/PxPhysicsAPI.h>

#include "Scripting/CSharp/CSharpScriptStorage.h"
#include "Scripting/Python/PythonScriptStorage.h"

#include "Scripting/CSharp/CSharpScriptEngine.h"
#include "Scripting/Python/PythonScriptEngine.h"

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
        Scene* CurrentScene = nullptr;

        virtual void BeginContact(b2Contact* contact) override
        {
            UUID aID = (UUID)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
            UUID bID = (UUID)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

            if (CurrentScene)
            {
                auto itA = CurrentScene->m_EntityIDMap.find(aID);
                auto itB = CurrentScene->m_EntityIDMap.find(bID);
                if (itA != CurrentScene->m_EntityIDMap.end())
                    CurrentScene->OnCollision2DBegin(itA->second);
                if (itB != CurrentScene->m_EntityIDMap.end())
                    CurrentScene->OnCollision2DBegin(itB->second);
            }
        }

        virtual void EndContact(b2Contact* contact) override
        {
            UUID aID = (UUID)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
            UUID bID = (UUID)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

            if (CurrentScene)
            {
                auto itA = CurrentScene->m_EntityIDMap.find(aID);
                auto itB = CurrentScene->m_EntityIDMap.find(bID);
                if (itA != CurrentScene->m_EntityIDMap.end())
                    CurrentScene->OnCollision2DEnd(itA->second);
                if (itB != CurrentScene->m_EntityIDMap.end())
                    CurrentScene->OnCollision2DEnd(itB->second);
            }
        }
    };

    static ContactListener s_Box2DContactListener;

    Scene::Scene(const std::string& debugName)
        : m_DebugName(debugName)
    {
        m_SceneEntity = m_Registry.create();
        m_Registry.emplace<SceneComponent>(m_SceneEntity, m_SceneID);

        // TODO: Obviously not necessary in all cases
        m_Registry.emplace<Box2DWorldComponent>(m_SceneEntity, std::make_unique<b2World>(b2Vec2{ 0.0f, -9.8f }));
        m_Registry.get<Box2DWorldComponent>(m_SceneEntity).World->SetContactListener(&s_Box2DContactListener);
        s_Box2DContactListener.CurrentScene = this;

        // PhysX
        {
            physx::PxSceneDesc sceneDesc = Physics3D::CreateSceneDesc();
            sceneDesc.gravity = physx::PxVec3(0.0F, -9.8F, 0.0F);
            PhysXSceneComponent& physxWorld = m_Registry.emplace<PhysXSceneComponent>(m_SceneEntity, Physics3D::CreateScene(sceneDesc));
            PR_CORE_ASSERT(physxWorld.World);
        }

        s_ActiveScenes[m_SceneID] = this;
        m_CSharpScriptStorage = new CSharpScriptStorage();
        m_PythonScriptStorage = new PythonScriptStorage();
        Init();

        // Always-active component lifecycle signals
        m_Registry.on_construct<CSharpScriptComponent>().connect<&Scene::OnCSharpScriptComponentConstruct>(this);
        m_Registry.on_destroy<CSharpScriptComponent>().connect<&Scene::OnCSharpScriptComponentDestroy>(this);
        m_Registry.on_construct<PythonScriptComponent>().connect<&Scene::OnPythonScriptComponentConstruct>(this);
        m_Registry.on_destroy<PythonScriptComponent>().connect<&Scene::OnPythonScriptComponentDestroy>(this);
    }

    Scene::~Scene()
    {
        OnShutdown();
        delete m_CSharpScriptStorage;
        m_CSharpScriptStorage = nullptr;
        delete m_PythonScriptStorage;
        m_PythonScriptStorage = nullptr;

        // Disconnect always-active signals
        m_Registry.on_construct<CSharpScriptComponent>().disconnect(this);
        m_Registry.on_destroy<CSharpScriptComponent>().disconnect(this);
        m_Registry.on_construct<PythonScriptComponent>().disconnect(this);
        m_Registry.on_destroy<PythonScriptComponent>().disconnect(this);

        m_Registry.clear();
        s_ActiveScenes.erase(m_SceneID);
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
        float ts = Time::GetDeltaTime();

        // C# Script OnUpdate — iterate Behaviours (skip disabled)
        {
            auto view = m_Registry.view<CSharpScriptComponent>();
            UUID sceneID = GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Registry.get<CSharpScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid() && obj->GetPropertyValue<Rolky::Bool32>("Enabled"))
                        obj->InvokeMethod("OnUpdate");
                }
            }
        }

        // Python Script OnUpdate — iterate Behaviours (skip disabled)
        {
            auto view = m_Registry.view<PythonScriptComponent>();
            UUID sceneID = GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Registry.get<PythonScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid() && obj->GetField<bool>("enabled"))
                        obj->Invoke<void>("OnUpdate");
                }
            }
        }

        // C# Script LateUpdate — iterate Behaviours (skip disabled)
        {
            auto view = m_Registry.view<CSharpScriptComponent>();
            UUID sceneID = GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Registry.get<CSharpScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid() && obj->GetPropertyValue<Rolky::Bool32>("Enabled"))
                        obj->InvokeMethod("LateUpdate");
                }
            }
        }

        // Python Script LateUpdate — iterate Behaviours (skip disabled)
        {
            auto view = m_Registry.view<PythonScriptComponent>();
            UUID sceneID = GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Registry.get<PythonScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid() && obj->GetField<bool>("enabled"))
                        obj->Invoke<void>("LateUpdate");
                }
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

        // C# Script OnFixedUpdate — iterate Behaviours (skip disabled)
        {
            auto view = m_Registry.view<CSharpScriptComponent>();
            UUID sceneID = GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Registry.get<CSharpScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid() && obj->GetPropertyValue<Rolky::Bool32>("Enabled"))
                        obj->InvokeMethod("OnFixedUpdate");
                }
            }
        }

        // Python Script OnFixedUpdate — iterate Behaviours (skip disabled)
        {
            auto view = m_Registry.view<PythonScriptComponent>();
            UUID sceneID = GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Registry.get<PythonScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid() && obj->GetField<bool>("enabled"))
                        obj->Invoke<void>("OnFixedUpdate");
                }
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
        CSharpScriptEngine::SetSceneContext(this);
        PythonScriptEngine::SetSceneContext(this);

        // C#: Create Behaviour instances, then Awake → OnCreate → OnEnable
        {
            auto view = m_Registry.view<CSharpScriptComponent>();
            UUID sceneID = GetUUID();
            for (auto entity : view)
            {
                Entity e = { entity, this };
                auto& comp = m_Registry.get<CSharpScriptComponent>(entity);
                // PASS 1: Create instances
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (!obj || !obj->IsValid())
                        CSharpScriptEngine::AddBehaviour(e, binding);
                }
                // PASS 2: Awake
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid())
                        obj->InvokeMethod("Awake");
                }
                // PASS 3: OnCreate (= Unity Start)
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid())
                        obj->InvokeMethod("OnCreate");
                }
                // PASS 4: OnEnable (only if Enabled=true)
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid() && obj->GetPropertyValue<Rolky::Bool32>("Enabled"))
                        obj->InvokeMethod("OnEnable");
                }
            }
        }

        // Python: Create Behaviour instances, then Awake → OnCreate → OnEnable
        {
            auto view = m_Registry.view<PythonScriptComponent>();
            UUID sceneID = GetUUID();
            for (auto entity : view)
            {
                Entity e = { entity, this };
                auto& comp = m_Registry.get<PythonScriptComponent>(entity);
                // PASS 1: Create instances
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (!obj || !obj->IsValid())
                        PythonScriptEngine::AddBehaviour(e, binding);
                }
                // PASS 2: Awake
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid())
                        obj->Invoke<void>("Awake");
                }
                // PASS 3: OnCreate (= Unity Start)
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid())
                        obj->Invoke<void>("OnCreate");
                }
                // PASS 4: OnEnable (only if enabled=true)
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid() && obj->GetField<bool>("enabled"))
                        obj->Invoke<void>("OnEnable");
                }
            }
        }

        // --- Connect runtime component lifecycle signals ---
        // These handle dynamic component additions/removals during play mode.
        // Signals are disconnected in OnRuntimeStop().
        m_Registry.on_construct<RigidBody2DComponent>().connect<&Scene::OnRigidBody2DComponentConstruct>(this);
        m_Registry.on_destroy<RigidBody2DComponent>().connect<&Scene::OnRigidBody2DComponentDestroy>(this);
        m_Registry.on_construct<BoxCollider2DComponent>().connect<&Scene::OnBoxCollider2DComponentConstruct>(this);
        m_Registry.on_construct<CircleCollider2DComponent>().connect<&Scene::OnCircleCollider2DComponentConstruct>(this);
        m_Registry.on_construct<RigidBodyComponent>().connect<&Scene::OnRigidBodyComponentConstruct>(this);
        m_Registry.on_destroy<RigidBodyComponent>().connect<&Scene::OnRigidBodyComponentDestroy>(this);

        // --- Initial batch sync for existing entities ---
        // entt signals only fire for future construct/destroy events, so we must
        // manually iterate existing components to create their runtime physics bodies.

        // Box2D bodies
        {
            auto view = m_Registry.view<RigidBody2DComponent>();
            for (auto entity : view)
                OnRigidBody2DComponentConstruct(m_Registry, entity);
        }

        // Box2D box colliders
        {
            auto view = m_Registry.view<BoxCollider2DComponent>();
            for (auto entity : view)
                OnBoxCollider2DComponentConstruct(m_Registry, entity);
        }

        // Box2D circle colliders
        {
            auto view = m_Registry.view<CircleCollider2DComponent>();
            for (auto entity : view)
                OnCircleCollider2DComponentConstruct(m_Registry, entity);
        }

        // PhysX 3D physics: initial sync
        {
            Physics3D::SetCollisionScene(this);
            auto physxView = m_Registry.view<PhysXSceneComponent>();
            if (!physxView.empty())
            {
                physx::PxScene* physxScene = m_Registry.get<PhysXSceneComponent>(physxView.front()).World;

                // Create rigid body actors
                auto rigidBodyView = m_Registry.view<RigidBodyComponent>();
                for (auto entity : rigidBodyView)
                    OnRigidBodyComponentConstruct(m_Registry, entity);

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
        // Disconnect runtime signals (prevent callbacks during teardown)
        m_Registry.on_construct<RigidBody2DComponent>().disconnect(this);
        m_Registry.on_destroy<RigidBody2DComponent>().disconnect(this);
        m_Registry.on_construct<BoxCollider2DComponent>().disconnect(this);
        m_Registry.on_construct<CircleCollider2DComponent>().disconnect(this);
        m_Registry.on_construct<RigidBodyComponent>().disconnect(this);
        m_Registry.on_destroy<RigidBodyComponent>().disconnect(this);

        // Clear physics collision scene pointers
        s_Box2DContactListener.CurrentScene = nullptr;
        Physics3D::SetCollisionScene(nullptr);

        // Cleanup C# script runtime — OnDisable → OnDestroy, then clear storage
        {
            auto view = m_Registry.view<CSharpScriptComponent>();
            UUID sceneID = GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Registry.get<CSharpScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid())
                    {
                        if (obj->GetPropertyValue<Rolky::Bool32>("Enabled"))
                            obj->InvokeMethod("OnDisable");
                        obj->InvokeMethod("OnDestroy");
                    }
                }
                comp.Behaviours.clear();
            }
            m_CSharpScriptStorage->Clear();
        }

        // Cleanup Python script runtime — OnDisable → OnDestroy, then clear storage
        {
            auto view = m_Registry.view<PythonScriptComponent>();
            UUID sceneID = GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Registry.get<PythonScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid())
                    {
                        if (obj->GetField<bool>("enabled"))
                            obj->Invoke<void>("OnDisable");
                        obj->Invoke<void>("OnDestroy");
                    }
                }
                comp.Behaviours.clear();
            }
            m_PythonScriptStorage->Clear();
        }

        // Cleanup managed object maps for this scene
        UUID currentSceneID = GetUUID();
        CSharpScriptEngine::s_ManagedObjects.erase(currentSceneID);
        PythonScriptEngine::s_PythonScriptObjects.erase(currentSceneID);

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
        entity.AddComponent<CSharpScriptComponent>();
        entity.AddComponent<PythonScriptComponent>();
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
        entity.AddComponent<CSharpScriptComponent>();
        entity.AddComponent<PythonScriptComponent>();
        if (!name.empty())
            entity.AddComponent<TagComponent>(name);

        PR_CORE_ASSERT(m_EntityIDMap.find(uuid) == m_EntityIDMap.end());
        m_EntityIDMap[uuid] = entity;
        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        if (!entity)
            return;

        // Trigger on_destroy handlers for script components while entity is still intact,
        // so callbacks can safely access remaining components (IDComponent, TransformComponent).
        m_Registry.remove<CSharpScriptComponent>(entity.m_EntityHandle);
        m_Registry.remove<PythonScriptComponent>(entity.m_EntityHandle);

        // Destroy entity — on_destroy signals for physics (runtime-only) fire automatically.
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
        CopyComponentIfExists<CSharpScriptComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
        CopyComponentIfExists<PythonScriptComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
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

    Entity Scene::TryGetEntityByUUID(UUID uuid)
    {
        auto it = m_EntityIDMap.find(uuid);
        if (it != m_EntityIDMap.end())
            return it->second;
        return {};
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

        CSharpScriptEngine::SetSceneContext(target);
        PythonScriptEngine::SetSceneContext(target);

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
        CopyComponent<CSharpScriptComponent>(target->m_Registry, m_Registry, enttMap);
        CopyComponent<PythonScriptComponent>(target->m_Registry, m_Registry, enttMap);
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


        CSharpScriptEngine::SetSceneContext(this);
        PythonScriptEngine::SetSceneContext(this);

        target->SetPhysics2DGravity(GetPhysics2DGravity());
    }

    // Collision dispatch — invoke matching method on all script groups for this entity
    void Scene::OnCollision2DBegin(Entity entity) { OnCollisionBegin(entity); }
    void Scene::OnCollision2DEnd(Entity entity)   { OnCollisionEnd(entity); }

    void Scene::OnCollisionBegin(Entity entity)
    {
        UUID sceneID = GetUUID();

        if (entity.HasComponent<CSharpScriptComponent>())
        {
            auto& comp = entity.GetComponent<CSharpScriptComponent>();
            for (auto& binding : comp.Behaviours)
            {
                auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                if (obj && obj->IsValid())
                    obj->TryInvokeMethod("OnCollisionBegin", 0.0f);
            }
        }

        if (entity.HasComponent<PythonScriptComponent>())
        {
            auto& comp = entity.GetComponent<PythonScriptComponent>();
            for (auto& binding : comp.Behaviours)
            {
                auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                if (obj && obj->IsValid())
                    obj->Invoke<void>("OnCollisionBegin", 0.0f);
            }
        }
    }

    void Scene::OnCollisionEnd(Entity entity)
    {
        UUID sceneID = GetUUID();

        if (entity.HasComponent<CSharpScriptComponent>())
        {
            auto& comp = entity.GetComponent<CSharpScriptComponent>();
            for (auto& binding : comp.Behaviours)
            {
                auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                if (obj && obj->IsValid())
                    obj->TryInvokeMethod("OnCollisionEnd", 0.0f);
            }
        }

        if (entity.HasComponent<PythonScriptComponent>())
        {
            auto& comp = entity.GetComponent<PythonScriptComponent>();
            for (auto& binding : comp.Behaviours)
            {
                auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                if (obj && obj->IsValid())
                    obj->Invoke<void>("OnCollisionEnd", 0.0f);
            }
        }
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


    // ============================================================
    // Component lifecycle callbacks (entt signal handlers)
    // ============================================================

    // --- Script: CSharpScriptComponent ---

    void Scene::OnCSharpScriptComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        Entity e = { entity, this };
        if (!e.HasComponent<IDComponent>())
            return;

        uint64_t entityID = (uint64_t)e.GetComponent<IDComponent>().ID;
        CSharpScriptEngine::InstantiateEngine(entityID, "Prism.Entity", *m_CSharpScriptStorage);
        auto& comp = registry.get<CSharpScriptComponent>(entity);
        comp.ScriptID = entityID;
    }

    void Scene::OnCSharpScriptComponentDestroy(entt::registry& registry, entt::entity entity)
    {
        auto& comp = registry.get<CSharpScriptComponent>(entity);
        UUID sceneID = GetUUID();

        // Destroy all Behaviour instances first
        for (auto& binding : comp.Behaviours)
        {
            auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
            if (obj && obj->IsValid())
                obj->InvokeMethod("OnDestroy");
        }
        comp.Behaviours.clear();

        // Destroy the Entity managed object
        if (comp.ScriptID)
        {
            auto& entry = CSharpScriptEngine::GetEntityScriptStorage(*m_CSharpScriptStorage, comp.ScriptID);
            if (entry.Instance->IsValid())
                entry.Instance->InvokeMethod("OnDestroy");
            CSharpScriptEngine::RemoveManagedObject(*m_CSharpScriptStorage, comp.ScriptID);
            comp.ScriptID = 0;
        }
    }

    // --- Script: PythonScriptComponent ---

    void Scene::OnPythonScriptComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        Entity e = { entity, this };
        if (!e.HasComponent<IDComponent>())
            return;
        uint64_t entityID = (uint64_t)e.GetComponent<IDComponent>().ID;
        PythonScriptEngine::Instantiate(entityID, "Prism.Entity", *m_PythonScriptStorage);
        auto& comp = registry.get<PythonScriptComponent>(entity);
        comp.ScriptID = entityID;
    }

    void Scene::OnPythonScriptComponentDestroy(entt::registry& registry, entt::entity entity)
    {
        auto& comp = registry.get<PythonScriptComponent>(entity);
        UUID sceneID = GetUUID();

        // Destroy all Behaviour instances first
        for (auto& binding : comp.Behaviours)
        {
            auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
            if (obj && obj->IsValid())
                obj->Invoke<void>("OnDestroy");
        }
        comp.Behaviours.clear();

        // Destroy the Entity managed object
        if (comp.ScriptID)
        {
            auto& entry = PythonScriptEngine::GetEntityScriptStorage(*m_PythonScriptStorage, comp.ScriptID);
            if (entry.Instance && entry.Instance->IsValid())
                entry.Instance->Invoke<void>("OnDestroy");
            PythonScriptEngine::RemoveScriptObject(*m_PythonScriptStorage, comp.ScriptID);
            comp.ScriptID = 0;
        }
    }

    // --- Box2D 2D: RigidBody2DComponent ---

    void Scene::OnRigidBody2DComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        Entity e = { entity, this };
        auto& tc = e.Transform();
        auto& rigidBody2D = registry.get<RigidBody2DComponent>(entity);
        auto& world = registry.get<Box2DWorldComponent>(m_SceneEntity).World;

        b2BodyDef bodyDef;
        if (rigidBody2D.BodyType == RigidBody2DComponent::Type::Static)
            bodyDef.type = b2_staticBody;
        else if (rigidBody2D.BodyType == RigidBody2DComponent::Type::Dynamic)
            bodyDef.type = b2_dynamicBody;
        else if (rigidBody2D.BodyType == RigidBody2DComponent::Type::Kinematic)
            bodyDef.type = b2_kinematicBody;
        bodyDef.position.Set(tc.Position.x, tc.Position.y);
        bodyDef.angle = glm::eulerAngles(tc.Rotation).z;

        b2Body* body = world->CreateBody(&bodyDef);
        body->SetFixedRotation(rigidBody2D.FixedRotation);
        body->GetUserData().pointer = (uintptr_t)e.GetUUID();
        rigidBody2D.RuntimeBody = body;
    }

    void Scene::OnRigidBody2DComponentDestroy(entt::registry& registry, entt::entity entity)
    {
        auto& rigidBody2D = registry.get<RigidBody2DComponent>(entity);
        if (rigidBody2D.RuntimeBody)
        {
            auto& world = registry.get<Box2DWorldComponent>(m_SceneEntity).World;
            world->DestroyBody(static_cast<b2Body*>(rigidBody2D.RuntimeBody));
            rigidBody2D.RuntimeBody = nullptr;
        }
    }

    // --- Box2D 2D: BoxCollider2DComponent ---

    void Scene::OnBoxCollider2DComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        Entity e = { entity, this };
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

    // --- Box2D 2D: CircleCollider2DComponent ---

    void Scene::OnCircleCollider2DComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        Entity e = { entity, this };
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

    // --- PhysX 3D: RigidBodyComponent ---

    void Scene::OnRigidBodyComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        auto physxView = registry.view<PhysXSceneComponent>();
        if (physxView.empty())
            return;

        physx::PxScene* physxScene = registry.get<PhysXSceneComponent>(physxView.front()).World;
        if (!physxScene)
            return;

        Entity e = { entity, this };
        auto& rb = registry.get<RigidBodyComponent>(entity);

        physx::PxRigidActor* actor = Physics3D::CreateAndAddActor(physxScene, rb, e.Transform().GetTransform());
        if (actor)
        {
            actor->userData = (void*)(uintptr_t)e.GetUUID();
            rb.RuntimeActor = actor;
        }
    }

    void Scene::OnRigidBodyComponentDestroy(entt::registry& registry, entt::entity entity)
    {
        auto& rb = registry.get<RigidBodyComponent>(entity);
        if (!rb.RuntimeActor)
            return;

        auto physxView = registry.view<PhysXSceneComponent>();
        if (!physxView.empty())
        {
            physx::PxScene* pxScene = registry.get<PhysXSceneComponent>(physxView.front()).World;
            if (pxScene)
                pxScene->removeActor(*static_cast<physx::PxRigidActor*>(rb.RuntimeActor));
        }

        static_cast<physx::PxRigidActor*>(rb.RuntimeActor)->release();
        rb.RuntimeActor = nullptr;
    }
}
