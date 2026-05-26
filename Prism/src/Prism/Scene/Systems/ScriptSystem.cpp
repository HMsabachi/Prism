#include "prpch.h"
#include "ScriptSystem.h"
#include "../Scene.h"
#include "../Entity.h"
#include "../Components.h"

#include "Scripting/CSharp/CSharpScriptStorage.h"
#include "Scripting/Python/PythonScriptStorage.h"
#include "Scripting/CSharp/CSharpScriptEngine.h"
#include "Scripting/Python/PythonScriptEngine.h"

namespace Prism {

    ScriptSystem::ScriptSystem(Scene* scene)
        : m_Scene(scene)
    {
        m_CSharpScriptStorage = new CSharpScriptStorage();
        m_PythonScriptStorage = new PythonScriptStorage();

        auto& registry = scene->GetRegistry();
        registry.on_construct<CSharpScriptComponent>().connect<&ScriptSystem::OnCSharpScriptComponentConstruct>(this);
        registry.on_destroy<CSharpScriptComponent>().connect<&ScriptSystem::OnCSharpScriptComponentDestroy>(this);
        registry.on_construct<PythonScriptComponent>().connect<&ScriptSystem::OnPythonScriptComponentConstruct>(this);
        registry.on_destroy<PythonScriptComponent>().connect<&ScriptSystem::OnPythonScriptComponentDestroy>(this);
    }

    ScriptSystem::~ScriptSystem()
    {
        auto& registry = m_Scene->GetRegistry();
        registry.on_construct<CSharpScriptComponent>().disconnect(this);
        registry.on_destroy<CSharpScriptComponent>().disconnect(this);
        registry.on_construct<PythonScriptComponent>().disconnect(this);
        registry.on_destroy<PythonScriptComponent>().disconnect(this);

        if (m_CSharpScriptStorage->EntityStorage.size() > 0)
        {
            PR_CORE_WARN("[ScriptSystem] C# 脚本实例在销毁时可能会导致内存泄漏: {0} 个脚本实例.", m_CSharpScriptStorage->EntityStorage.size());
            m_CSharpScriptStorage->Clear();
        }
        if (m_PythonScriptStorage->EntityStorage.size() > 0)
        {
            PR_CORE_WARN("[ScriptSystem] Python 脚本实例在销毁时可能会导致内存泄漏: {0} 个脚本实例.", m_PythonScriptStorage->EntityStorage.size());
            m_PythonScriptStorage->Clear();
        }
        delete m_CSharpScriptStorage;
        m_CSharpScriptStorage = nullptr;
        delete m_PythonScriptStorage;
        m_PythonScriptStorage = nullptr;

        // Cleanup managed object maps for this scene
        UUID currentSceneID = m_Scene->GetUUID();
        CSharpScriptEngine::s_ManagedObjects.erase(currentSceneID);
        PythonScriptEngine::s_PythonScriptObjects.erase(currentSceneID);
    }

