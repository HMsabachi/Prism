#include "prpch.h"
#include "ScriptEngineRegistry.h"
#include "ScriptEngine.h"
#include <coreclr_delegates.h>

namespace Prism
{
	static void CoreLog_Native(const char* message)
	{
		PR_CORE_TRACE(message);
	}


	struct FunctionTable
	{
		void (__cdecl* coreLog)(const char*);
	}functionTable;
	static_assert(sizeof(FunctionTable) == sizeof(void*));

	void ScriptEngineRegistry::RegisterAll()
	{
		void (CORECLR_DELEGATE_CALLTYPE *registerFunc)(FunctionTable*);
		ScriptEngine::LoadFunction(L"Prism.Core, Prism.Scripting", L"PushFunctionTable", (void**)&registerFunc);
		functionTable.coreLog = CoreLog_Native;
		registerFunc(&functionTable);
	}
}