#include "prpch.h"
#include "CSharpScriptMetaRegistry.h"
#include "CSharpScriptEngine.h"

#include <Prism/Core/Hash.h>

#include <Rolky/ManagedObject.hpp>
#include <Rolky/String.hpp>
#include <glm/glm.hpp>

#define PR_CSHARP_META_INFO(...)  PR_CORE_INFO("[C# Meta] "  __VA_ARGS__)
#define PR_CSHARP_META_WARN(...)  PR_CORE_WARN("[C# Meta] "  __VA_ARGS__)
#define PR_CSHARP_META_ERROR(...) PR_CORE_ERROR("[C# Meta] " __VA_ARGS__)

namespace Prism
{

    std::unordered_map<UUID, ScriptClassMetadata> CSharpScriptMetaRegistry::s_Classes;
    std::unordered_map<UUID, std::string> CSharpScriptMetaRegistry::s_ClassIDToFullName;
    Rolky::Type* CSharpScriptMetaRegistry::s_BehaviourType = nullptr;
    bool CSharpScriptMetaRegistry::s_Initialized = false;

    UUID CSharpScriptMetaRegistry::GenerateClassID(const std::string& str)
    {
        return UUID(Hash::GenerateFNVHash64(str));
    }

    ScriptFieldType CSharpScriptMetaRegistry::GetFieldTypeFromManagedType(Rolky::Type* type)
    {
        Rolky::ScopedString name = type->GetFullName();
        std::string nameStr = name;

        if (nameStr == "System.Single")              return ScriptFieldType::Float;
        if (nameStr == "System.Double")              return ScriptFieldType::Double;
        if (nameStr == "System.Int32")                return ScriptFieldType::Int32;
        if (nameStr == "System.UInt32")               return ScriptFieldType::UInt32;
        if (nameStr == "System.Int64")                return ScriptFieldType::Int64;
        if (nameStr == "System.UInt64")               return ScriptFieldType::UInt64;
        if (nameStr == "System.Boolean")              return ScriptFieldType::Bool;
        if (nameStr == "Prism.Vector2" || nameStr == "System.Numerics.Vector2")     return ScriptFieldType::Vector2;
        if (nameStr == "Prism.Vector3" || nameStr == "System.Numerics.Vector3")     return ScriptFieldType::Vector3;
        if (nameStr == "Prism.Vector4" || nameStr == "System.Numerics.Vector4")     return ScriptFieldType::Vector4;

        return ScriptFieldType::None;
    }

    static void ReadDefaultFieldValue(ScriptFieldMetadata& meta, Rolky::ManagedObject& obj, const std::string& fieldName)
    {
        switch (meta.Type)
        {
            case ScriptFieldType::Float:
            {
                float val = obj.GetFieldValue<float>(fieldName);
                meta.DefaultValue = Buffer::Copy(&val, sizeof(float));
                PR_CSHARP_META_INFO("    默认值 {0} = {1}", fieldName, val);
                break;
            }
            case ScriptFieldType::Double:
            {
                double val = obj.GetFieldValue<double>(fieldName);
                meta.DefaultValue = Buffer::Copy(&val, sizeof(double));
                PR_CSHARP_META_INFO("    默认值 {0} = {1}", fieldName, val);
                break;
            }
            case ScriptFieldType::Bool:
            {
                bool val = obj.GetFieldValue<bool>(fieldName);
                meta.DefaultValue = Buffer::Copy(&val, sizeof(bool));
                PR_CSHARP_META_INFO("    默认值 {0} = {1}", fieldName, val ? "True" : "False");
                break;
            }
            case ScriptFieldType::Int32:
            {
                int32_t val = obj.GetFieldValue<int32_t>(fieldName);
                meta.DefaultValue = Buffer::Copy(&val, sizeof(int32_t));
                PR_CSHARP_META_INFO("    默认值 {0} = {1}", fieldName, val);
                break;
            }
            case ScriptFieldType::UInt32:
            {
                uint32_t val = obj.GetFieldValue<uint32_t>(fieldName);
                meta.DefaultValue = Buffer::Copy(&val, sizeof(uint32_t));
                PR_CSHARP_META_INFO("    默认值 {0} = {1}", fieldName, val);
                break;
            }
            case ScriptFieldType::Int64:
            {
                int64_t val = obj.GetFieldValue<int64_t>(fieldName);
                meta.DefaultValue = Buffer::Copy(&val, sizeof(int64_t));
                PR_CSHARP_META_INFO("    默认值 {0} = {1}", fieldName, val);
                break;
            }
            case ScriptFieldType::UInt64:
            {
                uint64_t val = obj.GetFieldValue<uint64_t>(fieldName);
                meta.DefaultValue = Buffer::Copy(&val, sizeof(uint64_t));
                PR_CSHARP_META_INFO("    默认值 {0} = {1}", fieldName, val);
                break;
            }
            case ScriptFieldType::Vector2:
            {
                glm::vec2 val = obj.GetFieldValue<glm::vec2>(fieldName);
                meta.DefaultValue = Buffer::Copy(&val, sizeof(glm::vec2));
                PR_CSHARP_META_INFO("    默认值 {0} = ({1}, {2})", fieldName, val.x, val.y);
                break;
            }
            case ScriptFieldType::Vector3:
            {
                glm::vec3 val = obj.GetFieldValue<glm::vec3>(fieldName);
                meta.DefaultValue = Buffer::Copy(&val, sizeof(glm::vec3));
                PR_CSHARP_META_INFO("    默认值 {0} = ({1}, {2}, {3})", fieldName, val.x, val.y, val.z);
                break;
            }
            case ScriptFieldType::Vector4:
            {
                glm::vec4 val = obj.GetFieldValue<glm::vec4>(fieldName);
                meta.DefaultValue = Buffer::Copy(&val, sizeof(glm::vec4));
                PR_CSHARP_META_INFO("    默认值 {0} = ({1}, {2}, {3}, {4})", fieldName, val.x, val.y, val.z, val.w);
                break;
            }
            default:
                break;
        }
    }

