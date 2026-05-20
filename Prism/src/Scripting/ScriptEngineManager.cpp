#include "prpch.h"
#include "ScriptEngineManager.h"

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

	void ScriptEngineManager::OnScriptAdded(Entity entity, const ScriptInstance& script)
	{
		auto* engine = Get(script.Language);
		if (engine)
			engine->InitScriptEntity(entity, script.ModuleName);
	}

	void ScriptEngineManager::OnScriptRemoved(Entity entity, const ScriptInstance& script)
	{
		auto* engine = Get(script.Language);
		if (engine)
			engine->ShutdownScriptEntity(entity, script.ModuleName);
	}

	void ScriptEngineManager::SetSceneContext(const Ref<Scene>& scene)
	{
		for (auto& [lang, engine] : s_Engines)
		{
			if (engine)
				engine->SetSceneContext(scene);
		}
	}

	void ScriptEngineManager::OnSceneDestruct(UUID sceneID)
	{
		for (auto& [lang, engine] : s_Engines)
		{
			if (engine)
				engine->OnSceneDestruct(sceneID);
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

	void ScriptEngineManager::OnCollision2DBegin(Entity entity)
	{
		for (auto& [lang, engine] : s_Engines)
		{
			if (engine)
				engine->OnCollision2DBegin(entity);
		}
	}

	void ScriptEngineManager::OnCollision2DEnd(Entity entity)
	{
		for (auto& [lang, engine] : s_Engines)
		{
			if (engine)
				engine->OnCollision2DEnd(entity);
		}
	}

	void ScriptEngineManager::OnCollisionBegin(Entity entity)
	{
		for (auto& [lang, engine] : s_Engines)
		{
			if (engine)
				engine->OnCollisionBegin(entity);
		}
	}

	void ScriptEngineManager::OnCollisionEnd(Entity entity)
	{
		for (auto& [lang, engine] : s_Engines)
		{
			if (engine)
				engine->OnCollisionEnd(entity);
		}
	}
}
