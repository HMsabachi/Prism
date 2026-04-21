#pragma once

namespace Prism::Scripting
{
	class PRISM_API ScriptingHost
	{
	public:
		static bool Initialize();

		static void Shutdown();

		static bool LoadAssembly(const std::string& assemblyPath);

		static void OnUpdate(float deltaTime);
		static void OnFixedUpdate(float fixedDeltaTime);


		static void RegisterEngineFunctions();

	private:

		static void* s_HostHandle;
		static void* s_AssemblyLoadContext;   
	};
}