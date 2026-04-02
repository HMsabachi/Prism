#include "prpch.h"
#include "Core.h"

#define PRISM_BUILD_ID "v0.1a"

namespace Prism
{
	void InitializeCore()
	{
		Prism::Log::Init();
		PR_CORE_TRACE("Prism Engine {0}", PRISM_BUILD_ID);
		PR_CORE_TRACE("Initializing  ... 正在初始化 ...");
	}

	void ShutdownCore()
	{
		PR_CORE_TRACE("Shutingdown  ... 正在关闭 ...");
	}
}
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Prism::InitializeCore();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		Prism::ShutdownCore();
		break;
	}
	return TRUE;
}