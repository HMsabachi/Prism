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

    private:
        void OnCSharpScriptComponentConstruct(entt::registry& registry, entt::entity entity);
        void OnCSharpScriptComponentDestroy(entt::registry& registry, entt::entity entity);
        void OnPythonScriptComponentConstruct(entt::registry& registry, entt::entity entity);
        void OnPythonScriptComponentDestroy(entt::registry& registry, entt::entity entity);

        Scene* m_Scene;
        CSharpScriptStorage* m_CSharpScriptStorage = nullptr;
        PythonScriptStorage* m_PythonScriptStorage = nullptr;
    };

}

