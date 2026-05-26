#pragma once
#include "Prism/Core/UUID.h"

#include <entt/entt.hpp>
#include "Systems/ISystem.h"
#include <vector>
#include <memory>

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
        alignas(16) glm::vec3 Direction{ -0.5f, -1.0f, -0.5f };
        alignas(16) glm::vec3 Radiance{ 1.0f, 1.0f, 1.0f };
        alignas(4) float Multiplier = 1.0f;
    };

    using EntityMap = std::unordered_map<UUID, Entity>;


    class PRISM_API Scene : public RefCounted
    {
    public:
        Scene(const std::string& debugName = "Scene");
        ~Scene();

        void Init();
        void OnShutdown();

        void OnUpdate();
        void OnFixedUpdate();
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

        bool IsShadowEnabled() const { return m_ShadowsEnabled; }
        void SetShadowEnabled(bool enabled) { m_ShadowsEnabled = enabled; }
        float GetShadowBias() const { return m_ShadowBias; }
        void SetShadowBias(float bias) { m_ShadowBias = bias; }
        float GetShadowNormalBias() const { return m_ShadowNormalBias; }
        void SetShadowNormalBias(float bias) { m_ShadowNormalBias = bias; }
        uint32_t GetCascadeCount() const { return m_CascadeCount; }
        void SetCascadeCount(uint32_t count) { m_CascadeCount = glm::clamp(count, 1u, 4u); }

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

        Entity FindEntityByTag(const std::string& tag);
        Entity TryGetEntityByUUID(UUID uuid);

        const EntityMap& GetEntityMap() const { return m_EntityIDMap; }

        void CopyTo(Ref<Scene>& target);

        UUID GetUUID() const { return m_SceneID; }

        static Ref<Scene> GetScene(UUID uuid);

        // System management
        template<typename T, typename... Args>
        T* AddSystem(Args&&... args)
        {
            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = system.get();
            m_Systems.push_back(std::move(system));
            return ptr;
        }

        template<typename T>
        T* GetSystem()
        {
            for (auto& sys : m_Systems)
                if (auto* ptr = dynamic_cast<T*>(sys.get()))
                    return ptr;
            return nullptr;
        }

        entt::registry& GetRegistry() { return m_Registry; }

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

        // 阴影设置
        bool m_ShadowsEnabled = true;
        float m_ShadowBias = 0.001f;
        float m_ShadowNormalBias = 0.1f;
        uint32_t m_CascadeCount = 4;

        Environment m_Environment;
        Ref<TextureCube> m_SkyboxTexture;
        Ref<MaterialInstance> m_SkyboxMaterial;

        entt::entity m_SelectedEntity;

        float m_SkyboxLod = 0.0f;
        bool m_IsPlaying = false;

        std::vector<std::unique_ptr<ISystem>> m_Systems;

        friend class Entity;
        friend class SceneRenderer;
        friend class SceneHierarchyPanel;
        friend class SceneSerializer;
    };

}
