#include "prpch.h"
#include "Scene.h"

#include "Prism/Events/Event.h"
#include "Entity.h"
#include "Components.h"

#include "Prism/Renderer/Renderer2D.h"
#include "Prism/Renderer/Renderer.h"

#include "Prism/Editor/EditorCamera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Systems/Physics2DSystem.h"
#include "Systems/Physics3DSystem.h"
#include "Systems/ScriptSystem.h"
#include "Systems/TransformSyncSystem.h"
#include "Systems/RenderSystem.h"

#include "Scripting/CSharp/CSharpScriptEngine.h"
#include "Scripting/Python/PythonScriptEngine.h"
#include "Prism/Renderer/Material.h"

namespace Prism
{

    static const std::string DefaultEntityName = "Entity";

    std::unordered_map<UUID, Scene*> s_ActiveScenes;

    struct SceneComponent
    {
        UUID SceneID;
    };

    Scene::Scene(const std::string& debugName, bool isEditorScene)
        : m_DebugName(debugName)
    {
        m_SceneEntity = m_Registry.create();
        m_Registry.emplace<SceneComponent>(m_SceneEntity, m_SceneID);

        s_ActiveScenes[m_SceneID] = this;

        AddSystem<ScriptSystem>(this);
        AddSystem<TransformSyncSystem>(this);
        AddSystem<Physics3DSystem>(this);
        AddSystem<Physics2DSystem>(this);
        AddSystem<RenderSystem>(this);
        for (auto& s : m_SystemOrder)
            s->OnCreate();
        Init();
    }

    Scene::~Scene()
    {
        m_Registry.clear();
        s_ActiveScenes.erase(m_SceneID);
        for (auto& s : m_SystemOrder)
            s->OnDestroy();
    }

    void Scene::Init()
    {
    }

    void Scene::OnUpdate()
    {
        PR_PROFILE_FUNCTION();
        float dt = Time::GetDeltaTime();
        

        for (auto* sys : m_SystemOrder) sys->OnEarlyUpdate(dt);

        if (Time::ShouldFixedUpdate())
        {
            float fixedDt = Time::GetFixedDeltaTime();
            PR_PROFILE_SCOPE("FixedUpdate");
            for (auto* sys : m_SystemOrder) sys->OnFixedUpdate(fixedDt);
        }

        for (auto* sys : m_SystemOrder) sys->OnPreUpdate(dt);
        for (auto* sys : m_SystemOrder) sys->OnUpdate(dt);
        for (auto* sys : m_SystemOrder) sys->OnPreLateUpdate(dt);
        for (auto* sys : m_SystemOrder) sys->OnLateUpdate(dt);
        for (auto* sys : m_SystemOrder) sys->OnPostLateUpdate(dt);
        for (auto* sys : m_SystemOrder) sys->OnImGuiRender();
    }

    void Scene::OnEvent(Event& e)
    {
    }
    void Scene::OnRuntimeStart()
    {
        for (auto* sys : m_SystemOrder) sys->OnRuntimeStart();
        m_IsPlaying = true;
        Time::ShouldFixedUpdate();
    }

    void Scene::OnRuntimeStop()
    {
        for (auto* sys : m_SystemOrder) sys->OnRuntimeStop();
        m_IsPlaying = false;
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
            dstRegistry.remove<T>(destEntity);
            dstRegistry.emplace<T>(destEntity, srcComponent);
        }
    }

    template<typename T>
    static void CopyComponentIfExists(entt::entity dst, entt::entity src, entt::registry& registry)
    {
        if (registry.any_of<T>(src))
        {
            auto& srcComponent = registry.get<T>(src);
            registry.remove<T>(dst);
            registry.emplace<T>(dst, srcComponent);
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
        CopyComponentIfExists<MeshRendererComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
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
        CopyComponentIfExists<DirectionalLightComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
        CopyComponentIfExists<SkyLightComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
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
        if (auto* srcRS = GetSystem<RenderSystem>())
            if (auto* dstRS = target->GetSystem<RenderSystem>())
                dstRS->GetConfig() = srcRS->GetConfig();

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
        CopyComponent<MeshRendererComponent>(target->m_Registry, m_Registry, enttMap);
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
        CopyComponent<DirectionalLightComponent>(target->m_Registry, m_Registry, enttMap);
        CopyComponent<SkyLightComponent>(target->m_Registry, m_Registry, enttMap);


        CSharpScriptEngine::SetSceneContext(this);
        PythonScriptEngine::SetSceneContext(this);

        if (auto* p2d = GetSystem<Physics2DSystem>())
            if (auto* targetP2d = target->GetSystem<Physics2DSystem>())
                targetP2d->SetGravity(p2d->GetGravity());
    }

    Ref<Scene> Scene::GetScene(UUID uuid)
    {
        if (s_ActiveScenes.find(uuid) != s_ActiveScenes.end())
            return s_ActiveScenes.at(uuid);

        return {};
    }

}
