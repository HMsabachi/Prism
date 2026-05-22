#include "prpch.h"
#include "CSharpScriptEngine.h"
#include "CSharpObject.h"
#include "ScriptEngineRegistry.h"
#include "Prism/Scene/Scene.h"
#include "Prism/Scene/Entity.h"
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
		auto it = s_FieldTypeMap.find(type.GetFullName());
		return it != s_FieldTypeMap.end() ? it->second : FieldType::None;
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

	// C# 公开字段 — 通过 ScriptGroup::Instance (CSharpObject*) 读写运行时值
	class CSharpPublicField : public PublicField
	{
	public:
		CSharpPublicField(const std::string& name, FieldType type, ScriptGroup* group)
			: PublicField(name, type), m_Group(group) {}

		bool IsRuntimeAvailable() const override
		{
			return m_Group && m_Group->Instance && m_Group->Instance->IsValid();
		}

		void CopyStoredValueToRuntime() override
		{
			PR_CORE_ASSERT(m_Group && m_Group->Instance && m_Group->Instance->IsValid());
			auto* handle = static_cast<CSharpObject*>(m_Group->Instance.get())->GetHandle();
			handle->SetFieldValueRaw(GetName(), const_cast<uint8_t*>(m_StoredValueBuffer));
		}

	protected:
		void GetRuntimeValue_Internal(void* outValue) const override
		{
			PR_CORE_ASSERT(m_Group && m_Group->Instance && m_Group->Instance->IsValid());
			auto* handle = static_cast<CSharpObject*>(m_Group->Instance.get())->GetHandle();
			handle->GetFieldValueRaw(GetName(), outValue);
		}

		void SetRuntimeValue_Internal(const void* value) override
		{
			PR_CORE_ASSERT(m_Group && m_Group->Instance && m_Group->Instance->IsValid());
			auto* handle = static_cast<CSharpObject*>(m_Group->Instance.get())->GetHandle();
			handle->SetFieldValueRaw(GetName(), const_cast<void*>(value));
		}

	private:
		ScriptGroup* m_Group = nullptr;
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
		m_Initialized = true;
		return true;
	}

	void CSharpScriptEngine::Shutdown()
	{
		if (m_Host && m_LoadContext)
			m_Host->UnloadAssemblyLoadContext(*m_LoadContext);
		if (m_Host)
		{
			m_Host->Shutdown();
			m_Host.reset();
		}
		m_SceneContext = nullptr;
		m_LoadContext = nullptr;
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

		Ref<Scene> scene = GetCurrentSceneContext();
		if (!scene) return;

		// 重新初始化所有脚本
		for (auto& [entityID, entityStorage] : scene->GetScriptStorage().GetEntities())
		{
			const auto& entityMap = scene->GetEntityMap();
			auto eit = entityMap.find(entityID);
			if (eit == entityMap.end()) continue;
			Entity entity = eit->second;
			for (auto& [moduleName, group] : entityStorage.Groups)
			{
				InitScriptEntity(entity, group);
			}
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

	bool CSharpScriptEngine::ModuleExists(const std::string& moduleName)
	{
		return m_AppAssembly.GetType(moduleName);
	}

	void CSharpScriptEngine::InitScriptEntity(Entity& entity, ScriptGroup& group)
	{
		PR_PROFILE_FUNCTION();

		if (group.ModuleName.empty())
			return;

		if (!ModuleExists(group.ModuleName))
		{
			PR_CORE_ERROR("Entity references non-existent C# script module '{0}'", group.ModuleName);
			return;
		}

		Rolky::Type& scriptClass = m_EntityClassMap[group.ModuleName];
		scriptClass = m_AppAssembly.GetType(group.ModuleName);

		// Detect methods
		group.MethodMask = 0;
		auto methods = scriptClass.GetMethods();
		for (auto& method : methods)
		{
			if (method.GetName() == "OnCreate")		group.MethodMask |= BIT(0);
			if (method.GetName() == "OnUpdate")		group.MethodMask |= BIT(1);
			if (method.GetName() == "OnFixedUpdate") group.MethodMask |= BIT(2);
		}

		// Save old fields
		std::unordered_map<std::string, std::unique_ptr<PublicField>> oldFields;
		for (auto& [name, field] : group.Fields)
			oldFields[name] = std::move(field);
		group.Fields.clear();

		// Discover public fields via reflection
		{
			Rolky::ManagedObject temp = scriptClass.CreateInstance();
			byte defaultValue[32];
			auto fields = scriptClass.GetFields();
			for (auto& field : fields)
			{
				if (field.GetAccessibility() == Rolky::TypeAccessibility::Public)
				{
					FieldType prismFieldType = GetPrismFieldType(field.GetType());
					if (oldFields.find(field.GetName()) != oldFields.end())
					{
						group.Fields[field.GetName()] = std::move(oldFields.at(field.GetName()));
					}
					else
					{
						auto prismField = std::make_unique<CSharpPublicField>(field.GetName(), prismFieldType, &group);
						temp.GetFieldValueRaw((std::string)field.GetName(), defaultValue);
						prismField->SetStoredValueRaw(defaultValue);
						group.Fields[field.GetName()] = std::move(prismField);
					}
				}
			}
			temp.Destroy();
		}
	}

	void CSharpScriptEngine::ShutdownScriptEntity(ScriptGroup& group)
	{
		group.Instance.reset(); // CSharpObject 析构时自动 Destroy ManagedObject
	}

	void CSharpScriptEngine::InstantiateEntityClass(ScriptGroup& group)
	{
		PR_PROFILE_FUNCTION();

		auto& scriptClass = m_EntityClassMap[group.ModuleName];

		auto managedObj = std::make_unique<Rolky::ManagedObject>();
		*managedObj = std::move(scriptClass.CreateInstance());
		managedObj->SetPropertyValue("ID", group.EntityID);

		group.Instance = std::make_unique<CSharpObject>(std::move(managedObj));

		// Copy stored field values to runtime
		for (auto& [name, field] : group.Fields)
			field->CopyStoredValueToRuntime();
	}

	Rolky::ManagedAssembly& CSharpScriptEngine::GetEngineAssembly()
	{
		return m_EngineAssembly;
	}

	void CSharpScriptEngine::OnImGuiRender()
	{
		ImGui::Begin("Script Engine Debug");
		ImGui::Text("C# Engine Initialized: %s", m_Initialized ? "Yes" : "No");
		ImGui::End();
	}

}
