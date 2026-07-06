#include "prpch.h"
#include "ScriptSystem.h"
#include "../Scene.h"
#include "../Entity.h"
#include "../Components.h"

#include <Rolky/GC.hpp>

#include "Scripting/CSharp/CSharpScriptStorage.h"
#include "Scripting/Python/PythonScriptStorage.h"
#include "Scripting/CSharp/CSharpScriptEngine.h"
#include "Scripting/Python/PythonScriptEngine.h"
#include "Scripting/CSharp/CSharpScriptMetaRegistry.h"
#include "Scripting/Python/PythonScriptMetaRegistry.h"

namespace Prism {

    template<typename... Args>
    static void PyCallMethod(pybind11::object* obj, const char* method, Args&&... args)
    {
        if (!obj) return;
        try { obj->attr(method)(std::forward<Args>(args)...); }
        catch (pybind11::error_already_set& e) {
            PR_CORE_ERROR("[Python] {0}: {1}", method, e.what());
        }
    }

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


        m_CSharpPreUnloadToken  = CSharpScriptEngine::RegisterPreUnloadCallback([this]() { OnCSharpPreUnload(); });
        m_CSharpPostReloadToken = CSharpScriptEngine::RegisterPostReloadCallback([this]() { OnCSharpPostReload(); });

