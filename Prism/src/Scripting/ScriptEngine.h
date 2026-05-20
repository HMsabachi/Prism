#pragma once
#include "Prism/Scene/Components.h"
#include "Prism/Scene/Entity.h"
#include "Scripting/ScriptTypes.h"

namespace Prism
{
	enum class FieldType
	{
		None = 0, Float, Int, UnsignedInt, String, Vec2, Vec3, Vec4
	};

	struct PublicFieldInfo
    {
        std::string Name;
        FieldType Type;
    };
	class PublicField;
	const char* FieldTypeToString(FieldType type);

	class PRISM_API ScriptEngine
	{
	public:
		virtual ~ScriptEngine() = default;

		// Lifecycle
		virtual bool Initialize() = 0;
		virtual void Shutdown() = 0;

		// Assembly/module loading
		virtual bool LoadEngineAssembly(const std::string& path) = 0;
		virtual bool LoadAppAssembly(const std::string& path) = 0;
		virtual void ReloadAssembly(const std::string& path) = 0;

		// Scene context
		virtual void SetSceneContext(const Ref<Scene>& scene) = 0;
		virtual const Ref<Scene>& GetCurrentSceneContext() = 0;
		virtual void OnSceneDestruct(UUID sceneID) = 0;
		virtual void CopyEntityScriptData(UUID dst, UUID src) = 0;
		virtual bool HasEntityScriptData(UUID sceneID) = 0;

		// Entity lifecycle (need explicit moduleName for multi-script support)
		virtual void InitScriptEntity(Entity entity, const std::string& moduleName) = 0;
		virtual void ShutdownScriptEntity(Entity entity, const std::string& moduleName) = 0;
		virtual void InstantiateEntityClass(Entity entity, const std::string& moduleName) = 0;
		virtual bool ModuleExists(const std::string& moduleName) = 0;

		virtual void OnCreateEntity(Entity entity, const std::string& moduleName) = 0;
		virtual void OnCreateEntity(UUID sceneID, UUID entityID, const std::string& moduleName) = 0;
		virtual void OnUpdateEntity(UUID sceneID, UUID entityID, const std::string& moduleName, float ts) = 0;
		virtual void OnFixedUpdateEntity(UUID sceneID, UUID entityID, const std::string& moduleName) = 0;
		virtual void OnScriptComponentDestroyed(UUID sceneID, UUID entityID) = 0;

		// Collision callbacks (iterates all scripts on entity internally)
		virtual void OnCollision2DBegin(Entity entity) = 0;
		virtual void OnCollision2DBegin(UUID sceneID, UUID entityID) = 0;
		virtual void OnCollision2DEnd(Entity entity) = 0;
		virtual void OnCollision2DEnd(UUID sceneID, UUID entityID) = 0;
		virtual void OnCollisionBegin(Entity entity) = 0;
		virtual void OnCollisionBegin(UUID sceneID, UUID entityID) = 0;
		virtual void OnCollisionEnd(Entity entity) = 0;
		virtual void OnCollisionEnd(UUID sceneID, UUID entityID) = 0;

		// Debug
		virtual void OnImGuiRender() = 0;

		// 字段访问（已有 moduleName 参数，保持不变）
		virtual uint32_t GetFieldCount(UUID sceneID, UUID entityID, const std::string& moduleName) = 0;
		virtual bool GetFieldInfo(UUID sceneID, UUID entityID, const std::string& moduleName, uint32_t index, PublicFieldInfo& outInfo) = 0;
		virtual PublicField* GetField(UUID sceneID, UUID entityID, const std::string& moduleName, const std::string& fieldName) = 0;
		virtual PublicField* GetOrCreateField(UUID sceneID, UUID entityID, const std::string& moduleName, const std::string& fieldName, FieldType type) = 0;

		// Entity 便捷重载
		uint32_t GetFieldCount(Entity entity, const std::string& moduleName)
		{
			return GetFieldCount(entity.GetSceneUUID(), entity.GetUUID(), moduleName);
		}
		bool GetFieldInfo(Entity entity, const std::string& moduleName, uint32_t index, PublicFieldInfo& outInfo)
		{
			return GetFieldInfo(entity.GetSceneUUID(), entity.GetUUID(), moduleName, index, outInfo);
		}
		PublicField* GetField(Entity entity, const std::string& moduleName, const std::string& fieldName)
		{
			return GetField(entity.GetSceneUUID(), entity.GetUUID(), moduleName, fieldName);
		}
		PublicField* GetOrCreateField(Entity entity, const std::string& moduleName, const std::string& fieldName, FieldType type)
		{
			return GetOrCreateField(entity.GetSceneUUID(), entity.GetUUID(), moduleName, fieldName, type);
		}
	};
}
