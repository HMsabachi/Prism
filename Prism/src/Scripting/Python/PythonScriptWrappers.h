#pragma once
#include "Scripting/Python/Interop/PythonScriptCore.h"

namespace Prism::Script
{

#pragma region Log
	enum class LogLevel : int32_t
	{
		Trace = BIT(0),
		Debug = BIT(1),
		Info = BIT(2),
		Warn = BIT(3),
		Error = BIT(4),
		Critical = BIT(5)
	};
	Python::ScriptValue* Prism_Log_LogMessage(Python::ScriptValue* self, Python::ScriptValue* args);
#pragma endregion

	// Time
	Python::ScriptValue* Prism_Time_GetDeltaTime(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_Time_GetTime(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_Time_GetUnscaledDeltaTime(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_Time_GetUnscaledTime(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_Time_GetFixedDeltaTime(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_Time_GetFrameCount(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_Time_GetTimeScale(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_Time_SetTimeScale(Python::ScriptValue* self, Python::ScriptValue* args);

	// Input
	Python::ScriptValue* Prism_Input_IsKeyPressed(Python::ScriptValue* self, Python::ScriptValue* args);

	// Entity
	Python::ScriptValue* Prism_Entity_GetTransform(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_Entity_SetTransform(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_Entity_CreateComponent(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_Entity_HasComponent(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_Entity_AddBehaviour(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_Entity_FindEntityByTag(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_Entity_RemoveBehaviour(Python::ScriptValue* self, Python::ScriptValue* args);

	// TransformComponent
	Python::ScriptValue* Prism_TransformComponent_GetPosition(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_TransformComponent_SetPosition(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_TransformComponent_GetRotation(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_TransformComponent_SetRotation(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_TransformComponent_GetScale(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_TransformComponent_SetScale(Python::ScriptValue* self, Python::ScriptValue* args);

	// 注册
	void RegisterPrismModule();

} // namespace Prism::Script
