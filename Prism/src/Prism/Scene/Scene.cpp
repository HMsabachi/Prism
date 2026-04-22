#include "prpch.h"
#include "Scene.h"

#include "Prism/Events/Event.h"
#include "Entity.h"
#include "Components.h"

#include "Scripting/ScriptEngine.h"

#include "Prism/Renderer/Renderer2D.h"
#include "Prism/Renderer/SceneRenderer.h"

namespace Prism 
{

	static const std::string DefaultEntityName = "Entity";

	std::unordered_map<uint32_t, Scene*> s_ActiveScenes;

	struct SceneComponent
	{
		uint32_t SceneID;
	};

	static uint32_t s_SceneIDCounter = 0; // 场景 ID 计数器

	void OnTransformConstruct(entt::registry& registry, entt::entity entity)
	{
		// PR_CORE_TRACE("Transform Component constructed!");
	}

	void OnScriptComponentConstruct(entt::registry& registry, entt::entity entity)
	{
		// Note: there should be exactly one scene component per registry
		auto view = registry.view<SceneComponent>();
		uint32_t sceneID = 0;
		for (auto entity : view)
		{
			auto& scene = registry.get<SceneComponent>(entity);
			sceneID = scene.SceneID;
		}
		// TODO: 这里还未完成 ScriptComponent的构造
		ScriptEngine::OnInitEntity(registry.get<ScriptComponent>(entity), (uint32_t)entity, sceneID);
	}

	Scene::Scene(const std::string& debugName)
		: m_DebugName(debugName), m_SceneID(++s_SceneIDCounter)
	{
		m_Registry.on_construct<TransformComponent>().connect<&OnTransformConstruct>();
		m_Registry.on_construct<ScriptComponent>().connect<&OnScriptComponentConstruct>();

		m_SceneEntity = m_Registry.create();
		m_Registry.emplace<SceneComponent>(m_SceneEntity, m_SceneID);

		s_ActiveScenes[m_SceneID] = this;
		Init();
	}

	Scene::~Scene()
	{
		m_Registry.clear();
		s_ActiveScenes.erase(m_SceneID);
	}

	void Scene::Init()
	{
#if 1
		auto skyboxShader = PrismShader::Create("assets/shaders/Skybox.Shader");
		m_SkyboxMaterial = MaterialInstance::Create(Material::Create(skyboxShader));
#endif
	}

	void Scene::OnUpdate()
	{
		float ts = Time::GetDeltaTime();

		Camera* camera = nullptr;
		{
			auto view = m_Registry.view<CameraComponent>();
			for (auto entity : view)
			{
				auto& comp = view.get<CameraComponent>(entity);
				camera = &comp.Camera;
				break;
			}
		}
		PR_CORE_ASSERT(camera, "场景中没有任何相机");
		camera->OnUpdate();

		m_SkyboxMaterial->Set("u_TextureLod", m_SkyboxLod);

		auto view = m_Registry.view<ScriptComponent>();
		for (auto entity : view)
			ScriptEngine::OnUpdateEntity((uint32_t)entity, ts);
#if 0

		// Render all sprites
		Renderer2D::BeginScene(*camera);
		{
			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRenderer>);
			for (auto entity : group)
			{
				auto [transformComponent, spriteRendererComponent] = group.get<TransformComponent, SpriteRenderer>(entity);
				if (spriteRendererComponent.Texture)
					Renderer2D::DrawQuad(transformComponent.Transform, spriteRendererComponent.Texture, spriteRendererComponent.TilingFactor);
				else
					Renderer2D::DrawQuad(transformComponent.Transform, spriteRendererComponent.Color);
			}
		}

		Renderer2D::EndScene();
#endif

		// 渲染3D场景
		m_SkyboxMaterial->Set("u_TextureLod", m_SkyboxLod);
		// 更新 所有 MeshComponent
		auto entities = m_Registry.view<MeshComponent>();
		for (auto entity : entities)
		{
			auto& meshComponent = m_Registry.get<MeshComponent>(entity);
		}
		auto group = m_Registry.group<MeshComponent>(entt::get<TransformComponent>);
		SceneRenderer::BeginScene(this, *camera);
		for (auto entity : group)
		{
			auto [transformComponent, meshComponent] = group.get<TransformComponent, MeshComponent>(entity);
			if (meshComponent.Mesh)
			{
				meshComponent.Mesh->OnUpdate(ts);

				// TODO: Should we render (logically)
				SceneRenderer::SubmitMesh(meshComponent, transformComponent, nullptr);
			}
		}
		SceneRenderer::EndScene();

	}

	void Scene::OnEvent(Event& e)
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& comp = view.get<CameraComponent>(entity);
			comp.Camera.OnEvent(e);
			break;
		}
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

	Entity Scene::CreateEntity(const std::string& name)
	{
		auto entity = Entity{ m_Registry.create(), this };
		entity.AddComponent<TransformComponent>(glm::mat4(1.0f));
		if (!name.empty())
			entity.AddComponent<TagComponent>(name);
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity.m_EntityHandle);
	}

	Environment Environment::Load(const std::string& filepath)
	{
		auto [radiance, irradiance] = SceneRenderer::CreateEnvironmentMap(filepath);
		return { radiance, irradiance };
	}
}