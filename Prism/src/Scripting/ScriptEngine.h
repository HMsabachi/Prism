#pragma once
#include "Prism/Core/UUID.h"
#include "Scripting/ScriptTypes.h"
#include <string>
#include <cstdint>
#include <memory>

namespace Prism {

	class Scene;
	class Entity;
	template<typename T> class Ref;
	struct ScriptGroup;
	class PublicField;

	enum class FieldType
	{
		None = 0, Float, Int, UnsignedInt, String, Vec2, Vec3, Vec4
	};

	struct PublicFieldInfo
	{
		std::string Name;
		FieldType Type;
	};

	const char* FieldTypeToString(FieldType type);

	// 简化的 ScriptEngine 接口 — 只做运行时服务，不管理实体数据
	class PRISM_API ScriptEngine
	{
	public:
		virtual ~ScriptEngine() = default;

		// Lifecycle
		virtual bool Initialize() = 0;
		virtual void Shutdown() = 0;

		// Module loading
		virtual bool LoadEngineAssembly(const std::string& path) = 0;
		virtual bool LoadAppAssembly(const std::string& path) = 0;
		virtual void ReloadAssembly(const std::string& path) = 0;

		// Scene context
		virtual void SetSceneContext(const Ref<Scene>& scene) = 0;
		virtual const Ref<Scene>& GetCurrentSceneContext() = 0;

		// Entity lifecycle — 操作在 Scene 持有的 ScriptGroup 上
		virtual void InitScriptEntity(Entity& entity, ScriptGroup& group) = 0;
		virtual void ShutdownScriptEntity(ScriptGroup& group) = 0;
		virtual void InstantiateEntityClass(ScriptGroup& group) = 0;

		// Module discovery
		virtual bool ModuleExists(const std::string& moduleName) = 0;

		// Debug
		virtual void OnImGuiRender() = 0;
	};

} // namespace Prism
