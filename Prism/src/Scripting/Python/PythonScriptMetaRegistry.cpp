#include "prpch.h"
#include "PythonScriptMetaRegistry.h"
#include "Prism/Core/Log.h"
#include "Prism/Core/Hash.h"
#include "Scripting/ScriptTypes.h"

#include <pybind11/pybind11.h>

#include <filesystem>
#include <algorithm>

namespace py = pybind11;

namespace Prism
{
    std::unordered_map<UUID, ScriptClassMetadata> PythonScriptMetaRegistry::s_Classes;
    std::unordered_map<UUID, std::string> PythonScriptMetaRegistry::s_ClassIDToFullName;
    bool PythonScriptMetaRegistry::s_Initialized = false;

    UUID PythonScriptMetaRegistry::GenerateClassID(const std::string& str)
    {
        return UUID(Hash::GenerateFNVHash64(str));
    }

    static void ReadPythonDefaultFieldValue(ScriptFieldMetadata& meta, py::object& obj, const std::string& fieldName)
    {
        try
        {
            py::object val = obj.attr(fieldName.c_str());
            switch (meta.Type)
            {
                case ScriptFieldType::Float:
                {
                    float f = val.cast<float>();
                    meta.DefaultValue = Buffer::Copy(&f, sizeof(float));
                    break;
                }
                case ScriptFieldType::Bool:
                {
                    bool b = val.cast<bool>();
                    meta.DefaultValue = Buffer::Copy(&b, sizeof(bool));
                    break;
                }
                case ScriptFieldType::Int32:
                {
                    int32_t i = val.cast<int32_t>();
                    meta.DefaultValue = Buffer::Copy(&i, sizeof(int32_t));
                    break;
                }
                case ScriptFieldType::Int64:
                {
                    int64_t i = val.cast<int64_t>();
                    meta.DefaultValue = Buffer::Copy(&i, sizeof(int64_t));
                    break;
                }
                case ScriptFieldType::Double:
                {
                    double d = val.cast<double>();
                    meta.DefaultValue = Buffer::Copy(&d, sizeof(double));
                    break;
                }
                case ScriptFieldType::Vector2:
                {
                    py::buffer_info info = py::buffer(val).request();
                    float* f = static_cast<float*>(info.ptr);
                    meta.DefaultValue = Buffer::Copy(f, sizeof(float) * 2);
                    break;
                }
                case ScriptFieldType::Vector3:
                {
                    py::buffer_info info = py::buffer(val).request();
                    float* f = static_cast<float*>(info.ptr);
                    meta.DefaultValue = Buffer::Copy(f, sizeof(float) * 3);
                    break;
                }
                case ScriptFieldType::Vector4:
                {
                    py::buffer_info info = py::buffer(val).request();
                    float* f = static_cast<float*>(info.ptr);
                    meta.DefaultValue = Buffer::Copy(f, sizeof(float) * 4);
                    break;
                }
                default:
                    break;
            }
        }
        catch (py::error_already_set& e)
        {
            PR_CORE_WARN("[Python Meta] 读取默认值异常: {}", e.what());
            PyErr_Clear();
        }
    }

