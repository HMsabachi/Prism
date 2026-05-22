#pragma once
#include "Scripting/ScriptEngine.h"
#include "Scripting/PublicField.h"
#include "Scripting/Python/PythonScriptCore.h"
#include "Scripting/ScriptStorage.h"
#include <unordered_map>
#include <memory>

namespace Prism
{
    class PythonScriptEngine : public ScriptEngine
    {
    public:
        PythonScriptEngine() = default;
        ~PythonScriptEngine() override;

        // ScriptEngine interface
        bool Initialize() override;
        void Shutdown() override;
        bool LoadEngineAssembly(const std::string& path) override;
        bool LoadAppAssembly(const std::string& path) override;
        void ReloadAssembly(const std::string& path) override;
        void SetSceneContext(const Ref<Scene>& scene) override;
        const Ref<Scene>& GetCurrentSceneContext() override;
        void InitScriptEntity(Entity& entity, ScriptGroup& group) override;
        void ShutdownScriptEntity(ScriptGroup& group) override;
        void InstantiateEntityClass(ScriptGroup& group) override;
        bool ModuleExists(const std::string& moduleName) override;
        void OnImGuiRender() override;

    private:
        Ref<Scene> m_SceneContext;
        bool m_Initialized = false;
    };
}
