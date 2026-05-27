#include "prpch.h"
#include "ScriptSystem.h"
#include "../Scene.h"
#include "../Entity.h"
#include "../Components.h"

#include "Scripting/CSharp/CSharpScriptStorage.h"
#include "Scripting/Python/PythonScriptStorage.h"
#include "Scripting/CSharp/CSharpScriptEngine.h"
#include "Scripting/Python/PythonScriptEngine.h"
#include "Scripting/CSharp/CSharpScriptMetaRegistry.h"
#include "Scripting/Python/PythonScriptMetaRegistry.h"

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
        CSharpScriptEngine::SetSceneContext(m_Scene);
        PythonScriptEngine::SetSceneContext(m_Scene);
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
                    if (obj && obj->IsValid() && obj->GetField<bool>("Enabled"))
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
                    if (obj && obj->IsValid() && obj->GetField<bool>("Enabled"))
                        obj->Invoke<void>("LateUpdate");
                }
            }
        }
    }

    void ScriptSystem::OnFixedUpdate(float ts)
    {
        CSharpScriptEngine::SetSceneContext(m_Scene);
        PythonScriptEngine::SetSceneContext(m_Scene);
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
                    if (obj && obj->IsValid() && obj->GetField<bool>("Enabled"))
                        obj->Invoke<void>("OnFixedUpdate");
                }
            }
        }
    }

    void ScriptSystem::OnRuntimeStart()
    {
        m_IsPlaying = true;
        CSharpScriptEngine::SetSceneContext(m_Scene);
        PythonScriptEngine::SetSceneContext(m_Scene);

        // C#: Instantiate each behaviour (create + Awake + OnCreate + OnEnable)
        {
            auto view = m_Scene->GetRegistry().view<CSharpScriptComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, m_Scene };
                auto& comp = m_Scene->GetRegistry().get<CSharpScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                    InstantiateCSharpBehaviour(e, binding);
            }
        }

        // Python: Instantiate each behaviour (create + Awake + OnCreate + OnEnable)
        {
            auto view = m_Scene->GetRegistry().view<PythonScriptComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, m_Scene };
                auto& comp = m_Scene->GetRegistry().get<PythonScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                    InstantiatePythonBehaviour(e, binding);
            }
        }
    }

    void ScriptSystem::OnRuntimeStop()
    {
        m_IsPlaying = false;
        CSharpScriptEngine::SetSceneContext(m_Scene);
        PythonScriptEngine::SetSceneContext(m_Scene);

        {
            auto view = m_Scene->GetRegistry().view<CSharpScriptComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, m_Scene };
                auto& comp = m_Scene->GetRegistry().get<CSharpScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                    DestroyCSharpBehaviour(e, binding);
                comp.Behaviours.clear();
            }
        }

        {
            auto view = m_Scene->GetRegistry().view<PythonScriptComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, m_Scene };
                auto& comp = m_Scene->GetRegistry().get<PythonScriptComponent>(entity);
                for (auto& binding : comp.Behaviours)
                    DestroyPythonBehaviour(e, binding);
                comp.Behaviours.clear();
            }
        }
    }

    void ScriptSystem::OnCollisionBegin(Entity entity)
    {
        CSharpScriptEngine::SetSceneContext(m_Scene);
        PythonScriptEngine::SetSceneContext(m_Scene);

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
        CSharpScriptEngine::SetSceneContext(m_Scene);
        PythonScriptEngine::SetSceneContext(m_Scene);

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

    CSharpBehaviourBinding ScriptSystem::CreateCSharpBinding(UUID classID)
    {
        CSharpBehaviourBinding binding;
        binding.BehaviourID = UUID();
        binding.ClassID = classID;

        if (auto* meta = CSharpScriptMetaRegistry::GetClassMetadata(classID))
        {
            binding.LifecycleMask = meta->LifecycleMask;
            for (auto& [hash, fieldMeta] : meta->Fields)
            {
                CSharpField field(fieldMeta.Name, fieldMeta.Type);
                if (fieldMeta.DefaultValue.Data && fieldMeta.DefaultValue.Size > 0)
                    field.SetBuffer(fieldMeta.DefaultValue);
                binding.Fields[hash] = std::move(field);
            }
        }
        return binding;
    }

    PythonBehaviourBinding ScriptSystem::CreatePythonBinding(UUID classID)
    {
        PythonBehaviourBinding binding;
        binding.BehaviourID = UUID();
        binding.ClassID = classID;

        if (auto* meta = PythonScriptMetaRegistry::GetClassMetadata(classID))
        {
            binding.LifecycleMask = meta->LifecycleMask;
            for (auto& [hash, fieldMeta] : meta->Fields)
            {
                PythonField field(fieldMeta.Name, fieldMeta.Type);
                if (fieldMeta.DefaultValue.Data && fieldMeta.DefaultValue.Size > 0)
                    field.SetBuffer(fieldMeta.DefaultValue);
                binding.Fields[hash] = std::move(field);
            }
        }
        return binding;
    }

    void ScriptSystem::RegisterCSharpBinding(Entity entity, CSharpBehaviourBinding&& binding)
    {
        auto& comp = entity.GetComponent<CSharpScriptComponent>();
        comp.Behaviours.push_back(std::move(binding));
    }

    void ScriptSystem::RegisterPythonBinding(Entity entity, PythonBehaviourBinding&& binding)
    {
        auto& comp = entity.GetComponent<PythonScriptComponent>();
        comp.Behaviours.push_back(std::move(binding));
    }

    UUID ScriptSystem::AddCSharpBehaviour(Entity entity, UUID classID)
    {
        auto binding = CreateCSharpBinding(classID);
        UUID behaviourID = binding.BehaviourID;
        RegisterCSharpBinding(entity, std::move(binding));

        if (m_IsPlaying)
        {
            auto& comp = entity.GetComponent<CSharpScriptComponent>();
            InstantiateCSharpBehaviour(entity, comp.Behaviours.back());
        }
        return behaviourID;
    }

    UUID ScriptSystem::AddPythonBehaviour(Entity entity, UUID classID)
    {
        auto binding = CreatePythonBinding(classID);
        UUID behaviourID = binding.BehaviourID;
        RegisterPythonBinding(entity, std::move(binding));

        if (m_IsPlaying)
        {
            auto& comp = entity.GetComponent<PythonScriptComponent>();
            InstantiatePythonBehaviour(entity, comp.Behaviours.back());
        }
        return behaviourID;
    }

    void ScriptSystem::RemoveCSharpBehaviour(Entity entity, UUID behaviourID)
    {
        auto& comp = entity.GetComponent<CSharpScriptComponent>();
        auto it = std::find_if(comp.Behaviours.begin(), comp.Behaviours.end(),
            [behaviourID](const auto& b) { return b.BehaviourID == behaviourID; });
        if (it == comp.Behaviours.end())
            return;

        if (m_IsPlaying)
        {
            CSharpScriptEngine::SetSceneContext(m_Scene);
            UUID sceneID = m_Scene->GetUUID();
            auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, behaviourID);
            if (obj && obj->IsValid())
            {
                if ((it->LifecycleMask & (uint16_t)LifecycleMethod::OnDisable)
                    && obj->GetPropertyValue<Rolky::Bool32>("Enabled"))
                    obj->InvokeMethod("OnDisable");
                if (it->LifecycleMask & (uint16_t)LifecycleMethod::OnDestroy)
                    obj->InvokeMethod("OnDestroy");
            }
            for (auto& [hash, field] : it->Fields)
                field.ClearInstance();
            CSharpScriptEngine::s_ManagedObjects[sceneID].erase(behaviourID);
        }

        comp.Behaviours.erase(it);
    }

    void ScriptSystem::RemovePythonBehaviour(Entity entity, UUID behaviourID)
    {
        auto& comp = entity.GetComponent<PythonScriptComponent>();
        auto it = std::find_if(comp.Behaviours.begin(), comp.Behaviours.end(),
            [behaviourID](const auto& b) { return b.BehaviourID == behaviourID; });
        if (it == comp.Behaviours.end())
            return;

        if (m_IsPlaying)
        {
            PythonScriptEngine::SetSceneContext(m_Scene);
            UUID sceneID = m_Scene->GetUUID();
            auto* obj = PythonScriptEngine::GetScriptObject(sceneID, behaviourID);
            if (obj && obj->IsValid())
            {
                if ((it->LifecycleMask & (uint16_t)LifecycleMethod::OnDisable)
                    && obj->GetField<bool>("Enabled"))
                    obj->Invoke<void>("OnDisable");
                if (it->LifecycleMask & (uint16_t)LifecycleMethod::OnDestroy)
                    obj->Invoke<void>("OnDestroy");
            }
            for (auto& [hash, field] : it->Fields)
                field.ClearInstance();
            PythonScriptEngine::s_PythonScriptObjects[sceneID].erase(behaviourID);
        }

        comp.Behaviours.erase(it);
    }

    void ScriptSystem::InstantiateCSharpBehaviour(Entity entity, CSharpBehaviourBinding& binding)
    {
        CSharpScriptEngine::SetSceneContext(m_Scene);
        UUID sceneID = m_Scene->GetUUID();
        auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
        if (!obj || !obj->IsValid())
            CSharpScriptEngine::AddBehaviour(entity, binding);

        obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
        if (!obj || !obj->IsValid())
            return;

        if (binding.LifecycleMask & (uint16_t)LifecycleMethod::Awake)
            obj->InvokeMethod("Awake");
        if (binding.LifecycleMask & (uint16_t)LifecycleMethod::OnCreate)
            obj->InvokeMethod("OnCreate");
        if ((binding.LifecycleMask & (uint16_t)LifecycleMethod::OnEnable)
            && obj->GetPropertyValue<Rolky::Bool32>("Enabled"))
            obj->InvokeMethod("OnEnable");
    }

    void ScriptSystem::InstantiatePythonBehaviour(Entity entity, PythonBehaviourBinding& binding)
    {
        PythonScriptEngine::SetSceneContext(m_Scene);
        UUID sceneID = m_Scene->GetUUID();
        auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
        if (!obj || !obj->IsValid())
            PythonScriptEngine::AddBehaviour(entity, binding);

        obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
        if (!obj || !obj->IsValid())
            return;

        if (binding.LifecycleMask & (uint16_t)LifecycleMethod::Awake)
            obj->Invoke<void>("Awake");
        if (binding.LifecycleMask & (uint16_t)LifecycleMethod::OnCreate)
            obj->Invoke<void>("OnCreate");
        if ((binding.LifecycleMask & (uint16_t)LifecycleMethod::OnEnable)
            && obj->GetField<bool>("Enabled"))
            obj->Invoke<void>("OnEnable");
    }

    void ScriptSystem::DestroyCSharpBehaviour(Entity entity, CSharpBehaviourBinding& binding)
    {
        CSharpScriptEngine::SetSceneContext(m_Scene);
        UUID sceneID = m_Scene->GetUUID();
        auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
        if (obj && obj->IsValid())
        {
            if ((binding.LifecycleMask & (uint16_t)LifecycleMethod::OnDisable)
                && obj->GetPropertyValue<Rolky::Bool32>("Enabled"))
                obj->InvokeMethod("OnDisable");
            if (binding.LifecycleMask & (uint16_t)LifecycleMethod::OnDestroy)
                obj->InvokeMethod("OnDestroy");
        }
        for (auto& [hash, field] : binding.Fields)
            field.ClearInstance();
        CSharpScriptEngine::s_ManagedObjects[sceneID].erase(binding.BehaviourID);
    }

    void ScriptSystem::DestroyPythonBehaviour(Entity entity, PythonBehaviourBinding& binding)
    {
        PythonScriptEngine::SetSceneContext(m_Scene);
        UUID sceneID = m_Scene->GetUUID();
        auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
        if (obj && obj->IsValid())
        {
            if ((binding.LifecycleMask & (uint16_t)LifecycleMethod::OnDisable)
                && obj->GetField<bool>("Enabled"))
                obj->Invoke<void>("OnDisable");
            if (binding.LifecycleMask & (uint16_t)LifecycleMethod::OnDestroy)
                obj->Invoke<void>("OnDestroy");
        }
        for (auto& [hash, field] : binding.Fields)
            field.ClearInstance();
        PythonScriptEngine::s_PythonScriptObjects[sceneID].erase(binding.BehaviourID);
    }

    // Component lifecycle callbacks

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

