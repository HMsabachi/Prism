#include "prpch.h"
#include "PythonScriptEngine.h"
#include "PythonScriptWrappers.h"

#include "Prism/Scene/Components.h"

#include <filesystem>
#include <imgui.h>
#include <functional>

#include "PythonPublicField.h"
#include <Python.h>

namespace Prism
{
	std::unordered_map<std::string, std::function<void(Entity&)>> s_PythonCreateComponentFuncs;
	std::unordered_map<std::string, std::function<bool(Entity&)>> s_PythonHasComponentFuncs;

#define REGISTER_PYTHON_COMPONENT(T, Name) \
	s_PythonCreateComponentFuncs[Name] = [](Entity& entity) { entity.AddComponent<T>(); }; \
	s_PythonHasComponentFuncs[Name] = [](Entity& entity) { return entity.HasComponent<T>(); };

	static void RegisterPythonComponentTypes()
	{
		REGISTER_PYTHON_COMPONENT(TagComponent,            "TagComponent");
		REGISTER_PYTHON_COMPONENT(TransformComponent,       "TransformComponent");
		REGISTER_PYTHON_COMPONENT(MeshComponent,            "MeshComponent");
		REGISTER_PYTHON_COMPONENT(CameraComponent,          "CameraComponent");
		REGISTER_PYTHON_COMPONENT(SpriteRendererComponent,  "SpriteRendererComponent");
		REGISTER_PYTHON_COMPONENT(MaterialComponent,        "MaterialComponent");
		REGISTER_PYTHON_COMPONENT(RigidBody2DComponent,     "RigidBody2DComponent");
		REGISTER_PYTHON_COMPONENT(BoxCollider2DComponent,   "BoxCollider2DComponent");
		REGISTER_PYTHON_COMPONENT(CircleCollider2DComponent,"CircleCollider2DComponent");
		REGISTER_PYTHON_COMPONENT(RigidBodyComponent,       "RigidBodyComponent");
		REGISTER_PYTHON_COMPONENT(BoxColliderComponent,     "BoxColliderComponent");
		REGISTER_PYTHON_COMPONENT(SphereColliderComponent,  "SphereColliderComponent");
		REGISTER_PYTHON_COMPONENT(CapsuleColliderComponent, "CapsuleColliderComponent");
	}

#undef REGISTER_PYTHON_COMPONENT
}


namespace Prism
{
	PythonScriptEngine::~PythonScriptEngine()
	{
		Shutdown();
	}

	bool PythonScriptEngine::Initialize()
	{
		PR_PROFILE_FUNCTION();

		if (m_Initialized)
			return true;

		if (!Python::ScriptHost::Initialize())
		{
			PR_CORE_ERROR("[Python] 初始化解释器失败！");
			return false;
		}

		RegisterPythonComponentTypes();
		Script::RegisterPrismModule();

		m_Initialized = true;
		PR_CORE_TRACE("[Python] Python 运行时已初始化");
        auto test = Python::ScriptModule::Import("SmokeTest");
        PR_CORE_TRACE(test.IsValid());
		return true;
	}

	void PythonScriptEngine::Shutdown()
	{
		PR_PROFILE_FUNCTION();

		if (!m_Initialized)
			return;

		m_EntityInstanceMap.clear();

		Python::ScriptHost::Shutdown();
		m_Initialized = false;
		PR_CORE_INFO("[Python] Python 解释器已关闭");
	}

	bool PythonScriptEngine::LoadEngineAssembly(const std::string& path)
	{
		PR_CORE_INFO("[Python] LoadEngineAssembly({0}) — Python 引擎不需要加载程序集", path);
		return true;
	}

	bool PythonScriptEngine::LoadAppAssembly(const std::string& path)
	{
		PR_CORE_INFO("[Python] LoadAppAssembly({0}) — 脚本目录已在 sys.path 中", path);
		return true;
	}

	void PythonScriptEngine::ReloadAssembly(const std::string& path)
	{
		PR_PROFILE_FUNCTION();
		PR_CORE_INFO("[Python] ReloadAssembly({0}) — 暂未实现", path);
	}

	void PythonScriptEngine::SetSceneContext(const Ref<Scene>& scene)
	{
		m_SceneContext = scene;
	}

	const Ref<Scene>& PythonScriptEngine::GetCurrentSceneContext()
	{
		return m_SceneContext;
	}

