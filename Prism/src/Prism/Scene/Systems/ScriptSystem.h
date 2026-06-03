#pragma once

#include "ISystem.h"
#include "Prism/Scene/Entity.h"
#include "Scripting/CSharp/CSharpScriptEngine.h"
#include "Scripting/Python/PythonScriptEngine.h"
#include <entt/entt.hpp>

namespace Prism {

    struct CSharpScriptStorage;
    struct PythonScriptStorage;
    class Scene;

    class PRISM_API ScriptSystem : public ISystem {
    public:
        explicit ScriptSystem(Scene* scene);
        ~ScriptSystem() override;

        void OnFixedUpdate(float dt) override;
        void OnUpdate(float dt) override;
        void OnLateUpdate(float dt) override;
        void OnRuntimeStart() override;
        void OnRuntimeStop() override;

        void OnCollisionBegin(Entity entity);
        void OnCollisionEnd(Entity entity);
        void OnTriggerBegin(Entity entity);
        void OnTriggerEnd(Entity entity);

        CSharpBehaviourBinding CreateCSharpBinding(UUID classID);
        PythonBehaviourBinding CreatePythonBinding(UUID classID);

        void RegisterCSharpBinding(Entity entity, CSharpBehaviourBinding&& binding);
        void RegisterPythonBinding(Entity entity, PythonBehaviourBinding&& binding);

        UUID AddCSharpBehaviour(Entity entity, UUID classID);
        UUID AddPythonBehaviour(Entity entity, UUID classID);

        void RemoveCSharpBehaviour(Entity entity, UUID behaviourID);
        void RemovePythonBehaviour(Entity entity, UUID behaviourID);

        bool GetEnabled(UUID behaviourID);
        void SetEnabled(UUID behaviourID, bool enabled);

    private:
        void OnCSharpScriptComponentConstruct(entt::registry& registry, entt::entity entity);
        void OnCSharpScriptComponentDestroy(entt::registry& registry, entt::entity entity);
        void OnPythonScriptComponentConstruct(entt::registry& registry, entt::entity entity);
        void OnPythonScriptComponentDestroy(entt::registry& registry, entt::entity entity);

        void InstantiateCSharpBehaviour(Entity entity, CSharpBehaviourBinding& binding);
        void InstantiatePythonBehaviour(Entity entity, PythonBehaviourBinding& binding);

        void DestroyCSharpBehaviour(Entity entity, CSharpBehaviourBinding& binding);
        void DestroyPythonBehaviour(Entity entity, PythonBehaviourBinding& binding);

        // ── 热重载回调 ──
        void OnCSharpPreUnload();
        void OnCSharpPostReload();
        void OnPythonPreUnload();
        void OnPythonPostReload();

        Scene* m_Scene;
        CSharpScriptStorage* m_CSharpScriptStorage = nullptr;
        PythonScriptStorage* m_PythonScriptStorage = nullptr;
        bool m_IsPlaying = false;

        std::unordered_map<UUID, CSharpBehaviourBinding*> m_CSharpBindingMap;
        std::unordered_map<UUID, PythonBehaviourBinding*> m_PythonBindingMap;

        // ── 热重载状态 ──
        CSharpScriptEngine::ReloadCallbackToken m_CSharpPreUnloadToken = 0;
        CSharpScriptEngine::ReloadCallbackToken m_CSharpPostReloadToken = 0;
        PythonScriptEngine::ReloadCallbackToken m_PythonPreUnloadToken = 0;
        PythonScriptEngine::ReloadCallbackToken m_PythonPostReloadToken = 0;
        std::unordered_map<UUID, CSharpScriptComponent> m_SavedCSharpComponents;
        std::unordered_map<UUID, PythonScriptComponent> m_SavedPythonComponents;
    };

}
