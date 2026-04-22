#pragma once
#include "Prism/Renderer/Camera/Camera.h"
#include "Prism/Renderer/Texture.h"
#include <entt/entt.hpp>

namespace Prism
{
	struct Event;
	class Entity;
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

		void SetEnvironment(const Environment& environment);
		void SetSkybox(const Ref<TextureCube>& skybox);

		Light& GetLight() { return m_Light; }

		float& GetSkyboxLod() { return m_SkyboxLod; }

		Entity CreateEntity(const std::string& name = "");
		void DestroyEntity(Entity entity);

		template<typename T>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<T>();
		}
	private:
		uint32_t m_SceneID;
		entt::entity m_SceneEntity;
		entt::registry m_Registry;

		std::string m_DebugName;

		Light m_Light;
		float m_LightMultiplier = 0.3f;

		Environment m_Environment;
		Ref<TextureCube> m_SkyboxTexture;
		Ref<MaterialInstance> m_SkyboxMaterial;

		float m_SkyboxLod = 0.0f;

		friend class Entity;
		friend class SceneRenderer;
		friend class SceneHierarchyPanel;
	};

}