	void PythonScriptEngine::OnSceneDestruct(UUID sceneID)
	{
		auto it = m_EntityInstanceMap.find(sceneID);
		if (it != m_EntityInstanceMap.end())
		{
			m_EntityInstanceMap.erase(it);
		}
	}

	void PythonScriptEngine::CopyEntityScriptData(UUID dst, UUID src)
	{
		PR_CORE_ASSERT(m_EntityInstanceMap.find(dst) != m_EntityInstanceMap.end());
		PR_CORE_ASSERT(m_EntityInstanceMap.find(src) != m_EntityInstanceMap.end());

		auto& dstEntityMap = m_EntityInstanceMap.at(dst);
		auto& srcEntityMap = m_EntityInstanceMap.at(src);

		for (auto& [entityID, srcEntityData] : srcEntityMap)
		{
			for (auto& [moduleName, srcFieldMap] : srcEntityData.ModuleFieldMap)
			{
				auto& dstModuleFieldMap = dstEntityMap[entityID].ModuleFieldMap;
				for (auto& [fieldName, field] : srcFieldMap)
				{
					if (dstModuleFieldMap.find(moduleName) == dstModuleFieldMap.end())
						continue;
					auto& fieldMap = dstModuleFieldMap.at(moduleName);
					if (fieldMap.find(fieldName) == fieldMap.end())
						continue;
					fieldMap.at(fieldName)->SetStoredValueRaw(field->GetStoredValueBuffer());
				}
			}
		}
	}

	bool PythonScriptEngine::HasEntityScriptData(UUID sceneID)
	{
		return m_EntityInstanceMap.find(sceneID) != m_EntityInstanceMap.end();
	}

	void PythonScriptEngine::InitScriptEntity(Entity entity, const std::string& moduleName)
	{
		PR_PROFILE_FUNCTION();

		UUID sceneID = entity.GetSceneUUID();
		UUID entityID = entity.GetUUID();

		if (moduleName.empty())
			return;
		PythonEntityInstanceData& entityData = m_EntityInstanceMap[sceneID][entityID];
		auto mod = Python::ScriptModule::Import(moduleName.c_str());
		if (!mod.IsValid())
		{
			PR_CORE_ERROR("[Python] 无法导入模块 '{0}'", moduleName);
			return;
		}
		auto scriptClass = Python::ScriptClass::From(mod, moduleName.c_str());
		if (!scriptClass.IsValid())
		{
			PR_CORE_ERROR("[Python] 在模块 '{0}' 中找不到类 '{0}'", moduleName, moduleName);
			return;
		}

		// 4. 创建 script 实例
		auto scriptInstance = std::make_unique<PythonEntityScriptInstance>();
		scriptInstance->Module = std::move(mod);
		scriptInstance->Class = scriptClass;

		// 5. 嗅探方法（bitmask）
		scriptInstance->HasMethods = 0;
		if (scriptClass.HasMethod("OnCreate"))     scriptInstance->HasMethods |= BIT(0);
		if (scriptClass.HasMethod("OnUpdate"))     scriptInstance->HasMethods |= BIT(1);
		if (scriptClass.HasMethod("OnFixedUpdate")) scriptInstance->HasMethods |= BIT(2);
		if (scriptClass.HasMethod("OnDestroy"))    scriptInstance->HasMethods |= BIT(3);

		entityData.Scripts[moduleName] = std::move(scriptInstance);

		// 6. 从类型注解发现 public fields
		auto annotations = scriptClass.GetAnnotations();

		auto& moduleFieldMap = entityData.ModuleFieldMap[moduleName];
		std::unordered_map<std::string, std::unique_ptr<PublicField>> oldFields;
		for (auto& [name, field] : moduleFieldMap)
			oldFields[name] = std::move(field);
		moduleFieldMap.clear();

		if (annotations.empty())
			return;

		// 建临时实例读取默认值
		auto tempObj = scriptClass.CreateInstance();

		for (auto& [fieldName, annoRef] : annotations)
		{
			PyObject* pyType = reinterpret_cast<PyObject*>(annoRef.Get());
			FieldType fieldType = FieldType::None;

			if (pyType == (PyObject*)&PyFloat_Type)
				fieldType = FieldType::Float;
			else if (pyType == (PyObject*)&PyLong_Type)
				fieldType = FieldType::Int;
			else if (pyType == (PyObject*)&PyUnicode_Type)
				fieldType = FieldType::String;

			if (fieldType == FieldType::None)
				continue;

			// 恢复旧字段或创建新字段
			if (oldFields.find(fieldName) != oldFields.end())
			{
				moduleFieldMap[fieldName] = std::move(oldFields.at(fieldName));
			}
			else if (tempObj.IsValid())
			{
				auto field = std::make_unique<PythonPublicField>(fieldName, fieldType, nullptr);
				uint8_t defaultBuf[32] = {};
				tempObj.GetFieldRaw(fieldName.c_str(), defaultBuf);
				field->SetStoredValueRaw(defaultBuf);
				moduleFieldMap[fieldName] = std::move(field);
			}
		}
	}

