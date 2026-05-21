#include "prpch.h"
#include "PythonScriptWrappers.h"

#include "Prism/Core/Input.h"
#include "Prism/Core/KeyCodes.h"
#include "Prism/Scene/Scene.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Scene/Components.h"

#include "Scripting/ScriptEngineManager.h"

#include <glm/gtc/type_ptr.hpp>

namespace Prism {
	extern std::unordered_map<std::string, std::function<void(Entity&)>> s_PythonCreateComponentFuncs;
	extern std::unordered_map<std::string, std::function<bool(Entity&)>> s_PythonHasComponentFuncs;
}

namespace Prism::Script
{

	static Entity GetEntityFromEntityID(uint64_t entityID)
	{
		Ref<Scene> scene = ScriptEngineManager::Get(ScriptLanguage::Python)->GetCurrentSceneContext();
		PR_CORE_ASSERT(scene, "没有激活的场景！");
		const auto& entityMap = scene->GetEntityMap();
		PR_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "无效的实体 ID！");
		return entityMap.at(entityID);
	}

#pragma region Log

	Python::ScriptValue* Prism_Log_LogMessage(Python::ScriptValue* self, Python::ScriptValue* args)
	{
		Python::ScriptRef argsRef(args);
		LogLevel level = static_cast<LogLevel>(Python::ValueToInt(Python::GetTupleElement(argsRef, 0)));
		std::string message = Python::ValueToString(Python::GetTupleElement(argsRef, 1));
		message = "[Python]: " + message;

		switch (level)
		{
		case LogLevel::Trace:	PR_CORE_TRACE(message); break;
		case LogLevel::Debug:	PR_CORE_INFO(message);  break;
		case LogLevel::Info:	PR_CORE_INFO(message);  break;
		case LogLevel::Warn:	PR_CORE_WARN(message);  break;
		case LogLevel::Error:	PR_CORE_ERROR(message); break;
		case LogLevel::Critical:PR_CORE_FATAL(message); break;
		}
		return Python::NoneValue().Detach();
	}

#pragma endregion

#pragma region Time

	Python::ScriptValue* Prism_Time_GetDeltaTime(Python::ScriptValue* self, Python::ScriptValue* args)
	{
		return Python::FloatToValue(Time::GetDeltaTime()).Detach();
	}

	Python::ScriptValue* Prism_Time_GetTime(Python::ScriptValue* self, Python::ScriptValue* args)
	{
		return Python::FloatToValue(Time::GetTime()).Detach();
	}

#pragma endregion

#pragma region Input

	Python::ScriptValue* Prism_Input_IsKeyPressed(Python::ScriptValue* self, Python::ScriptValue* args)
	{
		Python::ScriptRef argsRef(args);
		KeyCode key = static_cast<KeyCode>(Python::ValueToInt(Python::GetTupleElement(argsRef, 0)));
		return Python::BoolToValue(Input::IsKeyPressed(key)).Detach();
	}

#pragma endregion

#pragma region Entity

	Python::ScriptValue* Prism_Entity_GetTransform(Python::ScriptValue* self, Python::ScriptValue* args)
	{
		Python::ScriptRef argsRef(args);
		Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
		auto& tc = entity.GetComponent<TransformComponent>();
		glm::mat4 transform = tc.GetTransform();

		// 返回 16 个 float 的 tuple
		Python::ScriptRef elements[16];
		const float* data = glm::value_ptr(transform);
		for (int i = 0; i < 16; i++)
			elements[i] = Python::FloatToValue(data[i]);

		return Python::MakeTuple(elements, 16).Detach();
	}

	Python::ScriptValue* Prism_Entity_SetTransform(Python::ScriptValue* self, Python::ScriptValue* args)
	{
		Python::ScriptRef argsRef(args);
		Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
		Python::ScriptRef matTuple = Python::GetTupleElement(argsRef, 1);

		glm::mat4 transform;
		float* data = glm::value_ptr(transform);
		for (int i = 0; i < 16; i++)
			data[i] = Python::ValueToFloat(Python::GetTupleElement(matTuple, i));

		auto& tc = entity.GetComponent<TransformComponent>();
		tc.SetTransform(transform);
		return Python::NoneValue().Detach();
	}

	Python::ScriptValue* Prism_Entity_CreateComponent(Python::ScriptValue* self, Python::ScriptValue* args)
	{
		Python::ScriptRef argsRef(args);
		Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
		std::string typeName = Python::ValueToString(Python::GetTupleElement(argsRef, 1));

		auto it = s_PythonCreateComponentFuncs.find(typeName);
		if (it != s_PythonCreateComponentFuncs.end())
			it->second(entity);

		return Python::NoneValue().Detach();
	}

	Python::ScriptValue* Prism_Entity_HasComponent(Python::ScriptValue* self, Python::ScriptValue* args)
	{
		Python::ScriptRef argsRef(args);
		Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
		std::string typeName = Python::ValueToString(Python::GetTupleElement(argsRef, 1));

		auto it = s_PythonHasComponentFuncs.find(typeName);
		if (it != s_PythonHasComponentFuncs.end())
			return Python::BoolToValue(it->second(entity)).Detach();

		return Python::BoolToValue(false).Detach();
	}

