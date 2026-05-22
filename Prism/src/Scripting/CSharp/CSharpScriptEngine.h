#pragma once
#include "Scripting/ScriptEngine.h"
#include "Scripting/PublicField.h"
#include "Scripting/ScriptStorage.h"
#include <unordered_map>

namespace Rolky
{
	struct HostInstance;
	struct AssemblyLoadContext;
	struct ManagedAssembly;
	class ManagedObject;
	class Type;
}
#include <Rolky/HostInstance.hpp>

namespace Prism
{
	class CSharpScriptEngine : public ScriptEngine
	{
	public:
		CSharpScriptEngine();
		~CSharpScriptEngine() override;

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

		// C#-specific accessors
		Rolky::ManagedAssembly& GetEngineAssembly();

	private:
		std::unique_ptr<Rolky::HostInstance> m_Host;
		std::unique_ptr<Rolky::AssemblyLoadContext> m_LoadContext;

		Ref<Scene> m_SceneContext;

		std::unordered_map<std::string, Rolky::Type> m_EntityClassMap;
		Rolky::ManagedAssembly m_EngineAssembly;
		Rolky::ManagedAssembly m_AppAssembly;
		bool m_Initialized = false;
	};
}