	void PythonScriptEngine::ShutdownScriptEntity(Entity entity, const std::string& moduleName)
	{
		PythonEntityInstanceData& entityData = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID());

		auto it = entityData.Scripts.find(moduleName);
		if (it != entityData.Scripts.end() && it->second && it->second->Object.IsValid())
		{
			if (it->second->HasMethods & BIT(3))
				it->second->Object.Invoke("OnDestroy");
		}

		entityData.Scripts.erase(moduleName);
		if (entityData.ModuleFieldMap.find(moduleName) != entityData.ModuleFieldMap.end())
			entityData.ModuleFieldMap.erase(moduleName);
	}

	void PythonScriptEngine::InstantiateEntityClass(Entity entity, const std::string& moduleName)
	{
		PR_PROFILE_FUNCTION();

		UUID sceneID = entity.GetSceneUUID();
		UUID entityID = entity.GetUUID();

		auto& entityData = GetEntityInstanceData(sceneID, entityID);
		auto it = entityData.Scripts.find(moduleName);
		PR_CORE_ASSERT(it != entityData.Scripts.end(), "脚本实例未找到！");
		auto& scriptInstance = *it->second;

		// 1. 创建实例
		auto obj = scriptInstance.Class.CreateInstance();
		PR_CORE_ASSERT(obj.IsValid(), "Python 实例创建失败！");
		scriptInstance.Object = std::move(obj);

		// 2. 设置 EntityID
		scriptInstance.Object.SetField<uint64_t>("EntityID", (uint64_t)entityID);

		// 3. 设置字段运行时对象指针并拷贝值
		auto& moduleFieldMap = entityData.ModuleFieldMap[moduleName];
		for (auto& [fieldName, field] : moduleFieldMap)
		{
			auto* pyField = static_cast<PythonPublicField*>(field.get());
			pyField->SetScriptObject(&scriptInstance.Object);
			field->CopyStoredValueToRuntime();
		}

		// 4. 调用 OnCreate
		if (scriptInstance.HasMethods & BIT(0))
		{
			scriptInstance.Object.Invoke("OnCreate");
		}
	}

	bool PythonScriptEngine::ModuleExists(const std::string& moduleName)
	{
		PR_PROFILE_FUNCTION();

		return Python::ScriptModule::ModuleExists(moduleName.c_str());
	}

	void PythonScriptEngine::OnCreateEntity(Entity entity, const std::string& moduleName)
	{
		OnCreateEntity(entity.GetSceneUUID(), entity.GetComponent<IDComponent>().ID, moduleName);
	}

	PythonEntityScriptInstance* PythonScriptEngine::GetScriptInstance(UUID sceneID, UUID entityID, const std::string& moduleName)
	{
		if (m_EntityInstanceMap.find(sceneID) == m_EntityInstanceMap.end())
			return nullptr;
		auto& entityMap = m_EntityInstanceMap.at(sceneID);
		if (entityMap.find(entityID) == entityMap.end())
			return nullptr;
		auto& entityData = entityMap.at(entityID);
		if (entityData.Scripts.find(moduleName) == entityData.Scripts.end())
			return nullptr;
		return entityData.Scripts.at(moduleName).get();
	}

	void PythonScriptEngine::OnCreateEntity(UUID sceneID, UUID entityID, const std::string& moduleName)
	{
		PR_PROFILE_FUNCTION();
		auto* script = GetScriptInstance(sceneID, entityID, moduleName);
		if (script && script->Object.IsValid() && (script->HasMethods & BIT(0)))
			script->Object.Invoke("OnCreate");
	}

	void PythonScriptEngine::OnUpdateEntity(UUID sceneID, UUID entityID, const std::string& moduleName, float ts)
	{
		PR_PROFILE_FUNCTION();
		auto* script = GetScriptInstance(sceneID, entityID, moduleName);
		if (script && script->Object.IsValid() && (script->HasMethods & BIT(1)))
			script->Object.Invoke("OnUpdate");
	}

	void PythonScriptEngine::OnFixedUpdateEntity(UUID sceneID, UUID entityID, const std::string& moduleName)
	{
		PR_PROFILE_FUNCTION();
		auto* script = GetScriptInstance(sceneID, entityID, moduleName);
		if (script && script->Object.IsValid() && (script->HasMethods & BIT(2)))
			script->Object.Invoke("OnFixedUpdate");
	}

	void PythonScriptEngine::OnScriptComponentDestroyed(UUID sceneID, UUID entityID)
	{
		if (m_EntityInstanceMap.find(sceneID) != m_EntityInstanceMap.end())
		{
			auto& entityMap = m_EntityInstanceMap.at(sceneID);
			auto it = entityMap.find(entityID);
			if (it != entityMap.end())
			{
				entityMap.erase(it);
			}
		}
	}

