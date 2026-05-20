#pragma once
#include "Scripting/ScriptEngine.h"
#include "Scripting/PublicField.h"

namespace Rolky
{
	struct HostInstance;
	struct AssemblyLoadContext;
	struct ManagedAssembly;
	class FieldInfo;
	class ManagedObject;
	class Type;
}
#include <Rolky/HostInstance.hpp>

namespace Prism
{
	struct EntityInstance
	{
		Rolky::Type* ScriptClass = nullptr;
		std::unique_ptr<Rolky::ManagedObject> Object;
		Scene* SceneInstance = nullptr;
		uint32_t HasMethods;
	};

	using ScriptModuleFieldMap = std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<PublicField>>>;

	struct EntityInstanceData
	{
		EntityInstance Instance;
		ScriptModuleFieldMap ModuleFieldMap;
	};

	using EntityInstanceMap = std::unordered_map<UUID, std::unordered_map<UUID, EntityInstanceData>>;

	class CSharpScriptEngine : public ScriptEngine
	{
	public:
		CSharpScriptEngine();
		~CSharpScriptEngine() override;

		// ScriptEngine interface
		bool Initialize() override;
		void Shutdown() override;
		bool LoadEngineAssembly(const std::string& path) override;
		bool LoadAppAssembly(const std::string& path) override;
		void ReloadAssembly(const std::string& path) override;
		void SetSceneContext(const Ref<Scene>& scene) override;
		const Ref<Scene>& GetCurrentSceneContext() override;
		void OnSceneDestruct(UUID sceneID) override;
		void CopyEntityScriptData(UUID dst, UUID src) override;
		bool HasEntityScriptData(UUID sceneID) override;
		void InitScriptEntity(Entity entity) override;
		void ShutdownScriptEntity(Entity entity, const std::string& moduleName) override;
		void InstantiateEntityClass(Entity entity) override;
		bool ModuleExists(const std::string& moduleName) override;
		void OnCreateEntity(Entity entity) override;
		void OnCreateEntity(UUID sceneID, UUID entityID) override;
		void OnUpdateEntity(UUID sceneID, UUID entityID, float ts) override;
		void OnFixedUpdateEntity(UUID sceneID, UUID entityID) override;
		void OnScriptComponentDestroyed(UUID sceneID, UUID entityID) override;
		void OnCollision2DBegin(Entity entity) override;
		void OnCollision2DBegin(UUID sceneID, UUID entityID) override;
		void OnCollision2DEnd(Entity entity) override;
		void OnCollision2DEnd(UUID sceneID, UUID entityID) override;
		void OnCollisionBegin(Entity entity) override;
		void OnCollisionBegin(UUID sceneID, UUID entityID) override;
		void OnCollisionEnd(Entity entity) override;
		void OnCollisionEnd(UUID sceneID, UUID entityID) override;
		void OnImGuiRender() override;

		// Field access
		uint32_t GetFieldCount(UUID sceneID, UUID entityID, const std::string& moduleName) override;
		bool GetFieldInfo(UUID sceneID, UUID entityID, const std::string& moduleName, uint32_t index, PublicFieldInfo& outInfo) override;
		PublicField* GetField(UUID sceneID, UUID entityID, const std::string& moduleName, const std::string& fieldName) override;
		PublicField* GetOrCreateField(UUID sceneID, UUID entityID, const std::string& moduleName, const std::string& fieldName, FieldType type) override;

		// C#-specific accessors
		EntityInstanceMap& GetEntityInstanceMap();
		EntityInstanceData& GetEntityInstanceData(UUID sceneID, UUID entityID);
		Rolky::ManagedAssembly& GetEngineAssembly();

	private:
		std::unique_ptr<Rolky::HostInstance> m_Host;
		std::unique_ptr<Rolky::AssemblyLoadContext> m_LoadContext;

		EntityInstanceMap m_EntityInstanceMap;
		Ref<Scene> m_SceneContext;

		std::unordered_map<std::string, Rolky::Type> m_EntityClassMap;
		Rolky::ManagedAssembly m_EngineAssembly;
		Rolky::ManagedAssembly m_AppAssembly;
	};
}
#include <Rolky/HostInstance.hpp>
