#pragma once
#include "Scripting/Python/PythonScriptCore.h"

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

	// Input
	Python::ScriptValue* Prism_Input_IsKeyPressed(Python::ScriptValue* self, Python::ScriptValue* args);

	// Entity
	Python::ScriptValue* Prism_Entity_GetTransform(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_Entity_SetTransform(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_Entity_CreateComponent(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_Entity_HasComponent(Python::ScriptValue* self, Python::ScriptValue* args);

	// TransformComponent
	Python::ScriptValue* Prism_TransformComponent_GetPosition(Python::ScriptValue* self, Python::ScriptValue* args);
	Python::ScriptValue* Prism_TransformComponent_SetPosition(Python::ScriptValue* self, Python::ScriptValue* args);

	// 注册
	void RegisterPrismModule();

} // namespace Prism::Script