#pragma region Collision callbacks
	void PythonScriptEngine::OnCollision2DBegin(Entity entity)
	{
		OnCollision2DBegin(entity.GetSceneUUID(), entity.GetComponent<IDComponent>().ID);
	}

	void PythonScriptEngine::OnCollision2DBegin(UUID sceneID, UUID entityID)
	{
		PR_PROFILE_FUNCTION();
		if (m_EntityInstanceMap.find(sceneID) == m_EntityInstanceMap.end())
			return;
		auto& entityMap = m_EntityInstanceMap.at(sceneID);
		if (entityMap.find(entityID) == entityMap.end())
			return;
		for (auto& [moduleName, script] : entityMap.at(entityID).Scripts)
		{
			if (script && script->Object.IsValid())
				script->Object.Invoke("OnCollision2DBegin", 5.0f);
		}
	}

	void PythonScriptEngine::OnCollision2DEnd(Entity entity)
	{
		OnCollision2DEnd(entity.GetSceneUUID(), entity.GetComponent<IDComponent>().ID);
	}

	void PythonScriptEngine::OnCollision2DEnd(UUID sceneID, UUID entityID)
	{
		PR_PROFILE_FUNCTION();
		if (m_EntityInstanceMap.find(sceneID) == m_EntityInstanceMap.end())
			return;
		auto& entityMap = m_EntityInstanceMap.at(sceneID);
		if (entityMap.find(entityID) == entityMap.end())
			return;
		for (auto& [moduleName, script] : entityMap.at(entityID).Scripts)
		{
			if (script && script->Object.IsValid())
				script->Object.Invoke("OnCollision2DEnd", 5.0f);
		}
	}

	void PythonScriptEngine::OnCollisionBegin(Entity entity)
	{
		OnCollisionBegin(entity.GetSceneUUID(), entity.GetComponent<IDComponent>().ID);
	}

	void PythonScriptEngine::OnCollisionBegin(UUID sceneID, UUID entityID)
	{
		PR_PROFILE_FUNCTION();
		if (m_EntityInstanceMap.find(sceneID) == m_EntityInstanceMap.end())
			return;
		auto& entityMap = m_EntityInstanceMap.at(sceneID);
		if (entityMap.find(entityID) == entityMap.end())
			return;
		for (auto& [moduleName, script] : entityMap.at(entityID).Scripts)
		{
			if (script && script->Object.IsValid())
				script->Object.Invoke("OnCollisionBegin", 5.0f);
		}
	}

	void PythonScriptEngine::OnCollisionEnd(Entity entity)
	{
		OnCollisionEnd(entity.GetSceneUUID(), entity.GetComponent<IDComponent>().ID);
	}

	void PythonScriptEngine::OnCollisionEnd(UUID sceneID, UUID entityID)
	{
		PR_PROFILE_FUNCTION();
		if (m_EntityInstanceMap.find(sceneID) == m_EntityInstanceMap.end())
			return;
		auto& entityMap = m_EntityInstanceMap.at(sceneID);
		if (entityMap.find(entityID) == entityMap.end())
			return;
		for (auto& [moduleName, script] : entityMap.at(entityID).Scripts)
		{
			if (script && script->Object.IsValid())
				script->Object.Invoke("OnCollisionEnd", 5.0f);
		}
	}