    void ScriptSystem::OnUpdate(float ts)
    {
        // C# Script OnUpdate
        {
            auto view = m_Scene->GetAllEntitiesWith<CSharpScriptComponent>();
            UUID sceneID = m_Scene->GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Scene->GetRegistry().get<CSharpScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                {
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnUpdate))
                        continue;
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid() && obj->GetPropertyValue<Rolky::Bool32>("Enabled"))
                        obj->InvokeMethod("OnUpdate");
                }
            }
        }

        // Python Script OnUpdate
        {
            auto view = m_Scene->GetAllEntitiesWith<PythonScriptComponent>();
            UUID sceneID = m_Scene->GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Scene->GetRegistry().get<PythonScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                {
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnUpdate))
                        continue;
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid() && obj->GetField<bool>("enabled"))
                        obj->Invoke<void>("OnUpdate");
                }
            }
        }

        // C# Script LateUpdate
        {
            auto view = m_Scene->GetAllEntitiesWith<CSharpScriptComponent>();
            UUID sceneID = m_Scene->GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Scene->GetRegistry().get<CSharpScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                {
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::LateUpdate))
                        continue;
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid() && obj->GetPropertyValue<Rolky::Bool32>("Enabled"))
                        obj->InvokeMethod("LateUpdate");
                }
            }
        }

        // Python Script LateUpdate
        {
            auto view = m_Scene->GetAllEntitiesWith<PythonScriptComponent>();
            UUID sceneID = m_Scene->GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Scene->GetRegistry().get<PythonScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                {
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::LateUpdate))
                        continue;
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid() && obj->GetField<bool>("enabled"))
                        obj->Invoke<void>("LateUpdate");
                }
            }
        }
    }

    void ScriptSystem::OnFixedUpdate(float ts)
    {
        // C# Script OnFixedUpdate
        {
            auto view = m_Scene->GetAllEntitiesWith<CSharpScriptComponent>();
            UUID sceneID = m_Scene->GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Scene->GetRegistry().get<CSharpScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                {
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnFixedUpdate))
                        continue;
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid() && obj->GetPropertyValue<Rolky::Bool32>("Enabled"))
                        obj->InvokeMethod("OnFixedUpdate");
                }
            }
        }

        // Python Script OnFixedUpdate
        {
            auto view = m_Scene->GetAllEntitiesWith<PythonScriptComponent>();
            UUID sceneID = m_Scene->GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Scene->GetRegistry().get<PythonScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                {
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnFixedUpdate))
                        continue;
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid() && obj->GetField<bool>("enabled"))
                        obj->Invoke<void>("OnFixedUpdate");
                }
            }
        }
    }

    void ScriptSystem::OnRuntimeStart()
    {
        CSharpScriptEngine::SetSceneContext(m_Scene);
        PythonScriptEngine::SetSceneContext(m_Scene);

        // C#: Create Behaviour instances, then Awake → OnCreate → OnEnable
        {
            auto view = m_Scene->GetRegistry().view<CSharpScriptComponent>();
            UUID sceneID = m_Scene->GetUUID();
            for (auto entity : view)
            {
                Entity e = { entity, m_Scene };
                auto& comp = m_Scene->GetRegistry().get<CSharpScriptComponent>(entity);
                // Create instances
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (!obj || !obj->IsValid())
                        CSharpScriptEngine::AddBehaviour(e, binding);
                }
                // Awake
                for (auto& binding : comp.Behaviours)
                {
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::Awake))
                        continue;
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid())
                        obj->InvokeMethod("Awake");
                }
                // OnCreate
                for (auto& binding : comp.Behaviours)
                {
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnCreate))
                        continue;
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid())
                        obj->InvokeMethod("OnCreate");
                }
                // OnEnable
                for (auto& binding : comp.Behaviours)
                {
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnEnable))
                        continue;
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid() && obj->GetPropertyValue<Rolky::Bool32>("Enabled"))
                        obj->InvokeMethod("OnEnable");
                }
            }
        }

        // Python: Create Behaviour instances, then Awake → OnCreate → OnEnable
        {
            auto view = m_Scene->GetRegistry().view<PythonScriptComponent>();
            UUID sceneID = m_Scene->GetUUID();
            for (auto entity : view)
            {
                Entity e = { entity, m_Scene };
                auto& comp = m_Scene->GetRegistry().get<PythonScriptComponent>(entity);
                // Create instances
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (!obj || !obj->IsValid())
                        PythonScriptEngine::AddBehaviour(e, binding);
                }
                // Awake
                for (auto& binding : comp.Behaviours)
                {
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::Awake))
                        continue;
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid())
                        obj->Invoke<void>("Awake");
                }
                // OnCreate
                for (auto& binding : comp.Behaviours)
                {
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnCreate))
                        continue;
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid())
                        obj->Invoke<void>("OnCreate");
                }
                // OnEnable 
                for (auto& binding : comp.Behaviours)
                {
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnEnable))
                        continue;
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid() && obj->GetField<bool>("enabled"))
                        obj->Invoke<void>("OnEnable");
                }
            }
        }
    }

    void ScriptSystem::OnRuntimeStop()
    {
        // Cleanup C# script runtime — OnDisable → OnDestroy, then clear storage
        {
            auto view = m_Scene->GetRegistry().view<CSharpScriptComponent>();
            UUID sceneID = m_Scene->GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Scene->GetRegistry().get<CSharpScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid())
                    {
                        if (obj->GetPropertyValue<Rolky::Bool32>("Enabled") && (binding.LifecycleMask & (uint16_t)LifecycleMethod::OnDisable))
                            obj->InvokeMethod("OnDisable");
                        if (binding.LifecycleMask & (uint16_t)LifecycleMethod::OnDestroy)
                            obj->InvokeMethod("OnDestroy");
                        Entity e = { entity, m_Scene };
                        CSharpScriptEngine::RemoveBehaviour(e, binding.BehaviourID);
                    }
                }
                comp.Behaviours.clear();
            }
        }

        // Cleanup Python script runtime — OnDisable → OnDestroy, then clear storage
        {
            auto view = m_Scene->GetRegistry().view<PythonScriptComponent>();
            UUID sceneID = m_Scene->GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Scene->GetRegistry().get<PythonScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                {
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid())
                    {
                        if (obj->GetField<bool>("enabled") && (binding.LifecycleMask & (uint16_t)LifecycleMethod::OnDisable))
                            obj->Invoke<void>("OnDisable");
                        if (binding.LifecycleMask & (uint16_t)LifecycleMethod::OnDestroy)
                            obj->Invoke<void>("OnDestroy");
                        Entity e = { entity, m_Scene };
                        PythonScriptEngine::RemoveBehaviour(e, binding.BehaviourID);
                    }
                }
                comp.Behaviours.clear();
            }
        }
    }

    void ScriptSystem::OnCollisionBegin(Entity entity)
    {
        UUID sceneID = m_Scene->GetUUID();

        if (entity.HasComponent<CSharpScriptComponent>())
        {
            auto& comp = entity.GetComponent<CSharpScriptComponent>();
            for (auto& binding : comp.Behaviours)
            {
                if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnCollisionBegin))
                    continue;
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
                if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnCollisionBegin))
                    continue;
                auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                if (obj && obj->IsValid())
                    obj->Invoke<void>("OnCollisionBegin", 0.0f);
            }
        }
    }

    void ScriptSystem::OnCollisionEnd(Entity entity)
    {
        UUID sceneID = m_Scene->GetUUID();

        if (entity.HasComponent<CSharpScriptComponent>())
        {
            auto& comp = entity.GetComponent<CSharpScriptComponent>();
            for (auto& binding : comp.Behaviours)
            {
                if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnCollisionEnd))
                    continue;
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
                if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnCollisionEnd))
                    continue;
                auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                if (obj && obj->IsValid())
                    obj->Invoke<void>("OnCollisionEnd", 0.0f);
            }
        }
    }

    // ============================================================
    // Component lifecycle callbacks (entt signal handlers)
    // ============================================================

    void ScriptSystem::OnCSharpScriptComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        CSharpScriptEngine::SetSceneContext(m_Scene);
        Entity e = { entity, m_Scene };
        if (!e.HasComponent<IDComponent>())
            return;

        uint64_t entityID = (uint64_t)e.GetComponent<IDComponent>().ID;
        CSharpScriptEngine::InstantiateEngine(entityID, "Prism.Entity", *m_CSharpScriptStorage);
        auto& comp = registry.get<CSharpScriptComponent>(entity);
        comp.ScriptID = entityID;
    }

    void ScriptSystem::OnCSharpScriptComponentDestroy(entt::registry& registry, entt::entity entity)
    {
        CSharpScriptEngine::SetSceneContext(m_Scene);
        auto& comp = registry.get<CSharpScriptComponent>(entity);
        UUID sceneID = m_Scene->GetUUID();

        if (comp.ScriptID)
        {
            auto& entry = CSharpScriptEngine::GetEntityScriptStorage(*m_CSharpScriptStorage, comp.ScriptID);
            /*if (entry.Instance->IsValid())
                entry.Instance->InvokeMethod("OnDestroy");*/
            CSharpScriptEngine::RemoveManagedObject(*m_CSharpScriptStorage, comp.ScriptID);
            comp.ScriptID = 0;
        }
    }

    void ScriptSystem::OnPythonScriptComponentConstruct(entt::registry& registry, entt::entity entity)
    {
        PythonScriptEngine::SetSceneContext(m_Scene);
        Entity e = { entity, m_Scene };
        if (!e.HasComponent<IDComponent>())
            return;
        uint64_t entityID = (uint64_t)e.GetComponent<IDComponent>().ID;
        PythonScriptEngine::Instantiate(entityID, "Prism.Entity", *m_PythonScriptStorage);
        auto& comp = registry.get<PythonScriptComponent>(entity);
        comp.ScriptID = entityID;
    }

    void ScriptSystem::OnPythonScriptComponentDestroy(entt::registry& registry, entt::entity entity)
    {
        PythonScriptEngine::SetSceneContext(m_Scene);
        auto& comp = registry.get<PythonScriptComponent>(entity);
        UUID sceneID = m_Scene->GetUUID();

        if (comp.ScriptID)
        {
            auto& entry = PythonScriptEngine::GetEntityScriptStorage(*m_PythonScriptStorage, comp.ScriptID);
            /*if (entry.Instance && entry.Instance->IsValid())
                entry.Instance->Invoke<void>("OnDestroy");*/
            PythonScriptEngine::RemoveScriptObject(*m_PythonScriptStorage, comp.ScriptID);
            comp.ScriptID = 0;
        }
    }

}

