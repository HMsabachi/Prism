#pragma once
#include "ScriptEngine.h"
#include <memory>
#include <unordered_map>

namespace Prism
{
	class SceneScriptStorage;
	struct ScriptInstance;

	class PRISM_API ScriptEngineManager
	{
	public:
		static void Init();
		static void Shutdown();

		// Engine registry
		static ScriptEngine* Get(ScriptLanguage lang);
		static void Register(ScriptLanguage lang, std::unique_ptr<ScriptEngine> engine);
		static bool IsRegistered(ScriptLanguage lang);

		// Script lifecycle — creates/removes ScriptGroup in SceneScriptStorage
		static void OnScriptAdded(Entity entity, const ScriptInstance& script, SceneScriptStorage& storage);
		static void OnScriptRemoved(Entity entity, const ScriptInstance& script, SceneScriptStorage& storage);

		// Scene-wide operations (apply to all registered engines)
		static void SetSceneContext(const Ref<Scene>& scene);
		static void ReloadAssembly(const std::string& path);
		static void OnImGuiRender();

	private:
		static std::unordered_map<ScriptLanguage, std::unique_ptr<ScriptEngine>> s_Engines;
	};
}
