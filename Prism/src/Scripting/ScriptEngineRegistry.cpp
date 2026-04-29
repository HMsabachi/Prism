#include "prpch.h"
#include "ScriptEngineRegistry.h"
#include "ScriptEngine.h"
#include "ScriptWrappers.h"
#include <coreclr_delegates.h>
#include <Rolky/Assembly.hpp>
#include <spdlog/fmt/fmt.h>

#include "Native/String.h"
#include "Prism/Utilities/TypeInfo.h"
namespace Prism
{
	std::unordered_map<Rolky::TypeId, std::function<void(Entity&)>> s_CreateComponentFuncs;
	std::unordered_map<Rolky::TypeId, std::function<bool(Entity&)>> s_HasComponentFuncs;

	template<typename TComponent>
	static void RegisterManagedComponent(Rolky::ManagedAssembly& coreAssembly)
	{
		const TypeNameString& componentTypeName = TypeInfo<TComponent, true>().Name();
		std::string componentName = "Prism."; 
		componentName += componentTypeName;
		auto& type = coreAssembly.GetType(componentName);
		if (type)
		{
			s_CreateComponentFuncs[type.GetTypeId()] = [](Entity& entity) { entity.AddComponent<TComponent>(); };
			s_HasComponentFuncs[type.GetTypeId()] = [](Entity& entity) { return entity.HasComponent<TComponent>(); };
		}
		else
		{
			PR_CORE_ASSERT(false, "No C# component class found for {}!");
		}
	}

	static void InitComponentTypes()
	{
		auto& engineAssembly = ScriptEngine::GetEngineAssembly();
		RegisterManagedComponent<TagComponent>(engineAssembly);
		RegisterManagedComponent<TransformComponent>(engineAssembly);
		RegisterManagedComponent<MeshComponent>(engineAssembly);
		RegisterManagedComponent<ScriptComponent>(engineAssembly);
		RegisterManagedComponent<CameraComponent>(engineAssembly);
		RegisterManagedComponent<SpriteRendererComponent>(engineAssembly);
	}

	void ScriptEngineRegistry::RegisterAll()
	{
        InitComponentTypes();
		auto& engineAssembly = ScriptEngine::GetEngineAssembly();
#define PR_ADD_INTERNAL_CALL(func) engineAssembly.AddInternalCall("Prism.InternalCalls", #func, (void*)&func)
		using namespace Script;
		// Log
		PR_ADD_INTERNAL_CALL(Prism_Log_LogMessage);
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
        PR_ADD_INTERNAL_CALL(Prism_Entity_CreateComponent);
        PR_ADD_INTERNAL_CALL(Prism_Entity_HasComponent);

		engineAssembly.UploadInternalCalls();

	}
}