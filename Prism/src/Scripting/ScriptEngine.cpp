#include "prpch.h"
#include "ScriptEngine.h"
#include "ScriptEngineRegistry.h"
#include "Native/String.h"
#include <filesystem>
#include <imgui.h>

#include <Rolky/HostInstance.hpp>

namespace Prism
{

	static void RolkyMessageCallback(std::string_view message, Rolky::MessageLevel level)
	{
		if (level & Rolky::MessageLevel::Error) PR_CORE_ERROR("[Rolky] {0}", message);
		else if (level & Rolky::MessageLevel::Warning) PR_CORE_WARN("[Rolky] {0}", message);
		else if (level & Rolky::MessageLevel::Info) PR_CORE_INFO("[Rolky] {0}", message);
		else PR_CORE_TRACE("[Rolky] {0}", message);
	}
	static void RolkyExceptionCallback(std::string_view message)
	{
		PR_CORE_ERROR("[Rolky] {0}", message);
	}

    static enum ScriptMethods
    {
        Script_None = 0,
        Script_OnCreate = BIT(0),
        Script_OnUpdate = BIT(1),
        Script_OnFixedUpdate = BIT(2),
    };

	static std::vector<Rolky::ManagedObject> AllObjects;
	static std::unordered_map<std::string, Rolky::Type> s_EntityClassMap;
	static EntityInstanceMap s_EntityInstanceMap;
	static Ref<Scene> s_SceneContext;

	void* ScriptEngine::s_HostHandle = nullptr;
	void* ScriptEngine::s_AssemblyLoadContext = nullptr;

	
	std::unique_ptr<Rolky::HostInstance> ScriptEngine::m_Host;
	std::unique_ptr<Rolky::AssemblyLoadContext> ScriptEngine::m_LoadContext;
	static Rolky::ManagedAssembly s_EngineAssembly;
	static Rolky::ManagedAssembly s_AppAssembly;

#pragma region PublicField
	static std::unordered_map<std::string, FieldType> s_FieldTypeMap =
	{
		{"System.Single", FieldType::Float },
		{"System.Int32", FieldType::Int },
		{"System.UInt32", FieldType::UnsignedInt },
		{"System.String", FieldType::String },
		{"Prism.Vector2", FieldType::Vec2 },
		{"Prism.Vector3", FieldType::Vec3 },
		{"Prism.Vector4", FieldType::Vec4 },
	};
	static FieldType GetPrismFieldType(const Rolky::Type& type)
	{
		if (s_FieldTypeMap.find(type.GetFullName()) != s_FieldTypeMap.end())
		{
			return s_FieldTypeMap[type.GetFullName()];
		}
		return FieldType::None;
	}
	static uint32_t GetFieldSize(FieldType type)
	{
		switch (type)
		{
		case FieldType::Float:       return 4;
		case FieldType::Int:         return 4;
		case FieldType::UnsignedInt: return 4;
		case FieldType::String:   return 8; // TODO
		case FieldType::Vec2:        return 4 * 2;
		case FieldType::Vec3:        return 4 * 3;
		case FieldType::Vec4:        return 4 * 4;
		}
		PR_CORE_ASSERT(false, "Unknown field type!");
		return 0;
	}
	const char* FieldTypeToString(FieldType type)
	{
		switch (type)
		{
		case FieldType::Float:       return "Float";
		case FieldType::Int:         return "Int";
		case FieldType::UnsignedInt: return "UnsignedInt";
		case FieldType::String:      return "String";
		case FieldType::Vec2:        return "Vec2";
		case FieldType::Vec3:        return "Vec3";
		case FieldType::Vec4:        return "Vec4";
		}
		return "Unknown";
	}
	PublicField::PublicField(const std::string& name, FieldType type)
		: Name(name), Type(type), m_EntityInstance(nullptr)
	{
		m_StoredValueBuffer = AllocateBuffer(type);
	}
	PublicField::PublicField(PublicField&& other)
	{
		Name = std::move(other.Name);
		Type = other.Type;
		m_EntityInstance = other.m_EntityInstance;
		m_StoredValueBuffer = other.m_StoredValueBuffer;

		other.m_EntityInstance = nullptr;
		other.m_StoredValueBuffer = nullptr;
	}
	PublicField::~PublicField()
	{
		delete[] m_StoredValueBuffer;
	}
	void PublicField::CopyStoredValueToRuntime()
	{
		PR_CORE_ASSERT(m_EntityInstance && m_EntityInstance->Object && m_EntityInstance->Object->IsValid());
		m_EntityInstance->Object->SetFieldValueRaw(Name, m_StoredValueBuffer);
	}
	bool PublicField::IsRuntimeAvailable() const
	{
		return m_EntityInstance && m_EntityInstance->Object && m_EntityInstance->Object->IsValid();
	}
	void PublicField::SetStoredValueRaw(void* src)
	{
		uint32_t size = GetFieldSize(Type);
		memcpy(m_StoredValueBuffer, src, size);
	}
	uint8_t* PublicField::AllocateBuffer(FieldType type)
	{
		uint32_t size = GetFieldSize(type);
		uint8_t* buffer = new uint8_t[size];
		memset(buffer, 0, size);
		return buffer;
	}
	void PublicField::SetStoredValue_Internal(void* value) const
	{
		uint32_t size = GetFieldSize(Type);
		memcpy(m_StoredValueBuffer, value, size);
	}
	void PublicField::GetStoredValue_Internal(void* outValue) const
	{
		uint32_t size = GetFieldSize(Type);
		memcpy(outValue, m_StoredValueBuffer, size);
	}
	void PublicField::SetRuntimeValue_Internal(void* value) const
	{
		PR_CORE_ASSERT(m_EntityInstance && m_EntityInstance->Object && m_EntityInstance->Object->IsValid());
		m_EntityInstance->Object->SetFieldValueRaw(Name, value);
	}
	void PublicField::GetRuntimeValue_Internal(void* outValue) const
	{
		PR_CORE_ASSERT(m_EntityInstance && m_EntityInstance->Object && m_EntityInstance->Object->IsValid());
		m_EntityInstance->Object->GetFieldValueRaw(Name, outValue);
	}

