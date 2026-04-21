#pragma once
#include "Prism/Renderer/Camera/Camera.h"
#include "Entity.h"

namespace Prism
{
	struct Event;
	class MaterialInstance;
	class TextureCube;
	class Texture2D;
}

namespace Prism
{

	struct PRISM_API Environment
	{
		Ref<TextureCube> RadianceMap;
		Ref<TextureCube> IrradianceMap;

		static Environment Load(const std::string& filepath);
	};
	const size_t MAX_LIGHTS = 1;
	struct Light
	{
		alignas(16) glm::vec3 Direction;
		alignas(16) glm::vec3 Radiance;
		alignas(4) float Multiplier = 1.0f;
	};

	class PRISM_API Scene : public RefCounted
	{
	public:
		Scene(const std::string& debugName = "Scene");
		~Scene();

		void Init();

		void OnUpdate();
		void OnEvent(Event& e);

		void SetCamera(const Camera& camera);
		Camera& GetCamera() { return m_Camera; }

		void SetEnvironment(const Environment& environment);
		void SetSkybox(const Ref<TextureCube>& skybox);

		Light& GetLight() { return m_Light; }

		float& GetSkyboxLod() { return m_SkyboxLod; }

		void AddEntity(Entity* entity);
		Entity* CreateEntity(const std::string& name = "");
	private:
		std::string m_DebugName;
		std::vector<Entity*> m_Entities;
		Camera m_Camera;

		Light m_Light;
		float m_LightMultiplier = 0.3f;

		Environment m_Environment;
		Ref<TextureCube> m_SkyboxTexture;
		Ref<MaterialInstance> m_SkyboxMaterial;

		float m_SkyboxLod = 0.0f;

		friend class SceneRenderer;
		friend class SceneHierarchyPanel;
	};

}