    void CSharpScriptMetaRegistry::BuildCacheForAssembly(Rolky::ManagedAssembly& assembly)
    {
        for (const auto& type : assembly.GetLocalTypes())
        {
            Rolky::ScopedString fullName = type.GetFullName();
            std::string nameStr = fullName;

            if (nameStr.find("Prism.") == 0 || nameStr.find("System.") == 0 || nameStr.find("Rolky.") == 0)
                continue;

            if (!type.IsSubclassOf(*s_BehaviourType))
                continue;

            UUID classID = GenerateClassID(nameStr);
            auto& classMeta = s_Classes[classID];
            classMeta.ClassID = classID;
            classMeta.FullName = nameStr;

            auto dotPos = nameStr.rfind('.');
            classMeta.ModuleName = (dotPos != std::string::npos) ? nameStr.substr(0, dotPos) : nameStr;
            classMeta.ClassName = (dotPos != std::string::npos) ? nameStr.substr(dotPos + 1) : nameStr;

            s_ClassIDToFullName[classID] = nameStr;

            PR_CSHARP_META_INFO("模块: {0}", classMeta.ModuleName);
            PR_CSHARP_META_INFO("  类: {0} (ID={1})", nameStr, (uint64_t)classID);

            Rolky::ManagedObject tempInstance = type.CreateInstance();
            if (!tempInstance.IsValid())
            {
                PR_CSHARP_META_WARN("  无法创建临时实例: {0}", nameStr);
                continue;
            }

            for (auto& field : type.GetFields())
            {
                if (field.GetAccessibility() != Rolky::TypeAccessibility::Public)
                    continue;

                Rolky::ScopedString fieldName = field.GetName();
                std::string fieldNameStr = fieldName;

                if (fieldNameStr.find("k__BackingField") != std::string::npos)
                    continue;

                auto& fieldType = field.GetType();
                ScriptFieldType prismFieldType = GetFieldTypeFromManagedType(&fieldType);
                if (prismFieldType == ScriptFieldType::None)
                {
                    PR_CSHARP_META_WARN("  未知类型: {0}: {1}", fieldNameStr, fieldType.GetFullName().operator std::string());
                    continue;
                }

                uint32_t fieldHash = (uint32_t)(uint64_t)GenerateClassID(fieldNameStr);
                PR_CSHARP_META_INFO("    字段: {0} : {1}", fieldNameStr, (int)prismFieldType);

                ScriptFieldMetadata fieldMeta;
                fieldMeta.Name = fieldNameStr;
                fieldMeta.Type = prismFieldType;

                if (prismFieldType != ScriptFieldType::None)
                    ReadDefaultFieldValue(fieldMeta, tempInstance, fieldNameStr);

                classMeta.Fields[fieldHash] = std::move(fieldMeta);
            }

            // Scan lifecycle methods with signature matching
            {
                auto checkMethod = [&](const char* name, int expectedParamCount,
                                        const char* expectedParamType) -> bool {
                    for (auto& method : type.GetMethods())
                    {
                        std::string methodName = method.GetName();
                        if (methodName != name) continue;
                        auto& params = method.GetParameterTypes();
                        if ((int)params.size() != expectedParamCount) continue;
                        if (expectedParamCount > 0 && expectedParamType)
                        {
                            std::string pn = params[0]->GetFullName();
                            if (pn != expectedParamType) continue;
                        }
                        return true;
                    }
                    return false;
                };

                if (checkMethod("Awake", 0, nullptr))
                    classMeta.LifecycleMask |= (uint16_t)LifecycleMethod::Awake;
                if (checkMethod("OnEnable", 0, nullptr))
                    classMeta.LifecycleMask |= (uint16_t)LifecycleMethod::OnEnable;
                if (checkMethod("OnDisable", 0, nullptr))
                    classMeta.LifecycleMask |= (uint16_t)LifecycleMethod::OnDisable;
                if (checkMethod("OnCreate", 0, nullptr))
                    classMeta.LifecycleMask |= (uint16_t)LifecycleMethod::OnCreate;
                if (checkMethod("OnUpdate", 0, nullptr))
                    classMeta.LifecycleMask |= (uint16_t)LifecycleMethod::OnUpdate;
                if (checkMethod("LateUpdate", 0, nullptr))
                    classMeta.LifecycleMask |= (uint16_t)LifecycleMethod::LateUpdate;
                if (checkMethod("OnFixedUpdate", 0, nullptr))
                    classMeta.LifecycleMask |= (uint16_t)LifecycleMethod::OnFixedUpdate;
                if (checkMethod("OnDestroy", 0, nullptr))
                    classMeta.LifecycleMask |= (uint16_t)LifecycleMethod::OnDestroy;
                if (checkMethod("OnCollisionBegin", 1, "System.Single"))
                    classMeta.LifecycleMask |= (uint16_t)LifecycleMethod::OnCollisionBegin;
                if (checkMethod("OnCollisionEnd", 1, "System.Single"))
                    classMeta.LifecycleMask |= (uint16_t)LifecycleMethod::OnCollisionEnd;
                if (checkMethod("OnTriggerBegin", 1, "System.Single"))
                    classMeta.LifecycleMask |= (uint16_t)LifecycleMethod::OnTriggerBegin;
                if (checkMethod("OnTriggerEnd", 1, "System.Single"))
                    classMeta.LifecycleMask |= (uint16_t)LifecycleMethod::OnTriggerEnd;
            }

            tempInstance.Destroy();
        }
    }

