#pragma once

#include "Scene.h"
#include "Components.h"

namespace Prism
{

    class PRISM_API Entity
    {
    public:
        Entity() = default;
        Entity(entt::entity handle, Scene* scene)
            : m_EntityHandle(handle), m_Scene(scene) {
        }

        ~Entity() {}

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            PR_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
            return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
        }

        template<typename T>
        T& GetComponent()
        {
            PR_CORE_ASSERT(HasComponent<T>(), "Entity doesn't have component!");
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template<typename T>
        const T& GetComponent() const
        {
            PR_CORE_ASSERT(HasComponent<T>(), "Entity doesn't have component!");
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template<typename T>
        bool HasComponent()
        {
            return m_Scene->m_Registry.any_of<T>(m_EntityHandle);
        }

        template<typename T>
        bool HasComponent() const
        {
            return m_Scene->m_Registry.any_of<T>(m_EntityHandle);
        }

        template<typename T>
        void RemoveComponent()
        {
            PR_CORE_ASSERT(HasComponent<T>(), "Entity doesn't have component!");
            m_Scene->m_Registry.remove<T>(m_EntityHandle);
        }

        Transform& Transformation() { return m_Scene->m_Registry.get<TransformComponent>(m_EntityHandle); }
        const Transform& Transformation() const { return m_Scene->m_Registry.get<TransformComponent>(m_EntityHandle); }


        operator uint32_t () const { return (uint32_t)m_EntityHandle; }
        operator entt::entity() const { return m_EntityHandle; }
        operator bool() const { return (uint32_t)m_EntityHandle && m_Scene; }

        bool operator==(const Entity& other) const
        {
            return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
        }

        bool operator!=(const Entity& other) const
        {
            return !(*this == other);
        }

        UUID GetUUID() { return GetComponent<IDComponent>().ID; }
        UUID GetSceneUUID() { return m_Scene->GetUUID(); }
        Scene* GetScene() const { return m_Scene; }

        entt::entity& Parent() { return m_Parent; }
        const entt::entity& Parent() const { return m_Parent; }
        std::vector<entt::entity>& Children() { return m_Children; }
        const std::vector<entt::entity>& Children() const { return m_Children; }
        void AddChild(Entity child) { m_Children.push_back((entt::entity)child); }
    private:
        Entity(const std::string& name);
    private:
        entt::entity m_EntityHandle;
        Scene* m_Scene = nullptr;

        entt::entity m_Parent = entt::null;
        std::vector<entt::entity> m_Children;

        friend class Scene;
        friend class SceneSerializer;
    };

}
