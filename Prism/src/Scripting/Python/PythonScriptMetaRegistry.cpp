#include "prpch.h"
#include "PythonScriptMetaRegistry.h"
#include "Prism/Core/Log.h"

#include <filesystem>
#include <algorithm>

namespace Prism
{
#define PR_PYTHON_META_INFO(...)  PR_CORE_INFO("[Python Meta] "  __VA_ARGS__)
#define PR_PYTHON_META_WARN(...)  PR_CORE_WARN("[Python Meta] "  __VA_ARGS__)
#define PR_PYTHON_META_ERROR(...) PR_CORE_ERROR("[Python Meta] " __VA_ARGS__)

    std::unordered_map<UUID, ScriptClassMetadata> PythonScriptMetaRegistry::s_Classes;
    std::unordered_map<std::string, UUID> PythonScriptMetaRegistry::s_FullNameToID;
    bool PythonScriptMetaRegistry::s_Initialized = false;

    static constexpr uint64_t FNV1aBasis = 14695981039346656037ULL;
    static constexpr uint64_t FNV1aPrime = 1099511628211ULL;

    UUID PythonScriptMetaRegistry::GenerateScriptID(const std::string& str)
    {
        uint64_t hash = FNV1aBasis;
        for (char c : str)
        {
            hash ^= static_cast<uint64_t>(c);
            hash *= FNV1aPrime;
        }
        return UUID(hash);
    }

    void PythonScriptMetaRegistry::ReadPythonDefaultFieldValue(ScriptFieldMetadata& meta,
                                                               Python::ScriptObject& obj,
                                                               const std::string& fieldName)
    {
        switch (meta.Type)
        {
            case ScriptFieldType::Float:
            {
                float val = obj.GetField<float>(fieldName.c_str());
                meta.DefaultValue = Buffer::Copy(&val, sizeof(float));
                PR_PYTHON_META_INFO("    默认值 {0} = {1}", fieldName, val);
                break;
            }
            case ScriptFieldType::Bool:
            {
                bool val = obj.GetField<bool>(fieldName.c_str());
                meta.DefaultValue = Buffer::Copy(&val, sizeof(bool));
                PR_PYTHON_META_INFO("    默认值 {0} = {1}", fieldName, val ? "True" : "False");
                break;
            }
            case ScriptFieldType::Int32:
            {
                int32_t val = obj.GetField<int32_t>(fieldName.c_str());
                meta.DefaultValue = Buffer::Copy(&val, sizeof(int32_t));
                PR_PYTHON_META_INFO("    默认值 {0} = {1}", fieldName, val);
                break;
            }
            case ScriptFieldType::UInt32:
            case ScriptFieldType::UInt64:
            {
                uint64_t val = obj.GetField<uint64_t>(fieldName.c_str());
                meta.DefaultValue = Buffer::Copy(&val, sizeof(uint64_t));
                PR_PYTHON_META_INFO("    默认值 {0} = {1}", fieldName, val);
                break;
            }
            case ScriptFieldType::Vector2:
            {
                float vec[2];
                obj.GetFieldRaw(fieldName.c_str(), vec);
                meta.DefaultValue = Buffer::Copy(vec, sizeof(float) * 2);
                PR_PYTHON_META_INFO("    默认值 {0} = ({1}, {2})", fieldName, vec[0], vec[1]);
                break;
            }
            case ScriptFieldType::Vector3:
            {
                float vec[3];
                obj.GetFieldRaw(fieldName.c_str(), vec);
                meta.DefaultValue = Buffer::Copy(vec, sizeof(float) * 3);
                PR_PYTHON_META_INFO("    默认值 {0} = ({1}, {2}, {3})", fieldName, vec[0], vec[1], vec[2]);
                break;
            }
            case ScriptFieldType::Vector4:
            {
                float vec[4];
                obj.GetFieldRaw(fieldName.c_str(), vec);
                meta.DefaultValue = Buffer::Copy(vec, sizeof(float) * 4);
                PR_PYTHON_META_INFO("    默认值 {0} = ({1}, {2}, {3}, {4})", fieldName, vec[0], vec[1], vec[2], vec[3]);
                break;
            }
            default:
                break;
        }
    }

