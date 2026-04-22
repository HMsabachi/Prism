#include "prpch.h"
#include "ScriptEngine.h"
#include <nethost.h>
#include <hostfxr.h>
#include <coreclr_delegates.h>

namespace Prism
{
	// 全局函数指针
	static hostfxr_initialize_for_runtime_config_fn s_InitFn = nullptr;
	static hostfxr_get_runtime_delegate_fn s_GetDelegateFn = nullptr;
	static hostfxr_close_fn s_CloseFn = nullptr;

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
		return false;
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