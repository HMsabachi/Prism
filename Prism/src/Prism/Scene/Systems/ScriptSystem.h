#pragma once

#include "ISystem.h"
#include "Prism/Scene/Entity.h"
#include <entt/entt.hpp>

namespace Prism {

    struct CSharpScriptStorage;
    struct PythonScriptStorage;
    class Scene;

    class PRISM_API ScriptSystem : public ISystem {
    public:
        explicit ScriptSystem(Scene* scene);
        ~ScriptSystem() override;

        void OnUpdate(float ts) override;
        void OnFixedUpdate(float ts) override;
        void OnRuntimeStart() override;
        void OnRuntimeStop() override;

        void OnCollisionBegin(Entity entity);
        void OnCollisionEnd(Entity entity);

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

        Scene* m_Scene;
        CSharpScriptStorage* m_CSharpScriptStorage = nullptr;
        PythonScriptStorage* m_PythonScriptStorage = nullptr;
        bool m_IsPlaying = false;

        std::unordered_map<UUID, CSharpBehaviourBinding*> m_CSharpBindingMap;
        std::unordered_map<UUID, PythonBehaviourBinding*> m_PythonBindingMap;
    };

}