    void PythonScriptMetaRegistry::ScanModule(Python::ScriptModule& mod, const std::string& moduleName,
                                              Python::ScriptClass& behaviourClass)
    {
        std::vector<std::string> names = mod.GetNames();

        for (const auto& name : names)
        {
            if (!name.empty() && name[0] == '_')
                continue;

            Python::ScriptClass cls = Python::ScriptClass::From(mod, name.c_str());
            if (!cls.IsValid())
                continue;

            if (!cls.IsSubclassOf(behaviourClass))
                continue;

            std::string fullName = cls.GetFullName();
            if (fullName.empty())
                fullName = moduleName + "." + name;

            if (fullName.find("Prism.") == 0)
                continue;

            UUID scriptID = GenerateScriptID(fullName);

            if (s_Classes.find(scriptID) != s_Classes.end())
                continue;

            PR_PYTHON_META_INFO("模块: {0}", moduleName);
            PR_PYTHON_META_INFO("  类: {0} (ID={1})", fullName, (uint64_t)scriptID);

            auto& classMeta = s_Classes[scriptID];
            classMeta.ScriptID = scriptID;
            classMeta.FullName = fullName;
            classMeta.ModuleName = moduleName;
            classMeta.ClassName = cls.GetName();
            s_FullNameToID[fullName] = scriptID;

            Python::ScriptObject tempInstance = cls.CreateInstance();
            if (!tempInstance.IsValid())
            {
                PR_PYTHON_META_WARN("  无法创建临时实例: {0}", fullName);
                continue;
            }

            std::vector<Python::ScriptClass::FieldInfo> fields = cls.GetFields();
            for (const auto& field : fields)
            {
                ScriptFieldType fieldType = GetFieldTypeFromPythonAnnotation(field.TypeAnnotation);
                if (fieldType == ScriptFieldType::None)
                {
                    PR_PYTHON_META_WARN("  未知类型标注: {0}: {1}", field.Name, field.TypeAnnotation);
                    continue;
                }

                uint32_t fieldHash = (uint32_t)(uint64_t)GenerateScriptID(field.Name);
                PR_PYTHON_META_INFO("    字段: {0} : {1}", field.Name, field.TypeAnnotation);

                ScriptFieldMetadata fieldMeta;
                fieldMeta.Name = field.Name;
                fieldMeta.Type = fieldType;

                if (field.HasDefault)
                    ReadPythonDefaultFieldValue(fieldMeta, tempInstance, field.Name);

                classMeta.Fields[fieldHash] = std::move(fieldMeta);
            }
        }
    }

    void PythonScriptMetaRegistry::ScanDirectory(const std::string& dirPath, const std::string& packagePrefix,
                                                 Python::ScriptClass& behaviourClass)
    {
        namespace fs = std::filesystem;

        if (!fs::exists(dirPath))
            return;

        for (const auto& entry : fs::directory_iterator(dirPath))
        {
            std::string filename = entry.path().filename().string();

            if ((!filename.empty() && filename[0] == '.') || filename == "__pycache__")
                continue;
            if (filename == "Prism" && packagePrefix.empty())
                continue;

            if (entry.is_regular_file() && entry.path().extension() == ".py")
            {
                if (filename == "__init__.py")
                    continue;

                std::string moduleName = filename.substr(0, filename.size() - 3);
                if (!packagePrefix.empty())
                    moduleName = packagePrefix + "." + moduleName;

                Python::ScriptModule mod = Python::ScriptModule::Import(moduleName.c_str());
                if (!mod.IsValid())
                {
                    PR_PYTHON_META_WARN("无法导入模块: {0}", moduleName);
                    continue;
                }

                ScanModule(mod, moduleName, behaviourClass);
            }
            else if (entry.is_directory())
            {
                std::string subPrefix = packagePrefix.empty() ? filename : packagePrefix + "." + filename;
                std::string subPkgPath = entry.path().string();

                fs::path initPy = entry.path() / "__init__.py";
                if (fs::exists(initPy))
                {
                    Python::ScriptModule pkgMod = Python::ScriptModule::Import(subPrefix.c_str());
                    if (pkgMod.IsValid())
                        ScanModule(pkgMod, subPrefix, behaviourClass);
                }

                ScanDirectory(subPkgPath, subPrefix, behaviourClass);
            }
        }
    }

    void PythonScriptMetaRegistry::Init()
    {
        if (s_Initialized)
            return;
        s_Initialized = true;
    }

    void PythonScriptMetaRegistry::Shutdown()
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
        s_FullNameToID.clear();
        s_Initialized = false;
    }

    void PythonScriptMetaRegistry::BuildCache()
    {
        PR_PYTHON_META_INFO("开始扫描 Python Behaviour 类...");

        Python::ScriptModule prismMod = Python::ScriptModule::Import("Prism");
        if (!prismMod.IsValid())
        {
            PR_PYTHON_META_ERROR("无法导入 Prism 模块");
            return;
        }

        Python::ScriptClass behaviourClass = Python::ScriptClass::From(prismMod, "Behaviour");
        if (!behaviourClass.IsValid())
        {
            PR_PYTHON_META_ERROR("无法获取 Prism.Behaviour 类");
            return;
        }

        ScanDirectory("Assets/scripts/Python", "", behaviourClass);

        PR_PYTHON_META_INFO("扫描完成: {0} 个 Behaviour 类", s_Classes.size());
    }

    ScriptClassMetadata* PythonScriptMetaRegistry::GetClassMetadata(UUID scriptID)
    {
        auto it = s_Classes.find(scriptID);
        return it != s_Classes.end() ? &it->second : nullptr;
    }

    ScriptClassMetadata* PythonScriptMetaRegistry::GetClassMetadata(const std::string& fullName)
    {
        auto it = s_FullNameToID.find(fullName);
        return it != s_FullNameToID.end() ? GetClassMetadata(it->second) : nullptr;
    }

    std::vector<ScriptClassMetadata*> PythonScriptMetaRegistry::GetAllBehaviourClasses()
    {
        std::vector<ScriptClassMetadata*> result;
        result.reserve(s_Classes.size());
        for (auto& [id, meta] : s_Classes)
            result.push_back(&meta);
        return result;
    }

}
