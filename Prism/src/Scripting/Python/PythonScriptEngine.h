#pragma once
#include "Scripting/ScriptEngine.h"
#include "Scripting/PublicField.h"
#include <unordered_map>
#include <memory>

// Forward declare Python types to avoid <Python.h> / <Windows.h> macro conflicts
// when this header is included from files with precompiled headers
struct _object;
typedef _object PyObject;

namespace Prism
{
	struct PythonEntityScriptInstance
	{
		PyObject* Module = nullptr;
		PyObject* Class = nullptr;
		PyObject* Instance = nullptr;
		uint32_t HasMethods = 0;
	};

	using PythonScriptModuleFieldMap = std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<PublicField>>>;

	struct PythonEntityInstanceData
	{
		std::unordered_map<std::string, std::unique_ptr<PythonEntityScriptInstance>> Scripts;
		PythonScriptModuleFieldMap ModuleFieldMap;
	};

	using PythonEntityInstanceMap = std::unordered_map<UUID, std::unordered_map<UUID, PythonEntityInstanceData>>;

	class PythonScriptEngine : public ScriptEngine
	{
	public:
		PythonScriptEngine() = default;
		~PythonScriptEngine() override;

		// Lifecycle
		bool Initialize() override;
		void Shutdown() override;

		// Module loading
		bool LoadEngineAssembly(const std::string& path) override;
		bool LoadAppAssembly(const std::string& path) override;
		void ReloadAssembly(const std::string& path) override;

		// Scene context
		void SetSceneContext(const Ref<Scene>& scene) override;
		const Ref<Scene>& GetCurrentSceneContext() override;
		void OnSceneDestruct(UUID sceneID) override;
		void CopyEntityScriptData(UUID dst, UUID src) override;
		bool HasEntityScriptData(UUID sceneID) override;

		// Entity lifecycle
		void InitScriptEntity(Entity entity, const std::string& moduleName) override;
		void ShutdownScriptEntity(Entity entity, const std::string& moduleName) override;
		void InstantiateEntityClass(Entity entity, const std::string& moduleName) override;
		bool ModuleExists(const std::string& moduleName) override;
		void OnCreateEntity(Entity entity, const std::string& moduleName) override;
		void OnCreateEntity(UUID sceneID, UUID entityID, const std::string& moduleName) override;
		void OnUpdateEntity(UUID sceneID, UUID entityID, const std::string& moduleName, float ts) override;
		void OnFixedUpdateEntity(UUID sceneID, UUID entityID, const std::string& moduleName) override;
		void OnScriptComponentDestroyed(UUID sceneID, UUID entityID) override;

		// Collision callbacks
		void OnCollision2DBegin(Entity entity) override;
		void OnCollision2DBegin(UUID sceneID, UUID entityID) override;
		void OnCollision2DEnd(Entity entity) override;
		void OnCollision2DEnd(UUID sceneID, UUID entityID) override;
		void OnCollisionBegin(Entity entity) override;
		void OnCollisionBegin(UUID sceneID, UUID entityID) override;
		void OnCollisionEnd(Entity entity) override;
		void OnCollisionEnd(UUID sceneID, UUID entityID) override;

		// Debug
		void OnImGuiRender() override;

		// Field access
		uint32_t GetFieldCount(UUID sceneID, UUID entityID, const std::string& moduleName) override;
		bool GetFieldInfo(UUID sceneID, UUID entityID, const std::string& moduleName, uint32_t index, PublicFieldInfo& outInfo) override;
		PublicField* GetField(UUID sceneID, UUID entityID, const std::string& moduleName, const std::string& fieldName) override;
		PublicField* GetOrCreateField(UUID sceneID, UUID entityID, const std::string& moduleName, const std::string& fieldName, FieldType type) override;

		// Internal helpers (for wrappers)
		PythonEntityInstanceData& GetEntityInstanceData(UUID sceneID, UUID entityID);

	private:
		PythonEntityInstanceMap m_EntityInstanceMap;
		Ref<Scene> m_SceneContext;
		bool m_Initialized = false;
	};
}