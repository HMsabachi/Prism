#include "prpch.h"
#include "Scene.h"

#include "Prism/Renderer/SceneRenderer.h"

namespace Prism {

	Scene::Scene(const std::string& debugName)
		: m_DebugName(debugName)
	{
		Init();
	}

	Scene::~Scene()
	{
		for (Entity* entity : m_Entities)
			delete entity;
	}

	void Scene::Init()
	{
#if 0
		auto skyboxShader = Shader::Create("assets/shaders/Skybox.glsl");
		m_SkyboxMaterial = MaterialInstance::Create(Material::Create(skyboxShader));
		m_SkyboxMaterial->SetFlag(MaterialFlag::DepthTest, false);
#endif
	}

	void Scene::OnUpdate()
	{
		float ts = Time::GetDeltaTime();
		m_Camera.Update();

		m_SkyboxMaterial->Set("u_TextureLod", m_SkyboxLod);

		// Update all entities
		for (auto entity : m_Entities)
		{
			auto mesh = entity->GetMesh();
			if (mesh)
				mesh->OnUpdate(ts);
		}

		SceneRenderer::BeginScene(this);

		// Render entities
		for (auto entity : m_Entities)
		{
			// TODO: Should we render (logically)
			SceneRenderer::SubmitEntity(entity);
		}

		SceneRenderer::EndScene();
	}

	void Scene::SetCamera(const Camera& camera)
	{
		m_Camera = camera;
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

	void Scene::AddEntity(Entity* entity)
	{
		m_Entities.push_back(entity);
	}

	Entity* Scene::CreateEntity()
	{
		Entity* entity = new Entity();
		AddEntity(entity);
		return entity;
	}

	Environment Environment::Load(const std::string& filepath)
	{
		auto [radiance, irradiance] = SceneRenderer::CreateEnvironmentMap(filepath);
		return { radiance, irradiance };
	}
}