#pragma endregion

#pragma region TransformComponent

	Python::ScriptValue* Prism_TransformComponent_GetPosition(Python::ScriptValue* self, Python::ScriptValue* args)
	{
		Python::ScriptRef argsRef(args);
		Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
		auto& tc = entity.GetComponent<TransformComponent>();

		Python::ScriptRef elements[3] = {
			Python::FloatToValue(tc.Position.x),
			Python::FloatToValue(tc.Position.y),
			Python::FloatToValue(tc.Position.z)
		};
		return Python::MakeTuple(elements, 3).Detach();
	}

	Python::ScriptValue* Prism_TransformComponent_SetPosition(Python::ScriptValue* self, Python::ScriptValue* args)
	{
		Python::ScriptRef argsRef(args);
		Entity entity = GetEntityFromEntityID(Python::ValueToUInt64(Python::GetTupleElement(argsRef, 0)));
		Python::ScriptRef posTuple = Python::GetTupleElement(argsRef, 1);

		auto& tc = entity.GetComponent<TransformComponent>();
		tc.Position.x = Python::ValueToFloat(Python::GetTupleElement(posTuple, 0));
		tc.Position.y = Python::ValueToFloat(Python::GetTupleElement(posTuple, 1));
		tc.Position.z = Python::ValueToFloat(Python::GetTupleElement(posTuple, 2));
		return Python::NoneValue().Detach();
	}

#pragma endregion

	void RegisterPrismModule()
	{
		Python::NativeModule mod("PrismNative");

		// Log
	#define PR_PYTHON_FUNCTION(func, doc) mod.AddFunction(#func, func, doc)

		PR_PYTHON_FUNCTION(Prism_Log_LogMessage, "Log(level, message)");

		// Time
		PR_PYTHON_FUNCTION(Prism_Time_GetDeltaTime, "GetDeltaTime() -> float");
		PR_PYTHON_FUNCTION(Prism_Time_GetTime, "GetTime() -> float");

		// Input
		PR_PYTHON_FUNCTION(Prism_Input_IsKeyPressed, "IsKeyPressed(key) -> bool");

		// Entity
		PR_PYTHON_FUNCTION(Prism_Entity_GetTransform, "GetTransform(entityID) -> tuple[16]");
		PR_PYTHON_FUNCTION(Prism_Entity_SetTransform, "SetTransform(entityID, matTuple)");
		PR_PYTHON_FUNCTION(Prism_Entity_CreateComponent, "CreateComponent(entityID, typeName)");
		PR_PYTHON_FUNCTION(Prism_Entity_HasComponent, "HasComponent(entityID, typeName) -> bool");

		// TransformComponent
		PR_PYTHON_FUNCTION(Prism_TransformComponent_GetPosition, "GetPosition(entityID) -> (x, y, z)");
		PR_PYTHON_FUNCTION(Prism_TransformComponent_SetPosition, "SetPosition(entityID, (x, y, z))");

		mod.Register();

		PR_CORE_TRACE("[Python] PrismNative 模块已注册");
	}

} // namespace Prism::Script
