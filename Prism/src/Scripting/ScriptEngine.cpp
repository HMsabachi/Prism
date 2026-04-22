#include "prpch.h"
#include "ScriptEngine.h"
#include "ScriptEngineRegistry.h"
#include <nethost.h>
#include <hostfxr.h>
#include <coreclr_delegates.h>

namespace Prism
{
	// 全局函数指针
	static hostfxr_initialize_for_runtime_config_fn s_InitFn = nullptr;
	static hostfxr_get_runtime_delegate_fn s_GetDelegateFn = nullptr;
	static hostfxr_close_fn s_CloseFn = nullptr;

	static std::wstring s_AssemblyPath;
	static load_assembly_and_get_function_pointer_fn s_LoadFunction = nullptr;

	void* ScriptEngine::s_HostHandle = nullptr;
	void* ScriptEngine::s_AssemblyLoadContext = nullptr;

	bool ScriptEngine::Initialize()
	{
		PR_CORE_INFO("=== 开始初始化 .NET 9 脚本运行时 ===");
		char_t hostfxrPath[MAX_PATH];
		size_t bufferSize = MAX_PATH;
		if (get_hostfxr_path(hostfxrPath, &bufferSize, nullptr) != 0)
		{
			PR_CORE_ERROR("无法找到 hostfxr.dll！请确认已安装 .NET 9 SDK");
			return false;
		}
		HMODULE hostfxrLib = LoadLibraryW(hostfxrPath);
		if (!hostfxrLib)
		{
			PR_CORE_ERROR("LoadLibraryW hostfxr.dll 失败");
			return false;
		}
		s_InitFn = (hostfxr_initialize_for_runtime_config_fn)GetProcAddress(hostfxrLib, "hostfxr_initialize_for_runtime_config");
		s_GetDelegateFn = (hostfxr_get_runtime_delegate_fn)GetProcAddress(hostfxrLib, "hostfxr_get_runtime_delegate");
		s_CloseFn = (hostfxr_close_fn)GetProcAddress(hostfxrLib, "hostfxr_close");
		if (!s_InitFn || !s_GetDelegateFn || !s_CloseFn)
		{
			PR_CORE_ERROR("无法获取 hostfxr 函数指针");
			return false;
		}

		PR_CORE_INFO(".NET 9 hostfxr 初始化成功！");
		return true;
	}
	void ScriptEngine::Shutdown()
	{
		if (s_HostHandle && s_CloseFn)
		{
			s_CloseFn(s_HostHandle);
			s_HostHandle = nullptr;
		}
		PR_CORE_INFO(".NET 9 脚本运行时已关闭");
	}

	bool ScriptEngine::LoadAssembly(const std::string& assemblyPath)
	{
		PR_CORE_INFO("正在加载程序集: {0}", assemblyPath);
		std::wstring configPath = std::wstring(assemblyPath.begin(), assemblyPath.end());
		configPath = configPath.substr(0, configPath.find_last_of(L".")) + L".runtimeconfig.json";

		int rc = s_InitFn(configPath.c_str(), nullptr, &s_HostHandle);
		if (rc != 0 || s_HostHandle == nullptr)
		{
			PR_CORE_ERROR("hostfxr_initialize_for_runtime_config 失败: {0:x}", rc);
			s_CloseFn(s_HostHandle);
			return false;
		}

		s_LoadFunction = nullptr;
		rc = s_GetDelegateFn(
			s_HostHandle,
			hdt_load_assembly_and_get_function_pointer,
			(void**)&s_LoadFunction
		);

		if (rc != 0 || s_LoadFunction == nullptr)
		{
			PR_CORE_ERROR("获取 load_assembly 委托失败: {0:x}", rc);
			return false;
		}

		s_AssemblyPath = std::wstring(assemblyPath.begin(), assemblyPath.end());

		ScriptEngineRegistry::RegisterAll();

		typedef void (CORECLR_DELEGATE_CALLTYPE* engine_init_fn)();
		engine_init_fn initFunc = nullptr;
		LoadFunction(L"ExampleApp.Test, ExampleApp", L"Init", (void**)&initFunc);
		initFunc();

		return rc == 0;

	}

	bool ScriptEngine::LoadFunction(const std::wstring& className, const std::wstring& funcName, void** func)
	{
		auto path = s_AssemblyPath;
		int rc = s_LoadFunction(
			path.c_str(),
			className.c_str(),
			funcName.c_str(),
			UNMANAGEDCALLERSONLY_METHOD,
			nullptr,
			func
		);
		if (rc != 0) PR_CORE_ASSERT(false, "");
		return rc == 0;
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{

	}

	void ScriptEngine::OnUpdateEntity(uint32_t entityID, float ts)
	{

	}

	void ScriptEngine::OnInitEntity(ScriptComponent& script, uint32_t entityID, uint32_t sceneID)
	{

	}

	void ScriptEngine::RegisterEngineFunctions()
	{

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