#include "prpch.h"
#include "ScriptEngineRegistry.h"
#include "ScriptEngine.h"
#include "ScriptWrappers.h"
#include <coreclr_delegates.h>

#include "Native/String.h"
namespace Prism
{


	struct FunctionTable
	{
		// LOG
		void(__cdecl* logCoreTrace)(const char*);
		void(__cdecl* logCoreInfo)(const char*);
		void(__cdecl* logCoreWarn)(const char*);
		void(__cdecl* logCoreError)(const char*);
		void(__cdecl* logCoreFatal)(const char*);

		// NativeString
		Native::String(__cdecl* createNativeString)(const char*);
		const char* (__cdecl* nativeStringToCString)(const Native::String*);
		void(__cdecl* freeNativeString)(Native::String*);
		Native::String(__cdecl* copyNativeString)(const Native::String*);

		// Log
		void(__cdecl* logMessage)(Script::LogLevel, Native::String);

		// Entity
		void(__cdecl* entityGetTransform)(uint32_t, uint32_t, glm::mat4*);
		void(__cdecl* entitySetTransform)(uint32_t, uint32_t, glm::mat4*);
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

		// NativeString
		functionTable.createNativeString = Native::CreateNativeString;
		functionTable.nativeStringToCString = Native::NativeStringToCString;
		functionTable.freeNativeString = Native::FreeNativeString;
		functionTable.copyNativeString = Native::CopyNativeString;
		// Log
		functionTable.logMessage = Script::Log_LogMessage;
		// Entity
		functionTable.entityGetTransform = Script::Prism_Entity_GetTransform;
		functionTable.entitySetTransform = Script::Prism_Entity_SetTransform;

		registerFunc(&functionTable);
	}
}