    void PythonScriptMetaRegistry::ScanModule(py::module_& mod, const std::string& moduleName, py::object& behaviourClass)
    {
        py::dict modDict = mod.attr("__dict__");

        for (auto& item : modDict)
        {
            std::string name = py::str(item.first);
            if (name.empty() || name[0] == '_')
                continue;

            py::object cls = py::reinterpret_borrow<py::object>(item.second);
            if (!py::isinstance(cls, (PyObject*)&PyType_Type))
                continue;

            if (!PyObject_IsSubclass(cls.ptr(), behaviourClass.ptr()))
                continue;

            std::string fullName = cls.attr("__module__").cast<std::string>() + "." + name;
            if (fullName.find("Prism.") == 0)
                continue;

            UUID classID = PythonScriptMetaRegistry::GenerateClassID(fullName);
            if (PythonScriptMetaRegistry::s_Classes.find(classID) != PythonScriptMetaRegistry::s_Classes.end())
                continue;

            PR_CORE_INFO("[Python Meta] 模块: {0}", moduleName);
            PR_CORE_INFO("[Python Meta]   类: {0} (ID={1})", fullName, (uint64_t)classID);

            auto& classMeta = PythonScriptMetaRegistry::s_Classes[classID];
            classMeta.ClassID = classID;
            classMeta.FullName = fullName;
            classMeta.ModuleName = moduleName;
            classMeta.ClassName = name;
            PythonScriptMetaRegistry::s_ClassIDToFullName[classID] = fullName;

            py::object tempInstance;
            try { tempInstance = cls(); }
            catch (py::error_already_set& e)
            {
                PR_CORE_WARN("[Python Meta] 无法实例化 {}.{}: {}", moduleName, name, e.what());
                PyErr_Clear();
            }
            if (!tempInstance) continue;

            if (py::hasattr(cls, "__annotations__"))
            {
                py::dict ann = cls.attr("__annotations__");
                for (auto& annItem : ann)
                {
                    std::string fieldName = py::str(annItem.first);
                    std::string typeAnnotation = py::str(annItem.second);

                    if (fieldName.empty() || fieldName[0] == '_')
                        continue;

                    ScriptFieldType fieldType = ScriptFieldType::None;
                    if (typeAnnotation == "float") fieldType = ScriptFieldType::Float;
                    else if (typeAnnotation == "int") fieldType = ScriptFieldType::Int32;
                    else if (typeAnnotation == "bool") fieldType = ScriptFieldType::Bool;
                    else if (typeAnnotation == "str") fieldType = ScriptFieldType::Object;
                    else if (typeAnnotation.find("Vector2") != std::string::npos) fieldType = ScriptFieldType::Vector2;
                    else if (typeAnnotation.find("Vector3") != std::string::npos) fieldType = ScriptFieldType::Vector3;
                    else if (typeAnnotation.find("Vector4") != std::string::npos) fieldType = ScriptFieldType::Vector4;

                    if (fieldType == ScriptFieldType::None) continue;

                    uint32_t fieldHash = (uint32_t)(uint64_t)PythonScriptMetaRegistry::GenerateClassID(fieldName);
                    PR_CORE_INFO("[Python Meta]     字段: {0} : {1}", fieldName, typeAnnotation);

                    ScriptFieldMetadata fieldMeta;
                    fieldMeta.Name = fieldName;
                    fieldMeta.Type = fieldType;

                    try
                    {
                        if (py::hasattr(tempInstance, fieldName.c_str()))
                            ReadPythonDefaultFieldValue(fieldMeta, tempInstance, fieldName);
                    }
                    catch (...)
                    {
                        PR_CORE_WARN("[Python Meta] 读取字段默认值异常: {0}.{1}", fullName, fieldName);
                    }

                    classMeta.Fields[fieldHash] = std::move(fieldMeta);
                }
            }

            uint16_t mask = 0;
            if (py::hasattr(cls, "Awake"))            mask |= (uint16_t)LifecycleMethod::Awake;
            if (py::hasattr(cls, "OnEnable"))         mask |= (uint16_t)LifecycleMethod::OnEnable;
            if (py::hasattr(cls, "OnDisable"))        mask |= (uint16_t)LifecycleMethod::OnDisable;
            if (py::hasattr(cls, "OnCreate"))         mask |= (uint16_t)LifecycleMethod::OnCreate;
            if (py::hasattr(cls, "OnUpdate"))         mask |= (uint16_t)LifecycleMethod::OnUpdate;
            if (py::hasattr(cls, "LateUpdate"))       mask |= (uint16_t)LifecycleMethod::LateUpdate;
            if (py::hasattr(cls, "OnFixedUpdate"))    mask |= (uint16_t)LifecycleMethod::OnFixedUpdate;
            if (py::hasattr(cls, "OnDestroy"))        mask |= (uint16_t)LifecycleMethod::OnDestroy;
            if (py::hasattr(cls, "OnCollisionBegin")) mask |= (uint16_t)LifecycleMethod::OnCollisionBegin;
            if (py::hasattr(cls, "OnCollisionEnd"))   mask |= (uint16_t)LifecycleMethod::OnCollisionEnd;
            if (py::hasattr(cls, "OnTriggerBegin"))   mask |= (uint16_t)LifecycleMethod::OnTriggerBegin;
            if (py::hasattr(cls, "OnTriggerEnd"))     mask |= (uint16_t)LifecycleMethod::OnTriggerEnd;
            classMeta.LifecycleMask = mask;
        }
    }