	// Debug
	void ScriptEngine::OnImGuiRender()
	{
		ImGui::Begin("Script Engine Debug");
		for (auto& [sceneID, entityMap] : s_EntityInstanceMap)
		{
			bool opened = ImGui::TreeNode((void*)(uint64_t)sceneID, "Scene (%llx)", sceneID);
			if (opened)
			{
				Ref<Scene> scene = Scene::GetScene(sceneID);
				for (auto& [entityID, entityInstanceData] : entityMap)
				{
					Entity entity = scene->GetScene(sceneID)->GetEntityMap().at(entityID);
					std::string entityName = "Unnamed Entity";
					if (entity.HasComponent<TagComponent>())
						entityName = entity.GetComponent<TagComponent>().Tag;
					opened = ImGui::TreeNode((void*)(uint64_t)entityID, "%s (%llx)", entityName.c_str(), entityID);
					if (opened)
					{
						for (auto& [moduleName, fieldMap] : entityInstanceData.ModuleFieldMap)
						{
							opened = ImGui::TreeNode(moduleName.c_str());
							if (opened)
							{
								for (auto& [fieldName, field] : fieldMap)
								{

									opened = ImGui::TreeNodeEx((void*)&field, ImGuiTreeNodeFlags_Leaf, fieldName.c_str());
									if (opened)
									{

										ImGui::TreePop();
									}
								}
								ImGui::TreePop();
							}
						}
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}
		}
		ImGui::End();
	}
#pragma endregion

	bool ScriptEngine::Initialize()
	{
		PR_PROFILE_FUNCTION();

		Rolky::HostSettings setting;
		setting.RolkyDirectory = "Assets/scripts";
		setting.MessageCallback = RolkyMessageCallback;
		setting.ExceptionCallback = RolkyExceptionCallback;
		m_Host = std::make_unique<Rolky::HostInstance>();
		m_Host->Initialize(setting);
		m_LoadContext = std::make_unique<Rolky::AssemblyLoadContext>(std::move(m_Host->CreateAssemblyLoadContext("PrismLoadContext")));
		return true;
	}
	void ScriptEngine::Shutdown()
	{
		m_Host->UnloadAssemblyLoadContext(*m_LoadContext);
		m_Host->Shutdown();
		s_SceneContext = nullptr;
		s_EntityInstanceMap.clear();
	}
	bool ScriptEngine::LoadEngineAssembly(const std::string& assemblyPath)
	{
		PR_PROFILE_FUNCTION();
		auto path = std::filesystem::absolute(assemblyPath).string();
		s_EngineAssembly = m_LoadContext->LoadAssembly(path);
		ScriptEngineRegistry::RegisterAll();
		auto initClass = s_EngineAssembly.GetType("Prism.Core");
		initClass.InvokeStaticMethod("Init");
		return true;
	}
	bool ScriptEngine::LoadAppAssembly(const std::string& assemblyPath)
	{
		PR_PROFILE_FUNCTION();
		auto path = std::filesystem::absolute(assemblyPath).string();
		s_AppAssembly = m_LoadContext->LoadAssembly(path);
		return true;
	}

	void ScriptEngine::ReloadAssembly(const std::string& assemblyPath)
	{
		PR_PROFILE_FUNCTION();
		auto path = std::filesystem::absolute(assemblyPath).string();
		s_AppAssembly = m_LoadContext->LoadAssembly(path);
		if (s_EntityInstanceMap.size())
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			PR_CORE_ASSERT(scene, "No active scene!");
			if (s_EntityInstanceMap.find(scene->GetUUID()) != s_EntityInstanceMap.end())
			{
				auto& entityMap = s_EntityInstanceMap.at(scene->GetUUID());
				for (auto& [entityID, entityInstanceData] : entityMap)
				{
					const auto& entityMap = scene->GetEntityMap();
					PR_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
					InitScriptEntity(entityMap.at(entityID));
				}
			}
		}
	}

	void ScriptEngine::OnSceneDestruct(UUID sceneID)
	{
		if (s_EntityInstanceMap.find(sceneID) != s_EntityInstanceMap.end())
		{
            for (auto& [entityID, entityInstanceData] : s_EntityInstanceMap.at(sceneID))
			{
				//entityInstanceData.Instance.Object->InvokeMethod("OnDestroy");
				if (entityInstanceData.Instance.Object)
					entityInstanceData.Instance.Object->Destroy();
			}
			s_EntityInstanceMap.at(sceneID).clear();
			s_EntityInstanceMap.erase(sceneID);
		}
	}
	void ScriptEngine::SetSceneContext(const Ref<Scene>& scene)
	{
		s_SceneContext = scene;
	}
	const Ref<Scene>& ScriptEngine::GetCurrentSceneContext()
	{
		return s_SceneContext;
	}

	void ScriptEngine::CopyEntityScriptData(UUID dst, UUID src)
	{
		PR_CORE_ASSERT(s_EntityInstanceMap.find(dst) != s_EntityInstanceMap.end());
		PR_CORE_ASSERT(s_EntityInstanceMap.find(src) != s_EntityInstanceMap.end());

		auto& dstEntityMap = s_EntityInstanceMap.at(dst);
		auto& srcEntityMap = s_EntityInstanceMap.at(src);

		for (auto& [entityID, entityInstanceData] : srcEntityMap)
		{
			for (auto& [moduleName, srcFieldMap] : srcEntityMap[entityID].ModuleFieldMap)
			{
				auto& dstModuleFieldMap = dstEntityMap[entityID].ModuleFieldMap;
				for (auto& [fieldName, field] : srcFieldMap)
				{
					PR_CORE_ASSERT(dstModuleFieldMap.find(moduleName) != dstModuleFieldMap.end());
					auto& fieldMap = dstModuleFieldMap.at(moduleName);
					if(fieldMap.find(fieldName) == fieldMap.end())
						continue;
					fieldMap.at(fieldName).SetStoredValueRaw(field.m_StoredValueBuffer);
				}
			}
		}
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		OnCreateEntity(entity.m_Scene->GetUUID(), entity.GetComponent<IDComponent>().ID);
	}
	void ScriptEngine::OnCreateEntity(UUID sceneID, UUID entityID)
	{
        PR_PROFILE_FUNCTION();
		EntityInstance& entityInstance = GetEntityInstanceData(sceneID, entityID).Instance;
        if (entityInstance.ScriptClass->HasMethod("OnCreate"))
            entityInstance.Object->InvokeMethod("OnCreate");
	}
	void ScriptEngine::OnUpdateEntity(UUID sceneID, UUID entityID, float ts)
	{
        PR_PROFILE_FUNCTION();
        EntityInstance& entityInstance = GetEntityInstanceData(sceneID, entityID).Instance;
        if (entityInstance.ScriptClass->HasMethod("OnUpdate"))
            entityInstance.Object->InvokeMethod("OnUpdate");
	}
	void ScriptEngine::OnFixedUpdateEntity(UUID sceneID, UUID entityID)
	{
        PR_PROFILE_FUNCTION();
        EntityInstance& entityInstance = GetEntityInstanceData(sceneID, entityID).Instance;
        if (entityInstance.ScriptClass->HasMethod("OnFixedUpdate"))
		    entityInstance.Object->InvokeMethod("OnFixedUpdate");
	}
	void ScriptEngine::OnCollision2DBegin(Entity entity)
	{
		OnCollision2DBegin(entity.m_Scene->GetUUID(), entity.GetComponent<IDComponent>().ID);
	}
	void ScriptEngine::OnCollision2DBegin(UUID sceneID, UUID entityID)
	{
        PR_PROFILE_FUNCTION();
        EntityInstance& entityInstance = GetEntityInstanceData(sceneID, entityID).Instance;
		entityInstance.Object->InvokeMethod("OnCollision2DBegin", 5.0f);
	}
	void ScriptEngine::OnCollision2DEnd(Entity entity)
	{
		OnCollision2DEnd(entity.m_Scene->GetUUID(), entity.GetComponent<IDComponent>().ID);
	}
	void ScriptEngine::OnCollision2DEnd(UUID sceneID, UUID entityID)
	{
        PR_PROFILE_FUNCTION();
        EntityInstance& entityInstance = GetEntityInstanceData(sceneID, entityID).Instance;
		entityInstance.Object->InvokeMethod("OnCollision2DEnd", 5.0f);
	}
	void ScriptEngine::OnScriptComponentDestroyed(UUID sceneID, UUID entityID)
	{
		PR_CORE_ASSERT(s_EntityInstanceMap.find(sceneID) != s_EntityInstanceMap.end());
		auto& entityMap = s_EntityInstanceMap.at(sceneID);
		PR_CORE_ASSERT(entityMap.find(entityID) != entityMap.end());
		entityMap.erase(entityID);
	}
	bool ScriptEngine::ModuleExists(const std::string& moduleName)
	{
        return s_AppAssembly.GetType(moduleName);
	}

	void ScriptEngine::InitScriptEntity(Entity entity)
	{
		PR_PROFILE_FUNCTION();
		Scene* scene = entity.m_Scene;
		UUID id = entity.GetComponent<IDComponent>().ID;
		auto& moduleName = entity.GetComponent<ScriptComponent>().ModuleName;
		if (moduleName.empty())
			return;

		if (!ModuleExists(moduleName))
		{
			PR_CORE_ERROR("Entity references non-existent script module '{0}'", moduleName);
			return;
		}

		Rolky::Type& scriptClass = s_EntityClassMap[moduleName];
		scriptClass = s_AppAssembly.GetType(moduleName);

		EntityInstanceData& entityInstanceData = s_EntityInstanceMap[scene->GetUUID()][id];
		EntityInstance& entityInstance = entityInstanceData.Instance;
		entityInstance.ScriptClass = &scriptClass;
		ScriptModuleFieldMap& moduleFieldMap = entityInstanceData.ModuleFieldMap;
		auto& fieldMap = moduleFieldMap[moduleName];

		// 保存旧字段
		std::unordered_map<std::string, PublicField> oldFields;
		oldFields.reserve(fieldMap.size());
		for (auto& [fieldName, field] : fieldMap)
			oldFields.emplace(fieldName, std::move(field));
		fieldMap.clear();

		// 获取公共字段
		// TODO：如果模块被多次使用，则缓存这些字段
		{
			Rolky::ManagedObject temp = scriptClass.CreateInstance();
			byte defaultValue[32];
			auto Fields = scriptClass.GetFields();
			for (auto& field : Fields)
			{
				if (field.GetAccessibility() == Rolky::TypeAccessibility::Public)
				{
					FieldType prismFieldType = GetPrismFieldType(field.GetType());
					if (oldFields.find(field.GetName()) != oldFields.end())
					{
						fieldMap.emplace(field.GetName(), std::move(oldFields.at(field.GetName())));
					}
					else
					{
						PublicField prismField = { field.GetName(), prismFieldType };
						
						temp.GetFieldValueRaw((std::string)field.GetName(), defaultValue);
                        prismField.SetStoredValueRaw(defaultValue);
						prismField.m_EntityInstance = &entityInstance;
						fieldMap.emplace(field.GetName(), std::move(prismField));
					} // Example.MapGenerator
				}
			}
            temp.Destroy();
		}
	}
	void ScriptEngine::ShutdownScriptEntity(Entity entity, const std::string& moduleName)
	{
		EntityInstanceData& entityInstanceData = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID());
		ScriptModuleFieldMap& moduleFieldMap = entityInstanceData.ModuleFieldMap;
		if (moduleFieldMap.find(moduleName) != moduleFieldMap.end())
			moduleFieldMap.erase(moduleName);
	}
	void ScriptEngine::InstantiateEntityClass(Entity entity)
	{
		PR_PROFILE_FUNCTION();
		Scene* scene = entity.m_Scene;
		UUID id = entity.GetComponent<IDComponent>().ID;
		auto& moduleName = entity.GetComponent<ScriptComponent>().ModuleName;

		EntityInstanceData& entityInstanceData = GetEntityInstanceData(scene->GetUUID(), id);
		EntityInstance& entityInstance = entityInstanceData.Instance;
		PR_CORE_ASSERT(entityInstance.ScriptClass);
		
		entityInstance.Object = std::make_unique<Rolky::ManagedObject>();
        *entityInstance.Object = entityInstance.ScriptClass->CreateInstance();
		entityInstance.Object->SetPropertyValue("ID", id);
        

		// 将所有公共字段设置为适当的值
		ScriptModuleFieldMap& moduleFieldMap = entityInstanceData.ModuleFieldMap;
		if (moduleFieldMap.find(moduleName) != moduleFieldMap.end())
		{
			auto& publicFields = moduleFieldMap.at(moduleName);
			for (auto& [name, field] : publicFields)
				field.CopyStoredValueToRuntime();
		}

		
		OnCreateEntity(entity);
	}


