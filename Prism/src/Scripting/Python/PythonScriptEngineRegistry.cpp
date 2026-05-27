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

		// Math
		PR_PYTHON_FUNCTION(Prism_Noise_PerlinNoise, "PerlinNoise(x, y) -> float");

		// Input
		PR_PYTHON_FUNCTION(Prism_Input_IsKeyPressed, "IsKeyPressed(key) -> bool");

		// Entity
		PR_PYTHON_FUNCTION(Prism_Entity_GetTransform, "GetTransform(entityID) -> mat4");
		PR_PYTHON_FUNCTION(Prism_Entity_SetTransform, "SetTransform(entityID, mat4)");
		PR_PYTHON_FUNCTION(Prism_Entity_CreateComponent, "CreateComponent(entityID, typeName)");
		PR_PYTHON_FUNCTION(Prism_Entity_HasComponent, "HasComponent(entityID, typeName) -> bool");
		PR_PYTHON_FUNCTION(Prism_Entity_AddBehaviour, "AddBehaviour(entityID, cls) -> object");
		PR_PYTHON_FUNCTION(Prism_Entity_FindEntityByTag, "FindEntityByTag(tag) -> uint64");
		PR_PYTHON_FUNCTION(Prism_Entity_RemoveBehaviour, "RemoveBehaviour(entityID, behaviourID)");
		PR_PYTHON_FUNCTION(Prism_Entity_GetBehaviour, "GetBehaviour(entityID, cls) -> object");
		PR_PYTHON_FUNCTION(Prism_Behaviour_GetEnabled, "GetEnabled(behaviourID) -> bool");
		PR_PYTHON_FUNCTION(Prism_Behaviour_SetEnabled, "SetEnabled(behaviourID, enabled)");

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

		// Mesh
		PR_PYTHON_FUNCTION(Prism_Mesh_Constructor, "Mesh(cpp_handle, filepath)");
		PR_PYTHON_FUNCTION(Prism_Mesh_Destructor, "~Mesh(cpp_handle)");
		PR_PYTHON_FUNCTION(Prism_Mesh_GetMaterial, "GetMaterial(cpp_handle) -> handle");
		PR_PYTHON_FUNCTION(Prism_Mesh_GetMaterialByIndex, "GetMaterialByIndex(cpp_handle, index) -> handle");
		PR_PYTHON_FUNCTION(Prism_Mesh_GetMaterialCount, "GetMaterialCount(cpp_handle) -> int");
		PR_PYTHON_FUNCTION(Prism_Mesh_SetMaterialByIndex, "SetMaterialByIndex(cpp_handle, index, materialHandle)");
		PR_PYTHON_FUNCTION(Prism_Mesh_SetOverrideMaterial, "SetOverrideMaterial(cpp_handle, materialHandle)");
		PR_PYTHON_FUNCTION(Prism_Mesh_GetOverrideMaterial, "GetOverrideMaterial(cpp_handle) -> handle");

		// MeshFactory
		PR_PYTHON_FUNCTION(Prism_MeshFactory_CreatePlane, "CreatePlane(width, height) -> handle");

		// MeshComponent
		PR_PYTHON_FUNCTION(Prism_MeshComponent_GetMesh, "GetMesh(entityID) -> handle");
		PR_PYTHON_FUNCTION(Prism_MeshComponent_SetMesh, "SetMesh(entityID, handle)");

		// Texture2D
		PR_PYTHON_FUNCTION(Prism_Texture2D_Constructor, "Texture2D(cpp_handle, width, height)");
		PR_PYTHON_FUNCTION(Prism_Texture2D_Destructor, "~Texture2D(cpp_handle)");
		PR_PYTHON_FUNCTION(Prism_Texture2D_SetData, "SetData(cpp_handle, data)");

		// Material
		PR_PYTHON_FUNCTION(Prism_Material_Constructor, "Material(cpp_handle, shaderName)");
		PR_PYTHON_FUNCTION(Prism_Material_Destructor, "~Material(cpp_handle)");
		PR_PYTHON_FUNCTION(Prism_Material_SetFloat, "SetFloat(cpp_handle, uniform, value)");
		PR_PYTHON_FUNCTION(Prism_Material_SetTexture, "SetTexture(cpp_handle, uniform, texHandle)");
		PR_PYTHON_FUNCTION(Prism_Material_SetKeyword, "SetKeyword(cpp_handle, name, enabled)");
		PR_PYTHON_FUNCTION(Prism_Material_IsKeywordEnabled, "IsKeywordEnabled(cpp_handle, name) -> bool");

		// MaterialInstance
		PR_PYTHON_FUNCTION(Prism_MaterialInstance_Constructor, "MaterialInstance(cpp_handle, parentHandle)");
		PR_PYTHON_FUNCTION(Prism_MaterialInstance_Destructor, "~MaterialInstance(cpp_handle)");
		PR_PYTHON_FUNCTION(Prism_MaterialInstance_SetFloat, "SetFloat(cpp_handle, uniform, value)");
		PR_PYTHON_FUNCTION(Prism_MaterialInstance_SetVector3, "SetVector3(cpp_handle, uniform, vec3)");
		PR_PYTHON_FUNCTION(Prism_MaterialInstance_SetVector4, "SetVector4(cpp_handle, uniform, vec4)");
		PR_PYTHON_FUNCTION(Prism_MaterialInstance_SetTexture, "SetTexture(cpp_handle, uniform, texHandle)");
		PR_PYTHON_FUNCTION(Prism_MaterialInstance_SetKeyword, "SetKeyword(cpp_handle, name, enabled)");
		PR_PYTHON_FUNCTION(Prism_MaterialInstance_IsKeywordEnabled, "IsKeywordEnabled(cpp_handle, name) -> bool");

		// MaterialComponent
		PR_PYTHON_FUNCTION(Prism_MaterialComponent_GetMaterial, "GetMaterial(entityID) -> handle");
		PR_PYTHON_FUNCTION(Prism_MaterialComponent_SetMaterial, "SetMaterial(entityID, handle)");

		mod.Register();
		InitComponentTypes();
		PR_CORE_TRACE("[Python] PrismNative 模块已注册");
	}
}