    void PythonScriptMetaRegistry::ScanDirectory(const std::string& dirPath, const std::string& packagePrefix, py::object& behaviourClass)
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

                try
                {
                    py::module_ mod = py::module::import(moduleName.c_str());
                    ScanModule(mod, moduleName, behaviourClass);
                }
                catch (py::error_already_set& e)
                {
                    PR_CORE_WARN("[Python Meta] 无法导入模块: {0}\n  {1}", moduleName, e.what());
                    PyErr_Clear();
                }
            }
            else if (entry.is_directory())
            {
                std::string subPrefix = packagePrefix.empty() ? filename : packagePrefix + "." + filename;
                ScanDirectory(entry.path().string(), subPrefix, behaviourClass);
            }
        }
    }

    void PythonScriptMetaRegistry::Init()
    {
        if (s_Initialized) return;
        s_Initialized = true;
    }

    void PythonScriptMetaRegistry::Shutdown()
    {
        if (!s_Initialized) return;

        for (auto& [id, classMeta] : s_Classes)
            for (auto& [hash, fieldMeta] : classMeta.Fields)
                if (fieldMeta.DefaultValue)
                    fieldMeta.DefaultValue.Free();

        s_Classes.clear();
        s_ClassIDToFullName.clear();
        s_Initialized = false;
    }

    void PythonScriptMetaRegistry::BuildCache()
    {
        if (!s_Classes.empty())
        {
            for (auto& [id, classMeta] : s_Classes)
                for (auto& [hash, fieldMeta] : classMeta.Fields)
                    if (fieldMeta.DefaultValue)
                        fieldMeta.DefaultValue.Free();
            s_Classes.clear();
            s_ClassIDToFullName.clear();
        }

        PR_CORE_INFO("[Python Meta] 开始扫描 Python Behaviour 类...");

        try
        {
            py::module_ prismMod = py::module::import("Prism");
            py::object behaviourClass = prismMod.attr("Behaviour");
            ScanDirectory("Assets/scripts/Python", "", behaviourClass);
        }
        catch (py::error_already_set& e)
        {
            PR_CORE_WARN("[Python Meta] Prism.Behaviour 不可用，跳过扫描: {}", e.what());
            PyErr_Clear();
        }

        PR_CORE_INFO("[Python Meta] 扫描完成: {0} 个 Behaviour 类", s_Classes.size());
    }

    ScriptClassMetadata* PythonScriptMetaRegistry::GetClassMetadata(UUID classID)
    {
        auto it = s_Classes.find(classID);
        return it != s_Classes.end() ? &it->second : nullptr;
    }

    ScriptClassMetadata* PythonScriptMetaRegistry::GetClassMetadata(const std::string& fullName)
    {
        return GetClassMetadata(GenerateClassID(fullName));
    }

    ScriptFieldMetadata* PythonScriptMetaRegistry::GetFieldMetadata(UUID classID, const std::string& fieldName)
    {
        auto* classMeta = GetClassMetadata(classID);
        if (!classMeta)
            return nullptr;

        uint32_t fieldHash = (uint32_t)(uint64_t)GenerateClassID(fieldName);
        auto it = classMeta->Fields.find(fieldHash);
        return it != classMeta->Fields.end() ? &it->second : nullptr;
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
