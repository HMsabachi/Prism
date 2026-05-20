#include "prpch.h"
#include "PythonScriptEngine.h"

// Ensure Python.h is included after prpch.h (which includes Windows.h).
// Undefine problematic Windows macros that conflict with Python C API enums.
#ifdef ERROR
#undef ERROR
#endif
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include <Python.h>

#include <filesystem>
#include <imgui.h>


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

		// Initialize CPython
		Py_Initialize();

		if (!Py_IsInitialized())
		{
			PR_CORE_ERROR("[Python] Failed to initialize Python interpreter!");
			return false;
		}

		// Add script search paths
		PyRun_SimpleString(
			"import sys\n"
			"import os\n"
			"# Add Assets/scripts for user script discovery\n"
			"scripts_path = os.path.abspath('Assets/scripts')\n"
			"if scripts_path not in sys.path:\n"
			"    sys.path.insert(0, scripts_path)\n"
			"print(f'[Python] Added {scripts_path} to sys.path for script discovery')\n"
		);

		m_Initialized = true;
		PR_CORE_TRACE("[Python] Python {0}.{1}.{2} initialized",
			PY_MAJOR_VERSION, PY_MINOR_VERSION, PY_MICRO_VERSION);
		return true;
	}

	void PythonScriptEngine::Shutdown()
	{
		PR_PROFILE_FUNCTION();

		if (!m_Initialized)
			return;

		for (auto& [sceneID, entityMap] : m_EntityInstanceMap)
		{
			for (auto& [entityID, entityData] : entityMap)
			{
				for (auto& [moduleName, script] : entityData.Scripts)
				{
					if (script)
					{
						Py_XDECREF(script->Instance);
						Py_XDECREF(script->Class);
						Py_XDECREF(script->Module);
					}
				}
			}
		}
		m_EntityInstanceMap.clear();

		Py_Finalize();
		m_Initialized = false;
		PR_CORE_INFO("[Python] Python interpreter shut down");
	}

	bool PythonScriptEngine::LoadEngineAssembly(const std::string& path)
	{
		PR_CORE_INFO("[Python] LoadEngineAssembly({0}) — no-op for Python engine", path);
		return true;
	}

	bool PythonScriptEngine::LoadAppAssembly(const std::string& path)
	{
		PR_CORE_INFO("[Python] LoadAppAssembly({0}) — scripts dir already in sys.path", path);
		return true;
	}

	void PythonScriptEngine::ReloadAssembly(const std::string& path)
	{
		PR_PROFILE_FUNCTION();
		PR_CORE_INFO("[Python] ReloadAssembly({0}) — not yet implemented", path);
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
		if (m_EntityInstanceMap.find(sceneID) != m_EntityInstanceMap.end())
		{
			auto& entityMap = m_EntityInstanceMap.at(sceneID);
			for (auto& [entityID, entityData] : entityMap)
			{
				for (auto& [moduleName, script] : entityData.Scripts)
				{
					if (script)
					{
						Py_XDECREF(script->Instance);
						Py_XDECREF(script->Class);
						Py_XDECREF(script->Module);
					}
				}
			}
			m_EntityInstanceMap.erase(sceneID);
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
		PR_CORE_INFO("[Python] InitScriptEntity({0}) — stub", moduleName);
	}

	void PythonScriptEngine::ShutdownScriptEntity(Entity entity, const std::string& moduleName)
	{
		PythonEntityInstanceData& entityData = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID());
		if (entityData.Scripts.find(moduleName) != entityData.Scripts.end())
		{
			auto& script = entityData.Scripts.at(moduleName);
			if (script)
			{
				Py_XDECREF(script->Instance);
				Py_XDECREF(script->Class);
				Py_XDECREF(script->Module);
			}
			entityData.Scripts.erase(moduleName);
		}
		if (entityData.ModuleFieldMap.find(moduleName) != entityData.ModuleFieldMap.end())
			entityData.ModuleFieldMap.erase(moduleName);
	}

	void PythonScriptEngine::InstantiateEntityClass(Entity entity, const std::string& moduleName)
	{
		PR_PROFILE_FUNCTION();
		PR_CORE_INFO("[Python] InstantiateEntityClass({0}) — stub", moduleName);
	}

	bool PythonScriptEngine::ModuleExists(const std::string& moduleName)
	{
		PR_PROFILE_FUNCTION();

		PyGILState_STATE gil = PyGILState_Ensure();
		PyObject* mod = PyImport_ImportModule(moduleName.c_str());
		bool exists = (mod != nullptr);
		Py_XDECREF(mod);
		PyGILState_Release(gil);

		return exists;
	}

	void PythonScriptEngine::OnCreateEntity(Entity entity, const std::string& moduleName)
	{
		OnCreateEntity(entity.GetSceneUUID(), entity.GetComponent<IDComponent>().ID, moduleName);
	}

	void PythonScriptEngine::OnCreateEntity(UUID sceneID, UUID entityID, const std::string& moduleName)
	{
		PR_PROFILE_FUNCTION();
		PR_CORE_INFO("[Python] OnCreateEntity({0}) — stub", moduleName);
	}

	void PythonScriptEngine::OnUpdateEntity(UUID sceneID, UUID entityID, const std::string& moduleName, float ts)
	{
		PR_PROFILE_FUNCTION();
	}

	void PythonScriptEngine::OnFixedUpdateEntity(UUID sceneID, UUID entityID, const std::string& moduleName)
	{
		PR_PROFILE_FUNCTION();
	}

	void PythonScriptEngine::OnScriptComponentDestroyed(UUID sceneID, UUID entityID)
	{
		if (m_EntityInstanceMap.find(sceneID) != m_EntityInstanceMap.end())
		{
			auto& entityMap = m_EntityInstanceMap.at(sceneID);
			if (entityMap.find(entityID) != entityMap.end())
			{
				auto& entityData = entityMap.at(entityID);
				for (auto& [moduleName, script] : entityData.Scripts)
				{
					if (script)
					{
						Py_XDECREF(script->Instance);
						Py_XDECREF(script->Class);
						Py_XDECREF(script->Module);
					}
				}
				entityMap.erase(entityID);
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
	}

	void PythonScriptEngine::OnCollision2DEnd(Entity entity)
	{
		OnCollision2DEnd(entity.GetSceneUUID(), entity.GetComponent<IDComponent>().ID);
	}

	void PythonScriptEngine::OnCollision2DEnd(UUID sceneID, UUID entityID)
	{
		PR_PROFILE_FUNCTION();
	}

	void PythonScriptEngine::OnCollisionBegin(Entity entity)
	{
		OnCollisionBegin(entity.GetSceneUUID(), entity.GetComponent<IDComponent>().ID);
	}

	void PythonScriptEngine::OnCollisionBegin(UUID sceneID, UUID entityID)
	{
		PR_PROFILE_FUNCTION();
	}

	void PythonScriptEngine::OnCollisionEnd(Entity entity)
	{
		OnCollisionEnd(entity.GetSceneUUID(), entity.GetComponent<IDComponent>().ID);
	}

	void PythonScriptEngine::OnCollisionEnd(UUID sceneID, UUID entityID)
	{
		PR_PROFILE_FUNCTION();
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

		// TODO: Create PythonPublicField when that class exists
		return nullptr;
	}
#pragma endregion

	PythonEntityInstanceData& PythonScriptEngine::GetEntityInstanceData(UUID sceneID, UUID entityID)
	{
		PR_CORE_ASSERT(m_EntityInstanceMap.find(sceneID) != m_EntityInstanceMap.end(), "Invalid scene ID!");
		auto& entityIDMap = m_EntityInstanceMap.at(sceneID);
		PR_CORE_ASSERT(entityIDMap.find(entityID) != entityIDMap.end(), "Invalid entity ID!");
		return entityIDMap.at(entityID);
	}
}