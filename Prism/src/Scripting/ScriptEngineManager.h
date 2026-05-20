#pragma once
#include "ScriptEngine.h"
#include <memory>
#include <unordered_map>

namespace Prism
{
	class PRISM_API ScriptEngineManager
	{
	public:
		static void Init();
		static void Shutdown();

		// Engine registry
		static ScriptEngine* Get(ScriptLanguage lang);
		static void Register(ScriptLanguage lang, std::unique_ptr<ScriptEngine> engine);
		static bool IsRegistered(ScriptLanguage lang);

		static void OnScriptAdded(Entity entity, const ScriptInstance& script);
		static void OnScriptRemoved(Entity entity, const ScriptInstance& script);

		// Scene-wide operations (apply to all registered engines)
		static void SetSceneContext(const Ref<Scene>& scene);
		static void OnSceneDestruct(UUID sceneID);
		static void ReloadAssembly(const std::string& path);
		static void OnImGuiRender();

		// Collision dispatch (broadcast to all engines, each handles its own scripts internally)
		static void OnCollision2DBegin(Entity entity);
		static void OnCollision2DEnd(Entity entity);
		static void OnCollisionBegin(Entity entity);
		static void OnCollisionEnd(Entity entity);

	private:
		static std::unordered_map<ScriptLanguage, std::unique_ptr<ScriptEngine>> s_Engines;
	};
}
