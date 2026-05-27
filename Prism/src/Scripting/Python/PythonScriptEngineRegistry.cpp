#include "prpch.h"
#include "PythonScriptEngineRegistry.h"
#include "PythonScriptWrappers.h"
#include "Scripting/Python/Interop/PythonScriptCore.h"

#include "Prism/Scene/Components.h"
#include "Prism/Scene/Entity.h"
#include "Prism/Utilities/TypeInfo.h"

namespace Prism
{
	std::unordered_map<uint64_t, std::function<void(Entity&)>> s_PythonCreateComponentFuncs;
	std::unordered_map<uint64_t, std::function<bool(Entity&)>> s_PythonHasComponentFuncs;

	template<typename TComponent>
	static void RegisterPythonComponent()
	{
		const TypeNameString& name = TypeInfo<TComponent, true>().Name();

		// 获取 Python 组件类对象
		Python::ScriptModule mod = Python::ScriptModule::Import("Prism.Component");
		PR_CORE_ASSERT(mod.IsValid(), "Python module Prism.Component not found!");
		Python::ScriptClass cls = Python::ScriptClass::From(mod, name.data());
		PR_CORE_ASSERT(cls.IsValid(), "Python class {} not found in Prism.Component!", name);

		// 用 Python 类型对象地址做 key（与 id(cls) 等效）
		uint64_t typeId = cls.GetTypeId();
		s_PythonCreateComponentFuncs[typeId] = [](Entity& e) { e.AddComponent<TComponent>(); };
		s_PythonHasComponentFuncs[typeId] = [](Entity& e) { return e.HasComponent<TComponent>(); };
	}

	static void InitComponentTypes()
	{
		RegisterPythonComponent<TagComponent>();
		RegisterPythonComponent<TransformComponent>();
		RegisterPythonComponent<MeshComponent>();
		RegisterPythonComponent<CameraComponent>();
		RegisterPythonComponent<SpriteRendererComponent>();
		RegisterPythonComponent<MaterialComponent>();
		RegisterPythonComponent<RigidBody2DComponent>();
		RegisterPythonComponent<BoxCollider2DComponent>();
		RegisterPythonComponent<CircleCollider2DComponent>();
		RegisterPythonComponent<RigidBodyComponent>();
		RegisterPythonComponent<BoxColliderComponent>();
		RegisterPythonComponent<SphereColliderComponent>();
		RegisterPythonComponent<CapsuleColliderComponent>();
	}


	void PythonScriptEngineRegistry::RegisterAll()
	{
		using namespace Prism::Script;
		Python::NativeModule mod("PrismNative");

#define PR_PYTHON_FUNCTION(func, doc) mod.AddFunction(#func, func, doc)
		// Log
		PR_PYTHON_FUNCTION(Prism_Log_LogMessage, "Log(level, message)");

		// Time
		PR_PYTHON_FUNCTION(Prism_Time_GetDeltaTime, "GetDeltaTime() -> float");
		PR_PYTHON_FUNCTION(Prism_Time_GetTime, "GetTime() -> float");
		PR_PYTHON_FUNCTION(Prism_Time_GetUnscaledDeltaTime, "GetUnscaledDeltaTime() -> float");
		PR_PYTHON_FUNCTION(Prism_Time_GetUnscaledTime, "GetUnscaledTime() -> float");
		PR_PYTHON_FUNCTION(Prism_Time_GetFixedDeltaTime, "GetFixedDeltaTime() -> float");
		PR_PYTHON_FUNCTION(Prism_Time_GetFrameCount, "GetFrameCount() -> uint64");
		PR_PYTHON_FUNCTION(Prism_Time_GetTimeScale, "GetTimeScale() -> float");
		PR_PYTHON_FUNCTION(Prism_Time_SetTimeScale, "SetTimeScale(scale)");

		// Input
		PR_PYTHON_FUNCTION(Prism_Input_IsKeyPressed, "IsKeyPressed(key) -> bool");

		// Entity
		PR_PYTHON_FUNCTION(Prism_Entity_GetTransform, "GetTransform(entityID) -> mat4");
		PR_PYTHON_FUNCTION(Prism_Entity_SetTransform, "SetTransform(entityID, mat4)");
		PR_PYTHON_FUNCTION(Prism_Entity_CreateComponent, "CreateComponent(entityID, typeName)");
		PR_PYTHON_FUNCTION(Prism_Entity_HasComponent, "HasComponent(entityID, typeName) -> bool");
		PR_PYTHON_FUNCTION(Prism_Entity_AddBehaviour, "AddBehaviour(entityID, moduleName, className) -> object");
		PR_PYTHON_FUNCTION(Prism_Entity_FindEntityByTag, "FindEntityByTag(tag) -> uint64");
		PR_PYTHON_FUNCTION(Prism_Entity_RemoveBehaviour, "RemoveBehaviour(entityID, behaviourID)");

		// TagComponent
		PR_PYTHON_FUNCTION(Prism_TagComponent_GetTag, "GetTag(entityID) -> string");
		PR_PYTHON_FUNCTION(Prism_TagComponent_SetTag, "SetTag(entityID, tag)");

		// TransformComponent
		PR_PYTHON_FUNCTION(Prism_TransformComponent_GetPosition, "GetPosition(entityID) -> vec3");
		PR_PYTHON_FUNCTION(Prism_TransformComponent_SetPosition, "SetPosition(entityID, vec3)");
		PR_PYTHON_FUNCTION(Prism_TransformComponent_GetRotation, "GetRotation(entityID) -> vec3 radians");
		PR_PYTHON_FUNCTION(Prism_TransformComponent_SetRotation, "SetRotation(entityID, vec3) degrees");
		PR_PYTHON_FUNCTION(Prism_TransformComponent_GetScale, "GetScale(entityID) -> vec3");
		PR_PYTHON_FUNCTION(Prism_TransformComponent_SetScale, "SetScale(entityID, vec3)");

		// RigidBody2DComponent
		PR_PYTHON_FUNCTION(Prism_RigidBody2DComponent_ApplyLinearImpulse, "ApplyLinearImpulse(entityID, impulse, offset, wake)");
		PR_PYTHON_FUNCTION(Prism_RigidBody2DComponent_GetLinearVelocity, "GetLinearVelocity(entityID) -> vec2");
		PR_PYTHON_FUNCTION(Prism_RigidBody2DComponent_SetLinearVelocity, "SetLinearVelocity(entityID, velocity)");

		// RigidBodyComponent (3D)
		PR_PYTHON_FUNCTION(Prism_RigidBodyComponent_AddForce, "AddForce(entityID, force, forceMode)");
		PR_PYTHON_FUNCTION(Prism_RigidBodyComponent_AddTorque, "AddTorque(entityID, torque, forceMode)");
		PR_PYTHON_FUNCTION(Prism_RigidBodyComponent_GetLinearVelocity, "GetLinearVelocity(entityID) -> vec3");
		PR_PYTHON_FUNCTION(Prism_RigidBodyComponent_SetLinearVelocity, "SetLinearVelocity(entityID, velocity)");

		mod.Register();
		// 注册所有组件类型
		InitComponentTypes();
		PR_CORE_TRACE("[Python] PrismNative 模块已注册");
	}
}