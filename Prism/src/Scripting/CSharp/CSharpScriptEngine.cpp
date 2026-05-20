#include "prpch.h"
#include "CSharpScriptEngine.h"
#include "ScriptEngineRegistry.h"
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

	class CSharpPublicField : public PublicField
	{
	public:
		CSharpPublicField(const std::string& name, FieldType type, EntityInstance* entityInstance)
			: PublicField(name, type), m_EntityInstance(entityInstance) {}

		bool IsRuntimeAvailable() const override
		{
			return m_EntityInstance && m_EntityInstance->Object && m_EntityInstance->Object->IsValid();
		}

		void CopyStoredValueToRuntime() override
		{
			PR_CORE_ASSERT(m_EntityInstance && m_EntityInstance->Object && m_EntityInstance->Object->IsValid());
			m_EntityInstance->Object->SetFieldValueRaw(GetName(), const_cast<uint8_t*>(m_StoredValueBuffer));
		}

	protected:
		void GetRuntimeValue_Internal(void* outValue) const override
		{
			PR_CORE_ASSERT(m_EntityInstance && m_EntityInstance->Object && m_EntityInstance->Object->IsValid());
			m_EntityInstance->Object->GetFieldValueRaw(GetName(), outValue);
		}

		void SetRuntimeValue_Internal(const void* value) override
		{
			PR_CORE_ASSERT(m_EntityInstance && m_EntityInstance->Object && m_EntityInstance->Object->IsValid());
			m_EntityInstance->Object->SetFieldValueRaw(GetName(), const_cast<void*>(value));
		}

	private:
		EntityInstance* m_EntityInstance = nullptr;
	};

	CSharpScriptEngine::CSharpScriptEngine()
	{
	}

	CSharpScriptEngine::~CSharpScriptEngine()
	{
		Shutdown();
	}

	bool CSharpScriptEngine::Initialize()
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

	void CSharpScriptEngine::Shutdown()
	{
		if (m_Host && m_LoadContext)
			m_Host->UnloadAssemblyLoadContext(*m_LoadContext);
		if (m_Host)
			m_Host->Shutdown();
		m_SceneContext = nullptr;
		m_LoadContext = nullptr;
		m_EntityInstanceMap.clear();
	}

	bool CSharpScriptEngine::LoadEngineAssembly(const std::string& assemblyPath)
	{
		PR_PROFILE_FUNCTION();
		auto path = std::filesystem::absolute(assemblyPath).string();
		m_EngineAssembly = m_LoadContext->LoadAssembly(path);
		ScriptEngineRegistry::RegisterAll(*this);
		auto initClass = m_EngineAssembly.GetType("Prism.Core");
		initClass.InvokeStaticMethod("Init");
		return true;
	}

	bool CSharpScriptEngine::LoadAppAssembly(const std::string& assemblyPath)
	{
		PR_PROFILE_FUNCTION();
		auto path = std::filesystem::absolute(assemblyPath).string();
		m_AppAssembly = m_LoadContext->LoadAssembly(path);
		return true;
	}

	void CSharpScriptEngine::ReloadAssembly(const std::string& assemblyPath)
	{
		PR_PROFILE_FUNCTION();
		auto path = std::filesystem::absolute(assemblyPath).string();
		m_AppAssembly = m_LoadContext->LoadAssembly(path);
		if (m_EntityInstanceMap.size())
		{
			Ref<Scene> scene = GetCurrentSceneContext();
			PR_CORE_ASSERT(scene, "No active scene!");
			if (m_EntityInstanceMap.find(scene->GetUUID()) != m_EntityInstanceMap.end())
			{
				auto& entityMap = m_EntityInstanceMap.at(scene->GetUUID());
				for (auto& [entityID, entityInstanceData] : entityMap)
				{
					const auto& entityMap = scene->GetEntityMap();
					PR_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
					InitScriptEntity(entityMap.at(entityID));
				}
			}
		}
	}

	void CSharpScriptEngine::OnSceneDestruct(UUID sceneID)
	{
		if (m_EntityInstanceMap.find(sceneID) != m_EntityInstanceMap.end())
		{
			for (auto& [entityID, entityInstanceData] : m_EntityInstanceMap.at(sceneID))
			{
				if (entityInstanceData.Instance.Object)
					entityInstanceData.Instance.Object->Destroy();
			}
			m_EntityInstanceMap.at(sceneID).clear();
			m_EntityInstanceMap.erase(sceneID);
		}
	}

	void CSharpScriptEngine::SetSceneContext(const Ref<Scene>& scene)
	{
		m_SceneContext = scene;
	}

	const Ref<Scene>& CSharpScriptEngine::GetCurrentSceneContext()
	{
		return m_SceneContext;
	}

	void CSharpScriptEngine::CopyEntityScriptData(UUID dst, UUID src)
	{
		PR_CORE_ASSERT(m_EntityInstanceMap.find(dst) != m_EntityInstanceMap.end());
		PR_CORE_ASSERT(m_EntityInstanceMap.find(src) != m_EntityInstanceMap.end());

		auto& dstEntityMap = m_EntityInstanceMap.at(dst);
		auto& srcEntityMap = m_EntityInstanceMap.at(src);

		for (auto& [entityID, entityInstanceData] : srcEntityMap)
		{
			for (auto& [moduleName, srcFieldMap] : srcEntityMap[entityID].ModuleFieldMap)
			{
				auto& dstModuleFieldMap = dstEntityMap[entityID].ModuleFieldMap;
				for (auto& [fieldName, field] : srcFieldMap)
				{
					PR_CORE_ASSERT(dstModuleFieldMap.find(moduleName) != dstModuleFieldMap.end());
					auto& fieldMap = dstModuleFieldMap.at(moduleName);
					if (fieldMap.find(fieldName) == fieldMap.end())
						continue;
					fieldMap.at(fieldName)->SetStoredValueRaw(field->GetStoredValueBuffer());
				}
			}
		}
	}

	bool CSharpScriptEngine::HasEntityScriptData(UUID sceneID)
	{
		return m_EntityInstanceMap.find(sceneID) != m_EntityInstanceMap.end();
	}

	void CSharpScriptEngine::OnCreateEntity(Entity entity)
	{
		OnCreateEntity(entity.GetSceneUUID(), entity.GetComponent<IDComponent>().ID);
	}

	void CSharpScriptEngine::OnCreateEntity(UUID sceneID, UUID entityID)
	{
		PR_PROFILE_FUNCTION();
		EntityInstance& entityInstance = GetEntityInstanceData(sceneID, entityID).Instance;
		entityInstance.Object->TryInvokeMethod("OnCreate");
	}

	void CSharpScriptEngine::OnUpdateEntity(UUID sceneID, UUID entityID, float ts)
	{
		PR_PROFILE_FUNCTION();
		EntityInstance& entityInstance = GetEntityInstanceData(sceneID, entityID).Instance;
		entityInstance.Object->TryInvokeMethod("OnUpdate");
	}

	void CSharpScriptEngine::OnFixedUpdateEntity(UUID sceneID, UUID entityID)
	{
		PR_PROFILE_FUNCTION();
		EntityInstance& entityInstance = GetEntityInstanceData(sceneID, entityID).Instance;
		entityInstance.Object->TryInvokeMethod("OnFixedUpdate");
	}

	void CSharpScriptEngine::OnCollision2DBegin(Entity entity)
	{
		OnCollision2DBegin(entity.GetSceneUUID(), entity.GetComponent<IDComponent>().ID);
	}

	void CSharpScriptEngine::OnCollision2DBegin(UUID sceneID, UUID entityID)
	{
		PR_PROFILE_FUNCTION();
		EntityInstance& entityInstance = GetEntityInstanceData(sceneID, entityID).Instance;
		entityInstance.Object->InvokeMethod("OnCollision2DBegin", 5.0f);
	}

	void CSharpScriptEngine::OnCollision2DEnd(Entity entity)
	{
		OnCollision2DEnd(entity.GetSceneUUID(), entity.GetComponent<IDComponent>().ID);
	}

	void CSharpScriptEngine::OnCollision2DEnd(UUID sceneID, UUID entityID)
	{
		PR_PROFILE_FUNCTION();
		EntityInstance& entityInstance = GetEntityInstanceData(sceneID, entityID).Instance;
		entityInstance.Object->InvokeMethod("OnCollision2DEnd", 5.0f);
	}

	void CSharpScriptEngine::OnCollisionBegin(Entity entity)
	{
		OnCollisionBegin(entity.GetSceneUUID(), entity.GetComponent<IDComponent>().ID);
	}

	void CSharpScriptEngine::OnCollisionBegin(UUID sceneID, UUID entityID)
	{
		PR_PROFILE_FUNCTION();
		EntityInstance& entityInstance = GetEntityInstanceData(sceneID, entityID).Instance;
		entityInstance.Object->InvokeMethod("OnCollisionBegin", 5.0f);
	}

	void CSharpScriptEngine::OnCollisionEnd(Entity entity)
	{
		OnCollisionEnd(entity.GetSceneUUID(), entity.GetComponent<IDComponent>().ID);
	}

	void CSharpScriptEngine::OnCollisionEnd(UUID sceneID, UUID entityID)
	{
		PR_PROFILE_FUNCTION();
		EntityInstance& entityInstance = GetEntityInstanceData(sceneID, entityID).Instance;
		entityInstance.Object->InvokeMethod("OnCollisionEnd", 5.0f);
	}

	void CSharpScriptEngine::OnScriptComponentDestroyed(UUID sceneID, UUID entityID)
	{
		PR_CORE_ASSERT(m_EntityInstanceMap.find(sceneID) != m_EntityInstanceMap.end());
		auto& entityMap = m_EntityInstanceMap.at(sceneID);
		PR_CORE_ASSERT(entityMap.find(entityID) != entityMap.end());
		entityMap.erase(entityID);
	}

	bool CSharpScriptEngine::ModuleExists(const std::string& moduleName)
	{
		return m_AppAssembly.GetType(moduleName);
	}

	void CSharpScriptEngine::InitScriptEntity(Entity entity)
	{
		PR_PROFILE_FUNCTION();
		Scene* scene = entity.GetScene();
		UUID id = entity.GetComponent<IDComponent>().ID;
		auto& moduleName = entity.GetComponent<ScriptComponent>().ModuleName;
		if (moduleName.empty())
			return;

		if (!ModuleExists(moduleName))
		{
			PR_CORE_ERROR("Entity references non-existent script module '{0}'", moduleName);
			return;
		}

		Rolky::Type& scriptClass = m_EntityClassMap[moduleName];
		scriptClass = m_AppAssembly.GetType(moduleName);

		EntityInstanceData& entityInstanceData = m_EntityInstanceMap[scene->GetUUID()][id];
		EntityInstance& entityInstance = entityInstanceData.Instance;
		entityInstance.ScriptClass = &scriptClass;
		ScriptModuleFieldMap& moduleFieldMap = entityInstanceData.ModuleFieldMap;
		auto& fieldMap = moduleFieldMap[moduleName];

		// 保存旧字段
		std::unordered_map<std::string, std::unique_ptr<PublicField>> oldFields;
		for (auto& [fieldName, field] : fieldMap)
			oldFields[fieldName] = std::move(field);
		fieldMap.clear();

		// 获取公共字段
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
						fieldMap[field.GetName()] = std::move(oldFields.at(field.GetName()));
					}
					else
					{
						auto prismField = std::make_unique<CSharpPublicField>(field.GetName(), prismFieldType, &entityInstance);

						temp.GetFieldValueRaw((std::string)field.GetName(), defaultValue);
						prismField->SetStoredValueRaw(defaultValue);
						fieldMap[field.GetName()] = std::move(prismField);
					}
				}
			}
			temp.Destroy();
		}
	}

	void CSharpScriptEngine::ShutdownScriptEntity(Entity entity, const std::string& moduleName)
	{
		EntityInstanceData& entityInstanceData = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID());
		ScriptModuleFieldMap& moduleFieldMap = entityInstanceData.ModuleFieldMap;
		if (moduleFieldMap.find(moduleName) != moduleFieldMap.end())
			moduleFieldMap.erase(moduleName);
	}

	void CSharpScriptEngine::InstantiateEntityClass(Entity entity)
	{
		PR_PROFILE_FUNCTION();
		Scene* scene = entity.GetScene();
		UUID id = entity.GetComponent<IDComponent>().ID;
		auto& moduleName = entity.GetComponent<ScriptComponent>().ModuleName;

		EntityInstanceData& entityInstanceData = GetEntityInstanceData(scene->GetUUID(), id);
		EntityInstance& entityInstance = entityInstanceData.Instance;
		PR_CORE_ASSERT(entityInstance.ScriptClass);

		entityInstance.Object = std::make_unique<Rolky::ManagedObject>();
		*entityInstance.Object = std::move(entityInstance.ScriptClass->CreateInstance());
		entityInstance.Object->SetPropertyValue("ID", id);

		// 将所有公共字段设置为适当的值
		ScriptModuleFieldMap& moduleFieldMap = entityInstanceData.ModuleFieldMap;
		if (moduleFieldMap.find(moduleName) != moduleFieldMap.end())
		{
			auto& publicFields = moduleFieldMap.at(moduleName);
			for (auto& [name, field] : publicFields)
				field->CopyStoredValueToRuntime();
		}

		OnCreateEntity(entity);
	}

	EntityInstanceData& CSharpScriptEngine::GetEntityInstanceData(UUID sceneID, UUID entityID)
	{
		PR_CORE_ASSERT(m_EntityInstanceMap.find(sceneID) != m_EntityInstanceMap.end(), "Invalid scene ID!");
		auto& entityIDMap = m_EntityInstanceMap.at(sceneID);
		PR_CORE_ASSERT(entityIDMap.find(entityID) != entityIDMap.end(), "Invalid entity ID!");
		return entityIDMap.at(entityID);
	}

	EntityInstanceMap& CSharpScriptEngine::GetEntityInstanceMap()
	{
		return m_EntityInstanceMap;
	}

	Rolky::ManagedAssembly& CSharpScriptEngine::GetEngineAssembly()
	{
		return m_EngineAssembly;
	}

	// Debug
	void CSharpScriptEngine::OnImGuiRender()
	{
		ImGui::Begin("Script Engine Debug");
		for (auto& [sceneID, entityMap] : m_EntityInstanceMap)
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
									opened = ImGui::TreeNodeEx((void*)field.get(), ImGuiTreeNodeFlags_Leaf, fieldName.c_str());
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

	uint32_t CSharpScriptEngine::GetFieldCount(UUID sceneID, UUID entityID, const std::string& moduleName)
	{
		if (m_EntityInstanceMap.find(sceneID) == m_EntityInstanceMap.end())
			return 0;
		auto& entityMap = m_EntityInstanceMap.at(sceneID);
		if (entityMap.find(entityID) == entityMap.end())
			return 0;
		auto& entityInstanceData = entityMap.at(entityID);
		if (entityInstanceData.ModuleFieldMap.find(moduleName) == entityInstanceData.ModuleFieldMap.end())
			return 0;
		return (uint32_t)entityInstanceData.ModuleFieldMap.at(moduleName).size();
	}

	bool CSharpScriptEngine::GetFieldInfo(UUID sceneID, UUID entityID, const std::string& moduleName, uint32_t index, PublicFieldInfo& outInfo)
	{
		if (m_EntityInstanceMap.find(sceneID) == m_EntityInstanceMap.end())
			return false;
		auto& entityMap = m_EntityInstanceMap.at(sceneID);
		if (entityMap.find(entityID) == entityMap.end())
			return false;
		auto& entityInstanceData = entityMap.at(entityID);
		if (entityInstanceData.ModuleFieldMap.find(moduleName) == entityInstanceData.ModuleFieldMap.end())
			return false;
		auto& fields = entityInstanceData.ModuleFieldMap.at(moduleName);
		if (index >= fields.size())
			return false;
		auto it = fields.begin();
		std::advance(it, index);
		outInfo.Name = it->first;
		outInfo.Type = it->second->GetType();
		return true;
	}

	PublicField* CSharpScriptEngine::GetField(UUID sceneID, UUID entityID, const std::string& moduleName, const std::string& fieldName)
	{
		if (m_EntityInstanceMap.find(sceneID) == m_EntityInstanceMap.end())
			return nullptr;
		auto& entityMap = m_EntityInstanceMap.at(sceneID);
		if (entityMap.find(entityID) == entityMap.end())
			return nullptr;
		auto& entityInstanceData = entityMap.at(entityID);
		if (entityInstanceData.ModuleFieldMap.find(moduleName) == entityInstanceData.ModuleFieldMap.end())
			return nullptr;
		auto& fields = entityInstanceData.ModuleFieldMap.at(moduleName);
		auto it = fields.find(fieldName);
		if (it == fields.end())
			return nullptr;
		return it->second.get();
	}

	PublicField* CSharpScriptEngine::GetOrCreateField(UUID sceneID, UUID entityID, const std::string& moduleName, const std::string& fieldName, FieldType type)
	{
		auto& entityInstanceData = GetEntityInstanceData(sceneID, entityID);
		auto& fields = entityInstanceData.ModuleFieldMap[moduleName];
		auto it = fields.find(fieldName);
		if (it != fields.end())
			return it->second.get();

		auto field = std::make_unique<CSharpPublicField>(fieldName, type, &entityInstanceData.Instance);
		auto* ptr = field.get();
		fields[fieldName] = std::move(field);
		return ptr;
	}

}