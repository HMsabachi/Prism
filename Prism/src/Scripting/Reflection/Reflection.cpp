#include "prpch.h"
#include "Reflection.h"
#include "../Native/String.h"
#include "../ScriptEngine.h"

namespace Prism
{
	namespace Reflection
	{
		static void(__cdecl* s_AddInternalCall)(const char*, const char*, void*) = nullptr;

		void AddInternalCall(const char* className, const char* funcName, void* address)
		{
			if (!s_AddInternalCall) ScriptEngine::LoadFunction(L"Prism.InternalCallRegistry, Prism.Scripting", L"AddInternalCall", (void**)&s_AddInternalCall);
			s_AddInternalCall(className, funcName, address);
		}
	}
}