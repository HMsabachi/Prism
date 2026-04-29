#include "prpch.h"
#include "ScriptEngine.h"
#include "ScriptEngineRegistry.h"
#include "Native/String.h"
#include <filesystem>

#include <Rolky/HostInstance.hpp>

namespace Prism
{
	static void RolkyMessageCallback(std::string_view message, Rolky::MessageLevel level)
	{
		if (level & Rolky::MessageLevel::Error) PR_CORE_ERROR("[Rolky] {0}", message);
		else if (level & Rolky::MessageLevel::Warning) PR_CORE_WARN("[Rolky] {0}", message);
		else if (level & Rolky::MessageLevel::Info) PR_CORE_INFO("[Rolky] {0}", message);
		else PR_CORE_TRACE("[Rolky] {0}", message);
	}
	static void RolkyExceptionCallback(std::string_view message)
	{
		PR_CORE_ERROR("[Rolky] {0}", message);
	}

	static std::unordered_map<std::string, Rolky::Type> s_EntityClassMap;
	static std::unordered_map<uint32_t, Rolky::ManagedObject> s_EntityInstanceMap;

	void* ScriptEngine::s_HostHandle = nullptr;
	void* ScriptEngine::s_AssemblyLoadContext = nullptr;

	// 委托
	static Rolky::Type s_ScriptEngineClass;

	std::unique_ptr<Rolky::HostInstance> ScriptEngine::m_Host;
	std::unique_ptr<Rolky::AssemblyLoadContext> ScriptEngine::m_LoadContext;
	static Rolky::ManagedAssembly s_EngineAssembly;
	static Rolky::ManagedAssembly s_AppAssembly;

	static void ResetFunctionPointers()
	{
		s_ScriptEngineClass = Rolky::Type();
	}
	void ScriptEngine::RegisterEngineFunctions()
	{
		s_ScriptEngineClass = s_EngineAssembly.GetType("Prism.Core");
	}
	bool ScriptEngine::Initialize()
	{
		PR_PROFILE_FUNCTION();

		Rolky::HostSettings setting;
		setting.RolkyDirectory = "Assets/scripts";
		setting.MessageCallback = RolkyMessageCallback;
		setting.ExceptionCallback = RolkyExceptionCallback;
		m_Host = std::make_unique<Rolky::HostInstance>();
		m_Host->Initialize(setting);
		m_LoadContext = std::make_unique<Rolky::AssemblyLoadContext>(std::move(m_Host->CreateAssemblyLoadContext("PrismLoadContext")));
		ResetFunctionPointers();
		return true;
	}
	void ScriptEngine::Shutdown()
	{
		for (auto& pair : s_EntityInstanceMap)
            pair.second.Destroy();
		
		m_Host->UnloadAssemblyLoadContext(*m_LoadContext);
		m_Host->Shutdown();
	}

	bool ScriptEngine::LoadEngineAssembly(const std::string& assemblyPath)
	{
		PR_PROFILE_FUNCTION();
		auto path = std::filesystem::absolute(assemblyPath).string();
		s_EngineAssembly = m_LoadContext->LoadAssembly(path);
		ScriptEngineRegistry::RegisterAll();
		RegisterEngineFunctions();
		return true;
	}

	bool ScriptEngine::LoadAppAssembly(const std::string& assemblyPath)
	{
		PR_PROFILE_FUNCTION();
		auto path = std::filesystem::absolute(assemblyPath).string();
		s_AppAssembly = m_LoadContext->LoadAssembly(path);
		return true;
	}
	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		PR_PROFILE_FUNCTION();
		uint32_t entityID = entity;
		Rolky::ManagedObject& entityInstance = s_EntityInstanceMap[entityID];
		if (entityInstance.IsValid())
			entityInstance.InvokeMethod("OnCreate");
	}

	void ScriptEngine::OnUpdateEntity(uint32_t entityID, float ts)
	{
		PR_PROFILE_FUNCTION();
		if (s_EntityInstanceMap.find(entityID) == s_EntityInstanceMap.end())
			return;
		Rolky::ManagedObject& entityInstance = s_EntityInstanceMap[entityID];
		if (entityInstance.IsValid())
			entityInstance.InvokeMethod("OnUpdate");

	}

	void ScriptEngine::OnInitEntity(ScriptComponent& script, uint32_t entityID, uint32_t sceneID)
	{
		PR_PROFILE_FUNCTION();
		std::string_view moduleName = script.ModuleName;
		Rolky::Type& scriptClass = s_EntityClassMap[script.ModuleName];
		scriptClass = s_AppAssembly.GetType(moduleName);
		if (!scriptClass)
		{
			PR_CORE_ERROR("Failed to load script class {0}", moduleName);
			return;
		}
		Rolky::ManagedObject& entityInstance = s_EntityInstanceMap[entityID];
		entityInstance = scriptClass.CreateInstance();
		entityInstance.SetPropertyValue("EntityID", entityID);
		entityInstance.SetPropertyValue("SceneID", sceneID);
	}
	
	Rolky::ManagedAssembly& ScriptEngine::GetEngineAssembly()
	{
		return s_EngineAssembly;
	}

}


/// typedef int (CORECLR_DELEGATE_CALLTYPE* load_assembly_and_get_function_pointer_fn)(
/// 	const char_t* assembly_path,      // 程序集磁盘路径
/// 	const char_t* type_name,          // 类型全称 (命名空间.类名, 程序集名)
/// 	const char_t* method_name,        // 静态方法名
/// 	const char_t* delegate_type_name, // 委托类型名 (或使用特殊常量)
/// 	void* reserved,           // 必须为 nullptr
/// 	void** result              // 输出：接收到的函数指针
/// 	);