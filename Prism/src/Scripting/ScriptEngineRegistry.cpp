#include "prpch.h"
#include "ScriptEngineRegistry.h"
#include "ScriptEngine.h"
#include "ScriptWrappers.h"
#include <coreclr_delegates.h>

#include "Reflection/Reflection.h"

#include "Native/String.h"
namespace Prism
{
	template<typename TComponent>
	static void RegisterManagedComponent(Coral::ManagedAssembly& coreAssembly)
	{
		// NOTE(Peter): Get the demangled type name of TComponent
		const TypeNameString& componentTypeName = TypeInfo<TComponent, true>().Name();
		std::string componentName = std::format("Hazel.{}", componentTypeName);

		// Backwards compatibility: Submesh component is "Hazel.Mesh" for scripting.
		if constexpr (std::is_same_v<TComponent, SubmeshComponent>)
		{
			componentName = "Hazel.MeshComponent";
		}

		auto& type = coreAssembly.GetType(componentName);

		if (type)
		{
			s_CreateComponentFuncs[type.GetTypeId()] = [](Entity& entity) { entity.AddComponent<TComponent>(); };
			s_HasComponentFuncs[type.GetTypeId()] = [](Entity& entity) { return entity.HasComponent<TComponent>(); };
			s_RemoveComponentFuncs[type.GetTypeId()] = [](Entity& entity) { entity.RemoveComponent<TComponent>(); };
		}
		else
		{
			HZ_CORE_VERIFY(false, "No C# component class found for {}!", componentName);
		}
	}

	struct FunctionTable
	{
		// NativeString
		Native::String(__cdecl* createNativeString)(const char*);
		const char* (__cdecl* nativeStringToCString)(const Native::String*);
		void(__cdecl* freeNativeString)(Native::String*);
		Native::String(__cdecl* copyNativeString)(const Native::String*);
	}functionTable;

#define PR_ADD_INTERNAL_CALL(func) Reflection::AddInternalCall("Prism.InternalCalls", #func, (void*)&func)
	void ScriptEngineRegistry::RegisterAll()
	{
		void (CORECLR_DELEGATE_CALLTYPE *registerFunc)(FunctionTable*);
		ScriptEngine::LoadFunction(L"Prism.Core, Prism.Scripting", L"PushFunctionTable", (void**)&registerFunc);
		functionTable.createNativeString = Native::CreateNativeString;
		functionTable.nativeStringToCString = Native::NativeStringToCString;
		functionTable.freeNativeString = Native::FreeNativeString;
		functionTable.copyNativeString = Native::CopyNativeString;
		registerFunc(&functionTable);
		using namespace Script;
		// Log
		PR_ADD_INTERNAL_CALL(Prism_Log_LogMessage);
		PR_ADD_INTERNAL_CALL(Prism_Log_Core_Trace);
		PR_ADD_INTERNAL_CALL(Prism_Log_Core_Info);
		PR_ADD_INTERNAL_CALL(Prism_Log_Core_Warn);
		PR_ADD_INTERNAL_CALL(Prism_Log_Core_Error);
		PR_ADD_INTERNAL_CALL(Prism_Log_Core_Fatal);
		// Time
		PR_ADD_INTERNAL_CALL(Prism_Time_GetDeltaTime);
		PR_ADD_INTERNAL_CALL(Prism_Time_GetUnscaledDeltaTime);
		PR_ADD_INTERNAL_CALL(Prism_Time_GetTime);
		PR_ADD_INTERNAL_CALL(Prism_Time_GetUnscaledTime);
		PR_ADD_INTERNAL_CALL(Prism_Time_GetFixedDeltaTime);
		PR_ADD_INTERNAL_CALL(Prism_Time_GetFrameCount);
		PR_ADD_INTERNAL_CALL(Prism_Time_SetTimeScale);
		PR_ADD_INTERNAL_CALL(Prism_Time_GetTimeScale);
		// Math
		PR_ADD_INTERNAL_CALL(Prism_Noise_PerlinNoise);
		// Input
		PR_ADD_INTERNAL_CALL(Prism_Input_IsKeyPressed);
		// Entity
		PR_ADD_INTERNAL_CALL(Prism_Entity_GetTransform);
		PR_ADD_INTERNAL_CALL(Prism_Entity_SetTransform);

	}
}