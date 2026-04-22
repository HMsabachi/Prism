#pragma once
#include "Prism/Scene/Components.h"
#include "Prism/Scene/Entity.h"

namespace Prism
{
	class PRISM_API ScriptEngine
	{
	public:
		static bool Initialize();

		static void Shutdown();

		static bool LoadAssembly(const std::string& assemblyPath);
		static bool LoadFunction(const std::wstring& className, const std::wstring& funcName, void** func);

		static void RegisterEngineFunctions();

		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(uint32_t entityID, float ts);

		static void OnInitEntity(ScriptComponent& script, uint32_t entityID, uint32_t sceneID);

	private:

		static void* s_HostHandle;
		static void* s_AssemblyLoadContext;   
	};
}