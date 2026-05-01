#pragma once
#include "Prism/Core/UUID.h"


#include <entt/entt.hpp>

namespace Prism
{
	struct Event;
	class Entity;
	class MaterialInstance;
	class TextureCube;
	class Texture2D;
	class EditorCamera;
}

namespace Prism
{

	struct PRISM_API Environment
	{
		std::string FilePath;
		Ref<TextureCube> RadianceMap;
		Ref<TextureCube> IrradianceMap;

		static Environment Load(const std::string& filepath);
	};
	const size_t MAX_LIGHTS = 1;
	struct Light
	{
		alignas(16) glm::vec3 Direction{ 0.0f, 0.0f, 0.0f };
		alignas(16) glm::vec3 Radiance{ 0.0f, 0.0f, 0.0f };
		alignas(4) float Multiplier = 1.0f;
	};

	using EntityMap = std::unordered_map<UUID, Entity>;


	class PRISM_API Scene : public RefCounted
	{
	public:
		Scene(const std::string& debugName = "Scene");
		~Scene();

		void Init();

		void OnUpdate();
		void OnRenderRuntime();
		void OnRenderEditor(const EditorCamera& editorCamera);
		void OnEvent(Event& e);

		// Runtime
		void OnRuntimeStart();
		void OnRuntimeStop();

		void SetViewportSize(uint32_t width, uint32_t height);

		void SetEnvironment(const Environment& environment);
		const Environment& GetEnvironment() const { return m_Environment; }
		void SetSkybox(const Ref<TextureCube>& skybox);

		Light& GetLight() { return m_Light; }
		const Light& GetLight() const { return m_Light; }

		Entity GetMainCameraEntity();

		float& GetSkyboxLod() { return m_SkyboxLod; }

		Entity CreateEntity(const std::string& name = "");
		Entity CreateEntityWithID(UUID uuid, const std::string& name = "", bool runtimeMap = false);
		void DestroyEntity(Entity entity);
		void DuplicateEntity(Entity entity);

		template<typename T>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<T>();
		}

		const EntityMap& GetEntityMap() const { return m_EntityIDMap; }
		void CopyTo(Ref<Scene>& target);

		UUID GetUUID() const { return m_SceneID; }

		static Ref<Scene> GetScene(UUID uuid);

		// Editor-specific
		void SetSelectedEntity(entt::entity entity) { m_SelectedEntity = entity; }
	private:
		UUID m_SceneID;
		entt::entity m_SceneEntity;
		entt::registry m_Registry;

		std::string m_DebugName;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		EntityMap m_EntityIDMap;

		Light m_Light;
		float m_LightMultiplier = 0.3f;

		Environment m_Environment;
		Ref<TextureCube> m_SkyboxTexture;
		Ref<MaterialInstance> m_SkyboxMaterial;

		entt::entity m_SelectedEntity;

		float m_SkyboxLod = 0.0f;
		bool m_IsPlaying = false;

		friend class Entity;
		friend class SceneRenderer;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;

		friend void OnScriptComponentConstruct(entt::registry& registry, entt::entity entity);
	};

}