	EntityInstanceData& ScriptEngine::GetEntityInstanceData(UUID sceneID, UUID entityID)
	{
		PR_CORE_ASSERT(s_EntityInstanceMap.find(sceneID) != s_EntityInstanceMap.end(), "Invalid scene ID!");
		auto& entityIDMap = s_EntityInstanceMap.at(sceneID);
		PR_CORE_ASSERT(entityIDMap.find(entityID) != entityIDMap.end(), "Invalid entity ID!");
		return entityIDMap.at(entityID);
	}

	EntityInstanceMap& ScriptEngine::GetEntityInstanceMap()
	{
		return s_EntityInstanceMap;
	}
	

	Rolky::ManagedAssembly& ScriptEngine::GetEngineAssembly()
	{
		return s_EngineAssembly;
	}

}


/// typedef int (CORECLR_DELEGATE_CALLTYPE* load_assembly_and_get_function_pointer_fn)(
/// 	const char_t* assembly_path,      // 程序集磁盘路径
/// 	const char_t* type_name,          // 类型全称 (命名空间.类名, 程序集名)
/// 	const char_t* method_name,        // 静态方法名
/// 	const char_t* delegate_type_name, // 委托类型名 (或使用特殊常量)
/// 	void* reserved,           // 必须为 nullptr
/// 	void** result              // 输出：接收到的函数指针
/// 	);