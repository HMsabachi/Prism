#include "prpch.h"
#include "ScriptEngineManager.h"
#include "Scripting/ScriptStorage.h"
#include "Prism/Scene/Components.h"
#include "Prism/Scene/Entity.h"

namespace Prism
{
    std::unordered_map<ScriptLanguage, std::unique_ptr<ScriptEngine>> ScriptEngineManager::s_Engines;

    void ScriptEngineManager::Init()
    {
        s_Engines.clear();
    }

    void ScriptEngineManager::Shutdown()
    {
        for (auto& [lang, engine] : s_Engines)
        {
            if (engine)
            {
                engine->Shutdown();
                engine.reset();
            }
        }
        s_Engines.clear();
    }

    ScriptEngine* ScriptEngineManager::Get(ScriptLanguage lang)
    {
        auto it = s_Engines.find(lang);
        return it != s_Engines.end() ? it->second.get() : nullptr;
    }

    void ScriptEngineManager::Register(ScriptLanguage lang, std::unique_ptr<ScriptEngine> engine)
    {
        s_Engines[lang] = std::move(engine);
    }

    bool ScriptEngineManager::IsRegistered(ScriptLanguage lang)
    {
        return s_Engines.find(lang) != s_Engines.end();
    }

    void ScriptEngineManager::OnScriptAdded(Entity entity, const ScriptInstance& script, SceneScriptStorage& storage)
    {
        auto* engine = Get(script.Language);
        if (!engine)
            return;

        auto& entityStorage = storage.GetOrCreateEntity(entity.GetUUID());
        auto& group = entityStorage.Groups[script.ModuleName];
        group.EntityID = entity.GetUUID();
        group.ModuleName = script.ModuleName;
    


        engine->InitScriptEntity(entity, group);
    }

    void ScriptEngineManager::OnScriptRemoved(Entity entity, const ScriptInstance& script, SceneScriptStorage& storage)
    {
        auto* engine = Get(script.Language);
        if (!engine)
            return;

        auto* group = storage.FindGroup(entity.GetUUID(), script.ModuleName);
        if (!group)
            return;

        engine->ShutdownScriptEntity(*group);
        storage.RemoveGroup(entity.GetUUID(), script.ModuleName);
    }

    void ScriptEngineManager::SetSceneContext(const Ref<Scene>& scene)
    {
        for (auto& [lang, engine] : s_Engines)
        {
            if (engine)
                engine->SetSceneContext(scene);
        }
    }

    void ScriptEngineManager::ReloadAssembly(const std::string& path)
    {
        for (auto& [lang, engine] : s_Engines)
        {
            if (engine)
                engine->ReloadAssembly(path);
        }
    }

    void ScriptEngineManager::OnImGuiRender()
    {
        for (auto& [lang, engine] : s_Engines)
        {
            if (engine)
                engine->OnImGuiRender();
        }
    }

}