        m_PythonPreUnloadToken  = PythonScriptEngine::RegisterPreUnloadCallback([this]() { OnPythonPreUnload(); });
        m_PythonPostReloadToken = PythonScriptEngine::RegisterPostReloadCallback([this]() { OnPythonPostReload(); });
    }

    ScriptSystem::~ScriptSystem()
    {
        CSharpScriptEngine::UnregisterPreUnloadCallback(m_CSharpPreUnloadToken);
        CSharpScriptEngine::UnregisterPostReloadCallback(m_CSharpPostReloadToken);
        PythonScriptEngine::UnregisterPreUnloadCallback(m_PythonPreUnloadToken);
        PythonScriptEngine::UnregisterPostReloadCallback(m_PythonPostReloadToken);

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

        UUID currentSceneID = m_Scene->GetUUID();
        CSharpScriptEngine::s_ManagedObjects.erase(currentSceneID);
        PythonScriptEngine::s_PythonScriptObjects.erase(currentSceneID);
        Rolky::GC::Collect();
    }

    void ScriptSystem::OnUpdate(float dt)
    {
        PR_PROFILE_FUNCTION();
        CSharpScriptEngine::SetSceneContext(m_Scene);
        PythonScriptEngine::SetSceneContext(m_Scene);

        {
            auto view = m_Scene->GetAllEntitiesWith<CSharpScriptComponent>();
            UUID sceneID = m_Scene->GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Scene->GetRegistry().get<CSharpScriptComponent>(entity);
                for (auto& [bid, binding] : comp.Behaviours)
                {
                    if (!binding.Enabled) continue;
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnUpdate)) continue;
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid())
                        obj->InvokeMethod("OnUpdate");
                }
            }
        }

        {
            auto view = m_Scene->GetAllEntitiesWith<PythonScriptComponent>();
            UUID sceneID = m_Scene->GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Scene->GetRegistry().get<PythonScriptComponent>(entity);
                for (auto& [bid, binding] : comp.Behaviours)
                {
                    if (!binding.Enabled) continue;
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnUpdate)) continue;
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (obj)
                        PyCallMethod(obj, "OnUpdate");
                }
            }
        }
    }

    void ScriptSystem::OnLateUpdate(float dt)
    {
        PR_PROFILE_FUNCTION();
        CSharpScriptEngine::SetSceneContext(m_Scene);
        PythonScriptEngine::SetSceneContext(m_Scene);

        {
            auto view = m_Scene->GetAllEntitiesWith<CSharpScriptComponent>();
            UUID sceneID = m_Scene->GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Scene->GetRegistry().get<CSharpScriptComponent>(entity);
                for (auto& [bid, binding] : comp.Behaviours)
                {
                    if (!binding.Enabled) continue;
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::LateUpdate)) continue;
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid())
                        obj->InvokeMethod("LateUpdate");
                }
            }
        }

        {
            auto view = m_Scene->GetAllEntitiesWith<PythonScriptComponent>();
            UUID sceneID = m_Scene->GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Scene->GetRegistry().get<PythonScriptComponent>(entity);
                for (auto& [bid, binding] : comp.Behaviours)
                {
                    if (!binding.Enabled) continue;
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::LateUpdate)) continue;
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (obj)
                        PyCallMethod(obj, "LateUpdate");
                }
            }
        }
    }

    void ScriptSystem::OnFixedUpdate(float dt)
    {
        PR_PROFILE_FUNCTION();
        CSharpScriptEngine::SetSceneContext(m_Scene);
        PythonScriptEngine::SetSceneContext(m_Scene);
        // C# Script OnFixedUpdate
        {
            auto view = m_Scene->GetAllEntitiesWith<CSharpScriptComponent>();
            UUID sceneID = m_Scene->GetUUID();
            for (auto entity : view)
            {
                auto& comp = m_Scene->GetRegistry().get<CSharpScriptComponent>(entity);
                for (auto& [bid, binding] : comp.Behaviours)
                {
                    if (!binding.Enabled)
                        continue;
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnFixedUpdate))
                        continue;
                    auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                    if (obj && obj->IsValid())
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
                for (auto& [bid, binding] : comp.Behaviours)
                {
                    if (!binding.Enabled)
                        continue;
                    if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnFixedUpdate))
                        continue;
                    auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                    if (obj)
                        PyCallMethod(obj, "OnFixedUpdate");
                }
            }
        }
    }

    void ScriptSystem::OnRuntimeStart()
    {
        m_IsPlaying = true;
        CSharpScriptEngine::SetSceneContext(m_Scene);
        PythonScriptEngine::SetSceneContext(m_Scene);

        // C#: Instantiate each behaviour
        {
            auto view = m_Scene->GetRegistry().view<CSharpScriptComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, m_Scene };
                auto& comp = m_Scene->GetRegistry().get<CSharpScriptComponent>(entity);
                for (auto& [bid, binding] : comp.Behaviours)
                    InstantiateCSharpBehaviour(e, binding);
            }
        }

        // Python: Instantiate each behaviour
        {
            auto view = m_Scene->GetRegistry().view<PythonScriptComponent>();
            for (auto entity : view)
            {
                Entity e = { entity, m_Scene };
                auto& comp = m_Scene->GetRegistry().get<PythonScriptComponent>(entity);
                for (auto& [bid, binding] : comp.Behaviours)
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
                for (auto& [bid, binding] : comp.Behaviours)
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
                for (auto& [bid, binding] : comp.Behaviours)
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
            for (auto& [bid, binding] : comp.Behaviours)
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
            for (auto& [bid, binding] : comp.Behaviours)
            {
                if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnCollisionBegin))
                    continue;
                auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                if (obj)
                    PyCallMethod(obj, "OnCollisionBegin", 0.0f);
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
            for (auto& [bid, binding] : comp.Behaviours)
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
            for (auto& [bid, binding] : comp.Behaviours)
            {
                if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnCollisionEnd))
                    continue;
                auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                if (obj)
                    PyCallMethod(obj, "OnCollisionEnd", 0.0f);
            }
        }
    }

    void ScriptSystem::OnTriggerBegin(Entity entity)
    {
        CSharpScriptEngine::SetSceneContext(m_Scene);
        PythonScriptEngine::SetSceneContext(m_Scene);

        UUID sceneID = m_Scene->GetUUID();

        if (entity.HasComponent<CSharpScriptComponent>())
        {
            auto& comp = entity.GetComponent<CSharpScriptComponent>();
            for (auto& [bid, binding] : comp.Behaviours)
            {
                if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnTriggerBegin))
                    continue;
                auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                if (obj && obj->IsValid())
                    obj->TryInvokeMethod("OnTriggerBegin", 0.0f);
            }
        }

        if (entity.HasComponent<PythonScriptComponent>())
        {
            auto& comp = entity.GetComponent<PythonScriptComponent>();
            for (auto& [bid, binding] : comp.Behaviours)
            {
                if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnTriggerBegin))
                    continue;
                auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                if (obj)
                    PyCallMethod(obj, "OnTriggerBegin", 0.0f);
            }
        }
    }

    void ScriptSystem::OnTriggerEnd(Entity entity)
    {
        CSharpScriptEngine::SetSceneContext(m_Scene);
        PythonScriptEngine::SetSceneContext(m_Scene);

        UUID sceneID = m_Scene->GetUUID();

        if (entity.HasComponent<CSharpScriptComponent>())
        {
            auto& comp = entity.GetComponent<CSharpScriptComponent>();
            for (auto& [bid, binding] : comp.Behaviours)
            {
                if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnTriggerEnd))
                    continue;
                auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
                if (obj && obj->IsValid())
                    obj->TryInvokeMethod("OnTriggerEnd", 0.0f);
            }
        }

        if (entity.HasComponent<PythonScriptComponent>())
        {
            auto& comp = entity.GetComponent<PythonScriptComponent>();
            for (auto& [bid, binding] : comp.Behaviours)
            {
                if (!(binding.LifecycleMask & (uint16_t)LifecycleMethod::OnTriggerEnd))
                    continue;
                auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
                if (obj)
                    PyCallMethod(obj, "OnTriggerEnd", 0.0f);
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
        auto bid = binding.BehaviourID;
        auto& comp = entity.GetComponent<CSharpScriptComponent>();
        auto [it, _] = comp.Behaviours.emplace(bid, std::move(binding));
        m_CSharpBindingMap[bid] = &it->second;
    }

    void ScriptSystem::RegisterPythonBinding(Entity entity, PythonBehaviourBinding&& binding)
    {
        auto bid = binding.BehaviourID;
        auto& comp = entity.GetComponent<PythonScriptComponent>();
        auto [it, _] = comp.Behaviours.emplace(bid, std::move(binding));
        m_PythonBindingMap[bid] = &it->second;
    }

    UUID ScriptSystem::AddCSharpBehaviour(Entity entity, UUID classID)
    {
        auto binding = CreateCSharpBinding(classID);
        UUID behaviourID = binding.BehaviourID;
        RegisterCSharpBinding(entity, std::move(binding));

        if (m_IsPlaying)
        {
            auto& comp = entity.GetComponent<CSharpScriptComponent>();
            InstantiateCSharpBehaviour(entity, comp.Behaviours.at(behaviourID));
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
            InstantiatePythonBehaviour(entity, comp.Behaviours.at(behaviourID));
        }
        return behaviourID;
    }

    void ScriptSystem::RemoveCSharpBehaviour(Entity entity, UUID behaviourID)
    {
        auto& comp = entity.GetComponent<CSharpScriptComponent>();
        auto it = comp.Behaviours.find(behaviourID);
        if (it == comp.Behaviours.end())
            return;

        if (m_IsPlaying)
        {
            CSharpScriptEngine::SetSceneContext(m_Scene);
            UUID sceneID = m_Scene->GetUUID();
            auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, behaviourID);
            auto& binding = it->second;
            if (obj && obj->IsValid())
            {
                if ((binding.LifecycleMask & (uint16_t)LifecycleMethod::OnDisable) && binding.Enabled)
                    obj->InvokeMethod("OnDisable");
                if (binding.LifecycleMask & (uint16_t)LifecycleMethod::OnDestroy)
                    obj->InvokeMethod("OnDestroy");
            }
            for (auto& [hash, field] : binding.Fields)
                field.ClearInstance();
            CSharpScriptEngine::s_ManagedObjects[sceneID].erase(behaviourID);
        }

        m_CSharpBindingMap.erase(behaviourID);
        comp.Behaviours.erase(it);
    }

    void ScriptSystem::RemovePythonBehaviour(Entity entity, UUID behaviourID)
    {
        auto& comp = entity.GetComponent<PythonScriptComponent>();
        auto it = comp.Behaviours.find(behaviourID);
        if (it == comp.Behaviours.end())
            return;

        if (m_IsPlaying)
        {
            PythonScriptEngine::SetSceneContext(m_Scene);
            UUID sceneID = m_Scene->GetUUID();
            auto* obj = PythonScriptEngine::GetScriptObject(sceneID, behaviourID);
            auto& binding = it->second;
            if (obj)
            {
                if ((binding.LifecycleMask & (uint16_t)LifecycleMethod::OnDisable) && binding.Enabled)
                    PyCallMethod(obj, "OnDisable");
                if (binding.LifecycleMask & (uint16_t)LifecycleMethod::OnDestroy)
                    PyCallMethod(obj, "OnDestroy");
            }
            for (auto& [hash, field] : binding.Fields)
                field.ClearInstance();
            PythonScriptEngine::s_PythonScriptObjects[sceneID].erase(behaviourID);
        }

        m_PythonBindingMap.erase(behaviourID);
        comp.Behaviours.erase(it);
    }

    bool ScriptSystem::GetEnabled(UUID behaviourID)
    {
        auto it = m_CSharpBindingMap.find(behaviourID);
        if (it != m_CSharpBindingMap.end())
            return it->second->Enabled;

        auto pyIt = m_PythonBindingMap.find(behaviourID);
        if (pyIt != m_PythonBindingMap.end())
            return pyIt->second->Enabled;

        return true;
    }

    void ScriptSystem::SetEnabled(UUID behaviourID, bool enabled)
    {
        auto csIt = m_CSharpBindingMap.find(behaviourID);
        if (csIt != m_CSharpBindingMap.end())
        {
            auto* binding = csIt->second;
            if (binding->Enabled == enabled)
                return;
            binding->Enabled = enabled;

            if (m_IsPlaying)
            {
                UUID sceneID = m_Scene->GetUUID();
                auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, behaviourID);
                if (obj && obj->IsValid())
                {
                    if (!enabled && (binding->LifecycleMask & (uint16_t)LifecycleMethod::OnDisable))
                        obj->InvokeMethod("OnDisable");
                    else if (enabled && (binding->LifecycleMask & (uint16_t)LifecycleMethod::OnEnable))
                        obj->InvokeMethod("OnEnable");
                }
            }
            return;
        }

        auto pyIt = m_PythonBindingMap.find(behaviourID);
        if (pyIt != m_PythonBindingMap.end())
        {
            auto* binding = pyIt->second;
            if (binding->Enabled == enabled)
                return;
            binding->Enabled = enabled;

            if (m_IsPlaying)
            {
                UUID sceneID = m_Scene->GetUUID();
                auto* obj = PythonScriptEngine::GetScriptObject(sceneID, behaviourID);
                if (obj)
                {
                    if (!enabled && (binding->LifecycleMask & (uint16_t)LifecycleMethod::OnDisable))
                        PyCallMethod(obj, "OnDisable");
                    else if (enabled && (binding->LifecycleMask & (uint16_t)LifecycleMethod::OnEnable))
                        PyCallMethod(obj, "OnEnable");
                }
            }
        }
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

        obj->SetFieldValue("ID", (uint64_t)binding.BehaviourID);

        if (binding.LifecycleMask & (uint16_t)LifecycleMethod::Awake)
            obj->InvokeMethod("Awake");
        if (binding.LifecycleMask & (uint16_t)LifecycleMethod::OnCreate)
            obj->InvokeMethod("OnCreate");
        if ((binding.LifecycleMask & (uint16_t)LifecycleMethod::OnEnable) && binding.Enabled)
            obj->InvokeMethod("OnEnable");
    }

    void ScriptSystem::InstantiatePythonBehaviour(Entity entity, PythonBehaviourBinding& binding)
    {
        PythonScriptEngine::SetSceneContext(m_Scene);
        UUID sceneID = m_Scene->GetUUID();
        auto* obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
        if (!obj)
            PythonScriptEngine::AddBehaviour(entity, binding);

        obj = PythonScriptEngine::GetScriptObject(sceneID, binding.BehaviourID);
        if (!obj)
            return;

        try { obj->attr("ID") = pybind11::cast((uint64_t)binding.BehaviourID); }
        catch (pybind11::error_already_set& e) { PR_CORE_ERROR("[Python] SetField ID: {0}", e.what()); }

        if (binding.LifecycleMask & (uint16_t)LifecycleMethod::Awake)
            PyCallMethod(obj, "Awake");
        if (binding.LifecycleMask & (uint16_t)LifecycleMethod::OnCreate)
            PyCallMethod(obj, "OnCreate");
        if ((binding.LifecycleMask & (uint16_t)LifecycleMethod::OnEnable) && binding.Enabled)
            PyCallMethod(obj, "OnEnable");
    }

    void ScriptSystem::DestroyCSharpBehaviour(Entity entity, CSharpBehaviourBinding& binding)
    {
        CSharpScriptEngine::SetSceneContext(m_Scene);
        UUID sceneID = m_Scene->GetUUID();
        auto* obj = CSharpScriptEngine::GetManagedObject(sceneID, binding.BehaviourID);
        if (obj && obj->IsValid())
        {
            if ((binding.LifecycleMask & (uint16_t)LifecycleMethod::OnDisable) && binding.Enabled)
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
        if (obj)
        {
            if ((binding.LifecycleMask & (uint16_t)LifecycleMethod::OnDisable) && binding.Enabled)
                PyCallMethod(obj, "OnDisable");
            if (binding.LifecycleMask & (uint16_t)LifecycleMethod::OnDestroy)
                PyCallMethod(obj, "OnDestroy");
        }
        for (auto& [hash, field] : binding.Fields)
            field.ClearInstance();
        PythonScriptEngine::s_PythonScriptObjects[sceneID].erase(binding.BehaviourID);
    }

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

        auto oldBehaviours = std::move(comp.Behaviours);
        for (auto& [oldBid, oldBinding] : oldBehaviours)
        {
            auto* classMeta = CSharpScriptMetaRegistry::GetClassMetadata(oldBinding.ClassID);
            if (!classMeta)
            {
                PR_CORE_WARN("[ScriptSystem] Class (ID={0}) removed, dropping behaviour", (uint64_t)oldBinding.ClassID);
                for (auto& [hash, field] : oldBinding.Fields)
                    if (field.GetBuffer().Data)
                        const_cast<Buffer&>(field.GetBuffer()).Free();
                continue;
            }
            auto newBinding = CreateCSharpBinding(oldBinding.ClassID);
            newBinding.Enabled = oldBinding.Enabled;
            for (auto& [hash, newField] : newBinding.Fields)
            {
                auto oldIt = oldBinding.Fields.find(hash);
                if (oldIt != oldBinding.Fields.end() &&
                    oldIt->second.GetType() == newField.GetType() &&
                    oldIt->second.GetBuffer().Data)
                {
                    newField.SetBuffer(oldIt->second.GetBuffer());
                }
            }
            for (auto& [hash, oldField] : oldBinding.Fields)
            {
                if (!newBinding.Fields.contains(hash) && oldField.GetBuffer().Data)
                    const_cast<Buffer&>(oldField.GetBuffer()).Free();
            }

            UUID newBid = {};
            comp.Behaviours[newBid] = std::move(newBinding);
            comp.Behaviours[newBid].BehaviourID = newBid;
            m_CSharpBindingMap[newBid] = &comp.Behaviours[newBid];
        }
    }

    void ScriptSystem::OnCSharpScriptComponentDestroy(entt::registry& registry, entt::entity entity)
    {
        CSharpScriptEngine::SetSceneContext(m_Scene);
        auto& comp = registry.get<CSharpScriptComponent>(entity);
        UUID sceneID = m_Scene->GetUUID();

        for (auto& [bid, binding] : comp.Behaviours)
            m_CSharpBindingMap.erase(bid);

        if (comp.ScriptID)
        {
            auto& entry = CSharpScriptEngine::GetEntityScriptStorage(*m_CSharpScriptStorage, comp.ScriptID);
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


        auto oldBehaviours = std::move(comp.Behaviours);

        for (auto& [oldBid, oldBinding] : oldBehaviours)
        {
            auto* classMeta = PythonScriptMetaRegistry::GetClassMetadata(oldBinding.ClassID);
            if (!classMeta)
            {
                PR_CORE_WARN("[ScriptSystem] Python class (ID={0}) removed, dropping behaviour", (uint64_t)oldBinding.ClassID);
                for (auto& [hash, field] : oldBinding.Fields)
                    if (field.GetBuffer().Data)
                        const_cast<Buffer&>(field.GetBuffer()).Free();
                continue;
            }

            auto newBinding = CreatePythonBinding(oldBinding.ClassID);
            newBinding.Enabled = oldBinding.Enabled;

            for (auto& [hash, newField] : newBinding.Fields)
            {
                auto oldIt = oldBinding.Fields.find(hash);
                if (oldIt != oldBinding.Fields.end() &&
                    oldIt->second.GetType() == newField.GetType() &&
                    oldIt->second.GetBuffer().Data)
                {
                    newField.SetBuffer(oldIt->second.GetBuffer());
                }
            }

            for (auto& [hash, oldField] : oldBinding.Fields)
            {
                if (!newBinding.Fields.contains(hash) && oldField.GetBuffer().Data)
                    const_cast<Buffer&>(oldField.GetBuffer()).Free();
            }

            UUID newBid = {};
            comp.Behaviours[newBid] = std::move(newBinding);
            comp.Behaviours[newBid].BehaviourID = newBid;
            m_PythonBindingMap[newBid] = &comp.Behaviours[newBid];
        }
    }

    void ScriptSystem::OnPythonScriptComponentDestroy(entt::registry& registry, entt::entity entity)
    {
        PythonScriptEngine::SetSceneContext(m_Scene);
        auto& comp = registry.get<PythonScriptComponent>(entity);
        UUID sceneID = m_Scene->GetUUID();

        for (auto& [bid, binding] : comp.Behaviours)
            m_PythonBindingMap.erase(bid);

        if (comp.ScriptID)
        {
            auto& entry = PythonScriptEngine::GetEntityScriptStorage(*m_PythonScriptStorage, comp.ScriptID);
            PythonScriptEngine::RemoveScriptObject(*m_PythonScriptStorage, comp.ScriptID);
            comp.ScriptID = 0;
        }
    }

    void ScriptSystem::OnCSharpPreUnload()
    {
        auto& registry = m_Scene->GetRegistry();
        auto view = registry.view<CSharpScriptComponent>();
        for (auto e : view)
        {
            Entity entity = { e, m_Scene };
            m_SavedCSharpComponents[entity.GetUUID()] = std::move(view.get<CSharpScriptComponent>(e));
        }
        registry.clear<CSharpScriptComponent>();
        m_CSharpBindingMap.clear();
    }

    void ScriptSystem::OnCSharpPostReload()
    {
        CSharpScriptEngine::SetSceneContext(m_Scene);

        auto& registry = m_Scene->GetRegistry();
        for (auto& [entityUUID, savedComp] : m_SavedCSharpComponents)
        {
            Entity entity = m_Scene->FindEntityByUUID(entityUUID);
            if (!entity) continue;
            registry.emplace<CSharpScriptComponent>((entt::entity)entity, std::move(savedComp));
        }
        m_SavedCSharpComponents.clear();
    }


    void ScriptSystem::OnPythonPreUnload()
    {
        auto& registry = m_Scene->GetRegistry();
        auto view = registry.view<PythonScriptComponent>();

        for (auto e : view)
        {
            Entity entity = { e, m_Scene };
            m_SavedPythonComponents[entity.GetUUID()] = std::move(view.get<PythonScriptComponent>(e));
        }

        registry.clear<PythonScriptComponent>();
        m_PythonBindingMap.clear();
    }


    void ScriptSystem::OnPythonPostReload()
    {
        PythonScriptEngine::SetSceneContext(m_Scene);

        auto& registry = m_Scene->GetRegistry();
        for (auto& [entityUUID, savedComp] : m_SavedPythonComponents)
        {
            Entity entity = m_Scene->FindEntityByUUID(entityUUID);
            if (!entity) continue;
            registry.emplace<PythonScriptComponent>((entt::entity)entity, std::move(savedComp));
        }
        m_SavedPythonComponents.clear();
    }

}
