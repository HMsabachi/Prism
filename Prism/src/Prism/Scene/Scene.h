#pragma once
#include "Prism/Core/UUID.h"

#include <entt/entt.hpp>
#include "Prism/Utilities/TypeInfo.h"
#include <unordered_map>
#include <vector>
#include <memory>

namespace Prism
{
    class Event;
    class Entity;
    class ISystem;
}

namespace Prism
{


    using EntityMap = std::unordered_map<UUID, Entity>;


    class PRISM_API Scene : public RefCounted
    {
    public:
        Scene(const std::string& debugName = "Scene", bool isEditorScene = false);
        ~Scene();

        void Init();

        void OnUpdate();
        void OnEvent(Event& e);

        // Runtime
        void OnRuntimeStart();
        void OnRuntimeStop();
        void OnImGuiRender();

        Entity GetMainCameraEntity();

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
        Entity FindEntityByHandle(uint32_t handle);
        Entity TryGetEntityByUUID(UUID uuid);

        const EntityMap& GetEntityMap() const { return m_EntityIDMap; }

        void CopyTo(Ref<Scene>& target);

        UUID GetUUID() const { return m_SceneID; }

        static Ref<Scene> GetScene(UUID uuid);

        // System management
        template<typename T>
        static uint32_t GetSystemTypeHash()
        {
            static uint32_t hash = TypeInfo<T>().HashCode();
            return hash;
        }

        template<typename T, typename... Args>
        T* AddSystem(Args&&... args)
        {
            uint32_t hash = GetSystemTypeHash<T>();
            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = system.get();
            m_Systems[hash] = std::move(system);
            m_SystemOrder.push_back(ptr);
            return ptr;
        }

        template<typename T>
        T* GetSystem() const
        {
            uint32_t hash = GetSystemTypeHash<T>();
            auto it = m_Systems.find(hash);
            return it != m_Systems.end() ? static_cast<T*>(it->second.get()) : nullptr;
        }

        entt::registry& GetRegistry() { return m_Registry; }

        // Editor-specific
        void SetSelectedEntity(entt::entity entity) { m_SelectedEntity = entity; }
        entt::entity GetSelectedEntity() const { return m_SelectedEntity; }
    private:
        UUID m_SceneID;
        entt::entity m_SceneEntity;
        entt::registry m_Registry;

        std::string m_DebugName;

        EntityMap m_EntityIDMap;

        entt::entity m_SelectedEntity;

        bool m_IsPlaying = false;

        std::unordered_map<uint32_t, std::unique_ptr<ISystem>> m_Systems;
        std::vector<ISystem*> m_SystemOrder;

        friend class Entity;
        friend class SceneHierarchyPanel;
        friend class SceneSerializer;
    };

}
