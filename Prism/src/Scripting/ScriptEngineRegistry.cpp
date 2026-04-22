#include "prpch.h"
#include "ScriptEngineRegistry.h"
#include "ScriptEngine.h"
#include "ScriptWrappers.h"
#include <coreclr_delegates.h>

namespace Prism
{


	struct FunctionTable
	{
		// LOG
		void (__cdecl* logCoreTrace)(const char*);
		void (__cdecl* logCoreInfo)(const char*);
		void (__cdecl* logCoreWarn)(const char*);
		void (__cdecl* logCoreError)(const char*);
		void (__cdecl* logCoreFatal)(const char*);
	}functionTable;

	void ScriptEngineRegistry::RegisterAll()
	{
		void (CORECLR_DELEGATE_CALLTYPE *registerFunc)(FunctionTable*);
		ScriptEngine::LoadFunction(L"Prism.Core, Prism.Scripting", L"PushFunctionTable", (void**)&registerFunc);
		functionTable.logCoreTrace = Script::Prism_Log_Core_Trace;
		functionTable.logCoreInfo = Script::Prism_Log_Core_Info;
		functionTable.logCoreWarn = Script::Prism_Log_Core_Warn;
		functionTable.logCoreError = Script::Prism_Log_Core_Error;
		functionTable.logCoreFatal = Script::Prism_Log_Core_Fatal;
		registerFunc(&functionTable);
	}
}