#pragma endregion

	void PythonScriptEngine::OnImGuiRender()
	{
		if (ImGui::Begin("Python Script Engine"))
		{
			ImGui::Text("Initialized: %s", m_Initialized ? "Yes" : "No");
			ImGui::Text("Entity maps: %zu", m_EntityInstanceMap.size());
		}
		ImGui::End();
	}

#pragma region Field access
	uint32_t PythonScriptEngine::GetFieldCount(UUID sceneID, UUID entityID, const std::string& moduleName)
	{
		if (m_EntityInstanceMap.find(sceneID) == m_EntityInstanceMap.end())
			return 0;
		auto& entityMap = m_EntityInstanceMap.at(sceneID);
		if (entityMap.find(entityID) == entityMap.end())
			return 0;
		auto& entityData = entityMap.at(entityID);
		if (entityData.ModuleFieldMap.find(moduleName) == entityData.ModuleFieldMap.end())
			return 0;
		return (uint32_t)entityData.ModuleFieldMap.at(moduleName).size();
	}

	bool PythonScriptEngine::GetFieldInfo(UUID sceneID, UUID entityID, const std::string& moduleName, uint32_t index, PublicFieldInfo& outInfo)
	{
		if (m_EntityInstanceMap.find(sceneID) == m_EntityInstanceMap.end())
			return false;
		auto& entityMap = m_EntityInstanceMap.at(sceneID);
		if (entityMap.find(entityID) == entityMap.end())
			return false;
		auto& entityData = entityMap.at(entityID);
		if (entityData.ModuleFieldMap.find(moduleName) == entityData.ModuleFieldMap.end())
			return false;
		auto& fields = entityData.ModuleFieldMap.at(moduleName);
		if (index >= fields.size())
			return false;
		auto it = fields.begin();
		std::advance(it, index);
		outInfo.Name = it->first;
		outInfo.Type = it->second->GetType();
		return true;
	}

	PublicField* PythonScriptEngine::GetField(UUID sceneID, UUID entityID, const std::string& moduleName, const std::string& fieldName)
	{
		if (m_EntityInstanceMap.find(sceneID) == m_EntityInstanceMap.end())
			return nullptr;
		auto& entityMap = m_EntityInstanceMap.at(sceneID);
		if (entityMap.find(entityID) == entityMap.end())
			return nullptr;
		auto& entityData = entityMap.at(entityID);
		if (entityData.ModuleFieldMap.find(moduleName) == entityData.ModuleFieldMap.end())
			return nullptr;
		auto& fields = entityData.ModuleFieldMap.at(moduleName);
		auto it = fields.find(fieldName);
		if (it == fields.end())
			return nullptr;
		return it->second.get();
	}

	PublicField* PythonScriptEngine::GetOrCreateField(UUID sceneID, UUID entityID, const std::string& moduleName, const std::string& fieldName, FieldType type)
	{
		auto& entityData = GetEntityInstanceData(sceneID, entityID);
		auto scriptIt = entityData.Scripts.find(moduleName);
		if (scriptIt == entityData.Scripts.end())
		{
			auto script = std::make_unique<PythonEntityScriptInstance>();
			scriptIt = entityData.Scripts.emplace(moduleName, std::move(script)).first;
		}

		auto& fields = entityData.ModuleFieldMap[moduleName];
		auto it = fields.find(fieldName);
		if (it != fields.end())
			return it->second.get();

		// 创建字段，运行时对象在 InstantiateEntityClass 时通过 SetScriptObject 设置
		auto field = std::make_unique<PythonPublicField>(fieldName, type, nullptr);
		auto* ptr = field.get();
		fields[fieldName] = std::move(field);
		return ptr;
	}
#pragma endregion

	PythonEntityInstanceData& PythonScriptEngine::GetEntityInstanceData(UUID sceneID, UUID entityID)
	{
		PR_CORE_ASSERT(m_EntityInstanceMap.find(sceneID) != m_EntityInstanceMap.end(), "场景 ID 无效！");
		auto& entityIDMap = m_EntityInstanceMap.at(sceneID);
		PR_CORE_ASSERT(entityIDMap.find(entityID) != entityIDMap.end(), "实体 ID 无效！");
		return entityIDMap.at(entityID);
	}

} // namespace Prism