    void CSharpScriptMetaRegistry::Init()
    {
        if (s_Initialized)
            return;
        s_Initialized = true;
    }

    void CSharpScriptMetaRegistry::Shutdown()
    {
        if (!s_Initialized)
            return;

        for (auto& [id, classMeta] : s_Classes)
        {
            for (auto& [hash, fieldMeta] : classMeta.Fields)
            {
                if (fieldMeta.DefaultValue)
                    fieldMeta.DefaultValue.Free();
            }
        }

        s_Classes.clear();
        s_ClassIDToFullName.clear();
        s_BehaviourType = nullptr;
        s_Initialized = false;
    }

    void CSharpScriptMetaRegistry::BuildCache()
    {
        PR_CSHARP_META_INFO("开始扫描 C# Behaviour 类...");

        auto& engineAssembly = CSharpScriptEngine::GetEngineAssembly();
        auto& behaviourType = engineAssembly.GetLocalType("Prism.Behaviour");
        s_BehaviourType = &behaviourType;

        auto& appAssembly = CSharpScriptEngine::GetAppAssembly();
        BuildCacheForAssembly(appAssembly);

        PR_CSHARP_META_INFO("扫描完成: {0} 个 Behaviour 类", s_Classes.size());
    }

    ScriptClassMetadata* CSharpScriptMetaRegistry::GetClassMetadata(UUID classID)
    {
        auto it = s_Classes.find(classID);
        return it != s_Classes.end() ? &it->second : nullptr;
    }

    ScriptClassMetadata* CSharpScriptMetaRegistry::GetClassMetadata(const std::string& fullName)
    {
        return GetClassMetadata(GenerateClassID(fullName));
    }

    std::vector<ScriptClassMetadata*> CSharpScriptMetaRegistry::GetAllBehaviourClasses()
    {
        std::vector<ScriptClassMetadata*> result;
        result.reserve(s_Classes.size());
        for (auto& [id, meta] : s_Classes)
            result.push_back(&meta);
        return result;
    }

    ScriptFieldMetadata* CSharpScriptMetaRegistry::GetFieldMetadata(UUID classID, const std::string& fieldName)
    {
        auto* classMeta = GetClassMetadata(classID);
        if (!classMeta)
            return nullptr;

        uint32_t fieldHash = (uint32_t)(uint64_t)GenerateClassID(fieldName);
        auto it = classMeta->Fields.find(fieldHash);
        return it != classMeta->Fields.end() ? &it->second : nullptr;
    }

}
