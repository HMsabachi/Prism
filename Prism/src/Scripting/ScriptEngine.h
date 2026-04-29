#pragma once
#include "Prism/Scene/Components.h"
#include "Prism/Scene/Entity.h"

namespace Rolky
{
	struct HostInstance;
	struct AssemblyLoadContext;
	struct ManagedAssembly;
}

namespace Prism
{
	class PRISM_API ScriptEngine
	{
	public:
		static bool Initialize();

		static void Shutdown();

		static bool LoadEngineAssembly(const std::string& assemblyPath);
		static bool LoadAppAssembly(const std::string& assemblyPath);

		static void RegisterEngineFunctions();

		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(uint32_t entityID, float ts);

		static void OnInitEntity(ScriptComponent& script, uint32_t entityID, uint32_t sceneID);

	public:
		static Rolky::ManagedAssembly& GetEngineAssembly();
	private:
		static std::unique_ptr<Rolky::HostInstance> m_Host;
		static std::unique_ptr<Rolky::AssemblyLoadContext> m_LoadContext;

		static void* s_HostHandle;
		static void* s_AssemblyLoadContext;   
	};
}