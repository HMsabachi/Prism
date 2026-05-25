#include "prpch.h"
#include "PythonScriptCore.h"
#ifdef ERROR
#undef ERROR
#endif
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include <Python.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <memory>

namespace {

    inline PyObject* ToPy(Prism::Python::ScriptValue* v) { return reinterpret_cast<PyObject*>(v); }
    inline Prism::Python::ScriptValue* ToSV(PyObject* p) { return reinterpret_cast<Prism::Python::ScriptValue*>(p); }

    struct PersistentModuleData
    {
        std::string Name;
        PyModuleDef Def;
        std::vector<PyMethodDef> Methods;
        std::vector<std::string> MethodNames;
        std::vector<std::string> MethodDocs;
    };
    std::vector<std::unique_ptr<PersistentModuleData>> s_ModuleRegistry;

} // anonymous namespace

namespace Prism::Python {

    GILGuard::GILGuard()
        : m_State(reinterpret_cast<void*>(static_cast<uintptr_t>(PyGILState_Ensure())))
    {
    }

    GILGuard::~GILGuard()
    {
        PyGILState_Release(static_cast<PyGILState_STATE>(reinterpret_cast<uintptr_t>(m_State)));
    }

    PyErrorSaver::PyErrorSaver()
    {
        PyObject *e = nullptr, *v = nullptr, *tb = nullptr;
        PyErr_Fetch(&e, &v, &tb);
        m_Exc = e; m_Val = v; m_Tb = tb;
    }

    PyErrorSaver::~PyErrorSaver()
    {
        Restore();
    }

    void PyErrorSaver::Clear()
    {
        Py_XDECREF(reinterpret_cast<PyObject*>(m_Exc));
        Py_XDECREF(reinterpret_cast<PyObject*>(m_Val));
        Py_XDECREF(reinterpret_cast<PyObject*>(m_Tb));
        m_Exc = m_Val = m_Tb = nullptr;
    }

    void PyErrorSaver::Log()
    {
        if (!PyErr_Occurred()) return;

        PyObject *exc, *val, *tb;
        PyErr_Fetch(&exc, &val, &tb);
        PyErr_NormalizeException(&exc, &val, &tb);

        PyObject* str = PyObject_Str(val);
        const char* msg = str ? PyUnicode_AsUTF8(str) : "(unknown)";
        PR_CORE_ERROR("[Python Error] {}", msg);
        Py_XDECREF(str);

        PyErr_Restore(exc, val, tb);
        PyErr_Print();

        Clear();
    }

    void PyErrorSaver::Restore()
    {
        PyObject *curExc = nullptr, *curVal = nullptr, *curTb = nullptr;
        PyErr_Fetch(&curExc, &curVal, &curTb);
        Py_XDECREF(curExc); Py_XDECREF(curVal); Py_XDECREF(curTb);

        if (m_Exc)
            PyErr_Restore(
                reinterpret_cast<PyObject*>(m_Exc),
                reinterpret_cast<PyObject*>(m_Val),
                reinterpret_cast<PyObject*>(m_Tb)
            );
        else
        {
            Py_XDECREF(reinterpret_cast<PyObject*>(m_Exc));
            Py_XDECREF(reinterpret_cast<PyObject*>(m_Val));
            Py_XDECREF(reinterpret_cast<PyObject*>(m_Tb));
        }
    }

    ScriptRef::~ScriptRef()
    {
        Py_XDECREF(ToPy(m_Value));
        m_Value = nullptr;
    }

    ScriptRef::ScriptRef(ScriptValue* value)
        : m_Value(value)
    {
        Py_XINCREF(ToPy(m_Value));
    }

    ScriptRef::ScriptRef(const ScriptRef& other)
    {
        if (other.m_Value)
        {
            m_Value = other.m_Value;
            Py_XINCREF(ToPy(m_Value));
        }
    }

    ScriptRef::ScriptRef(ScriptRef&& other) noexcept
        : m_Value(other.m_Value)
    {
        other.m_Value = nullptr;
    }

    ScriptRef& ScriptRef::operator=(const ScriptRef& other)
    {
        if (this != &other)
        {
            Py_XDECREF(ToPy(m_Value));
            m_Value = other.m_Value;
            Py_XINCREF(ToPy(m_Value));
        }
        return *this;
    }

    ScriptRef& ScriptRef::operator=(ScriptRef&& other) noexcept
    {
        if (this != &other)
        {
            Py_XDECREF(ToPy(m_Value));
            m_Value = other.m_Value;
            other.m_Value = nullptr;
        }
        return *this;
    }

    bool ScriptRef::IsNone() const
    {
        return m_Value == ToSV(Py_None) || m_Value == nullptr;
    }

    ScriptRef ScriptRef::Adopt(ScriptValue* value)
    {
        ScriptRef ref;
        ref.m_Value = value;
        return ref;
    }

    ScriptValue* ScriptRef::Detach()
    {
        auto p = m_Value;
        m_Value = nullptr;
        return p;
    }

    ScriptRef ScriptRef::GetAttribute(const char* name) const
    {
        GILGuard gil;
        PyErrorSaver saver;
        ScriptRef result;
        if (m_Value)
        {
            PyObject* attr = PyObject_GetAttrString(ToPy(m_Value), name);
            if (attr)
                result = ScriptRef::Adopt(ToSV(attr));
        }
        return result;
    }

    void ScriptRef::SetAttribute(const char* name, const ScriptRef& value) const
    {
        GILGuard gil;
        if (m_Value && value.m_Value)
            PyObject_SetAttrString(ToPy(m_Value), name, ToPy(value.m_Value));
    }

    bool ScriptRef::HasAttribute(const char* name) const
    {
        GILGuard gil;
        bool result = m_Value && PyObject_HasAttrString(ToPy(m_Value), name);
        return result;
    }

    bool ScriptHost::Initialize()
    {
        if (Py_IsInitialized())
            return true;
        std::filesystem::path pythonHome = std::filesystem::absolute("../vendor/Python");
        Py_SetPythonHome(pythonHome.wstring().c_str());

        SetDllDirectoryW(pythonHome.wstring().c_str());

        Py_Initialize();
        if (!Py_IsInitialized())
            return false;
        PyRun_SimpleString(
            "import sys, os\n"

            "vendor_site = r'E:/PrismEngine/Prism/vendor/Python/Lib/site-packages'\n"
            "if vendor_site not in sys.path:\n"
            "    sys.path.insert(0, vendor_site)\n"

            "scripts_path = os.path.abspath('Assets/scripts/Python')\n"
            "if scripts_path not in sys.path:\n"
            "    sys.path.insert(0, scripts_path)\n"
            "print(f'[Python] {scripts_path}')\n"
            "import glm\n"
        );

        return true;
    }

    void ScriptHost::Shutdown()
    {
        if (Py_IsInitialized())
            Py_Finalize();
    }

    bool ScriptHost::IsInitialized()
    {
        return Py_IsInitialized() != 0;
    }

    ScriptModule ScriptModule::Import(const char* name)
    {
        GILGuard gil;
        PyErrorSaver saver;
        ScriptModule mod;
        PyObject* module = PyImport_ImportModule(name);
        if (module)
            mod.m_Ref = ScriptRef::Adopt(ToSV(module));
        else
            saver.Log();
        return mod;
    }

    bool ScriptModule::ModuleExists(const char* name)
    {
        GILGuard gil;
        PyErrorSaver saver;

        PyObject* importlib = PyImport_ImportModule("importlib.util");
        bool exists = false;
        if (importlib)
        {
            PyObject* findSpec = PyObject_GetAttrString(importlib, "find_spec");
            if (findSpec)
            {
                PyObject* arg = PyUnicode_FromString(name);
                PyObject* spec = PyObject_CallFunctionObjArgs(findSpec, arg, nullptr);
                if (spec)
                {
                    exists = (spec != Py_None);
                    Py_DECREF(spec);
                }
                Py_DECREF(arg);
                Py_DECREF(findSpec);
            }
            Py_DECREF(importlib);
        }

        return exists;
    }

    ScriptRef ScriptModule::GetAttribute(const char* name) const
    {
        return m_Ref.GetAttribute(name);
    }

    bool ScriptModule::HasAttribute(const char* name) const
    {
        return m_Ref.HasAttribute(name);
    }

    std::vector<std::string> ScriptModule::GetNames() const
    {
        std::vector<std::string> names;
        if (!IsValid())
            return names;

        GILGuard gil;
        PyErrorSaver saver;

        PyObject* pyMod = ToPy(m_Ref.Get());
        PyObject* dirList = PyObject_Dir(pyMod);
        if (dirList && PyList_Check(dirList))
        {
            Py_ssize_t count = PyList_Size(dirList);
            names.reserve(static_cast<size_t>(count));
            for (Py_ssize_t i = 0; i < count; i++)
            {
                PyObject* item = PyList_GetItem(dirList, i);
                if (item && PyUnicode_Check(item))
                {
                    const char* s = PyUnicode_AsUTF8(item);
                    if (s) names.emplace_back(s);
                }
            }
        }
        Py_XDECREF(dirList);
        return names;
    }

    ScriptClass ScriptClass::From(const ScriptModule& mod, const char* name)
    {
        ScriptClass cls;
        if (mod.IsValid())
        {
            ScriptRef attr = mod.GetAttribute(name);
            if (attr.IsValid())
                cls.m_Ref = std::move(attr);
        }
        return cls;
    }

    bool ScriptClass::HasMethod(const char* name) const
    {
        return m_Ref.HasAttribute(name);
    }

    bool ScriptClass::HasMethodWithArity(const char* name, int userArgCount) const
    {
        PyObject* pyCls = ToPy(m_Ref.Get());
        if (!pyCls) return false;

        PyObject* func = PyObject_GetAttrString(pyCls, name);
        if (!func) { PyErr_Clear(); return false; }

        bool ok = PyFunction_Check(func) || PyMethod_Check(func);
        if (ok)
        {
            PyObject* code = PyObject_GetAttrString(func, "__code__");
            if (code)
            {
                PyObject* coArgcount = PyObject_GetAttrString(code, "co_argcount");
                ok = coArgcount && (PyLong_AsLong(coArgcount) == userArgCount + 1);
                Py_XDECREF(coArgcount);
                Py_XDECREF(code);
            }
            else
            {
                PyErr_Clear();
                ok = false;
            }
        }
        Py_DECREF(func);
        return ok;
    }

    std::string ScriptClass::GetName() const
    {
        if (!m_Ref.IsValid()) return {};
        GILGuard gil;
        PyObject* pyCls = ToPy(m_Ref.Get());
        PyObject* nameAttr = PyObject_GetAttrString(pyCls, "__name__");
        std::string result;
        if (nameAttr && PyUnicode_Check(nameAttr))
        {
            const char* s = PyUnicode_AsUTF8(nameAttr);
            if (s) result = s;
        }
        Py_XDECREF(nameAttr);
        return result;
    }

    std::string ScriptClass::GetFullName() const
    {
        if (!m_Ref.IsValid()) return {};
        GILGuard gil;
        PyObject* pyCls = ToPy(m_Ref.Get());

        PyObject* modName = PyObject_GetAttrString(pyCls, "__module__");
        PyObject* qualName = PyObject_GetAttrString(pyCls, "__qualname__");

        std::string result;
        if (modName && PyUnicode_Check(modName) && qualName && PyUnicode_Check(qualName))
        {
            const char* mod = PyUnicode_AsUTF8(modName);
            const char* qn = PyUnicode_AsUTF8(qualName);
            if (mod && qn)
            {
                result = mod;
                result += '.';
                result += qn;
            }
        }
        else
        {
            if (qualName && PyUnicode_Check(qualName))
            {
                const char* qn = PyUnicode_AsUTF8(qualName);
                if (qn) result = qn;
            }
        }

        Py_XDECREF(modName);
        Py_XDECREF(qualName);
        return result;
    }

    bool ScriptClass::IsSubclassOf(const ScriptClass& other) const
    {
        if (!m_Ref.IsValid() || !other.m_Ref.IsValid())
            return false;

        GILGuard gil;
        PyObject* pyCls = ToPy(m_Ref.Get());
        PyObject* pyOther = ToPy(other.m_Ref.Get());
        int result = PyObject_IsSubclass(pyCls, pyOther);
        return result == 1;
    }

    std::vector<ScriptClass::FieldInfo> ScriptClass::GetFields() const
    {
        std::vector<FieldInfo> fields;
        if (!m_Ref.IsValid())
            return fields;

        GILGuard gil;
        PyErrorSaver saver;
        PyObject* pyCls = ToPy(m_Ref.Get());

        PyObject* annotations = PyObject_GetAttrString(pyCls, "__annotations__");
        if (!annotations || !PyDict_Check(annotations))
        {
            Py_XDECREF(annotations);
            return fields;
        }

        fields.reserve(static_cast<size_t>(PyDict_Size(annotations)));

        PyObject* key, * annValue;
        Py_ssize_t pos = 0;
        while (PyDict_Next(annotations, &pos, &key, &annValue))
        {
            if (!key || !PyUnicode_Check(key))
                continue;

            const char* fieldName = PyUnicode_AsUTF8(key);
            if (!fieldName)
                continue;

            FieldInfo info;
            info.Name = fieldName;

            PyObject* typeNameAttr = PyObject_GetAttrString(annValue, "__name__");
            if (typeNameAttr && PyUnicode_Check(typeNameAttr))
            {
                const char* tn = PyUnicode_AsUTF8(typeNameAttr);
                if (tn) info.TypeAnnotation = tn;
            }
            else
            {
                PyObject* strRepr = PyObject_Str(annValue);
                if (strRepr && PyUnicode_Check(strRepr))
                {
                    const char* s = PyUnicode_AsUTF8(strRepr);
                    if (s) info.TypeAnnotation = s;
                }
                Py_XDECREF(strRepr);
            }
            Py_XDECREF(typeNameAttr);

            PyObject* defaultVal = PyObject_GetAttrString(pyCls, fieldName);
            if (defaultVal)
            {
                info.HasDefault = !(PyFunction_Check(defaultVal) || PyMethod_Check(defaultVal));
                Py_DECREF(defaultVal);
            }

            fields.push_back(std::move(info));
        }

        Py_XDECREF(annotations);
        return fields;
    }

    ScriptClass::AnnotationMap ScriptClass::GetAnnotations() const
    {
        AnnotationMap result;
        if (!m_Ref.IsValid())
            return result;

        GILGuard gil;
        PyErrorSaver saver;

        PyObject* pyCls = ToPy(m_Ref.Get());
        PyObject* annotations = PyObject_GetAttrString(pyCls, "__annotations__");
        if (annotations && PyDict_Check(annotations))
        {
            PyObject* key, * value;
            Py_ssize_t pos = 0;
            while (PyDict_Next(annotations, &pos, &key, &value))
            {
                const char* fieldName = PyUnicode_AsUTF8(key);
                if (fieldName)
                {
                    result[fieldName] = ScriptRef(ToSV(value));
                }
            }
        }

        Py_XDECREF(annotations);
        return result;
    }

    ScriptObject ScriptClass::CreateInstance() const
    {
        if (!m_Ref.IsValid())
            return ScriptObject();

        GILGuard gil;
        PyErrorSaver saver;
        ScriptObject obj;
        PyObject* instance = PyObject_CallObject(ToPy(m_Ref.Get()), nullptr);
        if (instance)
        {
            obj.m_Ref = ScriptRef::Adopt(ToSV(instance));
        }
        else
        {
            saver.Log();
        }
        return obj;
    }

    ScriptObject::ScriptObject(ScriptRef ref)
        : m_Ref(std::move(ref))
    {
    }

    ScriptRef ScriptObject::InvokeWithTuple(const char* method, const ScriptRef& tuple)
    {
        GILGuard gil;
        PyErrorSaver saver;
        ScriptRef result;
        if (m_Ref.IsValid())
        {
            PyObject* func = PyObject_GetAttrString(ToPy(m_Ref.Get()), method);
            if (func)
            {
                PyObject* ret = PyObject_CallObject(func, ToPy(tuple.Get()));
                if (ret)
                    result = ScriptRef::Adopt(ToSV(ret));
                else
                    saver.Log();
                Py_DECREF(func);
            }
        }
        return result;
    }

    void ScriptObject::GetFieldRaw(const char* name, void* buffer) const
    {
        GILGuard gil;
        PyErrorSaver saver;
        if (m_Ref.IsValid())
        {
            PyObject* val = PyObject_GetAttrString(ToPy(m_Ref.Get()), name);
            if (val)
            {
                if (PyFloat_Check(val))
                {
                    float d = (float)PyFloat_AS_DOUBLE(val);
                    memcpy(buffer, &d, sizeof(float));
                }
                else if (PyLong_Check(val))
                {
                    int32_t ival = (int32_t)PyLong_AsLongLong(val);
                    memcpy(buffer, &ival, sizeof(int32_t));
                }
                else if (PyUnicode_Check(val))
                {
                    const char* str = PyUnicode_AsUTF8(val);
                    if (str && buffer)
                    {
                        size_t len = strlen(str) + 1;
                        memcpy(buffer, str, (std::min)(len, (size_t)256));
                    }
                }
                else if (PyTuple_Check(val))
                {
                    Py_ssize_t sz = PyTuple_Size(val);
                    for (Py_ssize_t i = 0; i < sz && i < 4; i++)
                    {
                        PyObject* elem = PyTuple_GetItem(val, i);
                        if (PyFloat_Check(elem))
                        {
                            float f = (float)PyFloat_AS_DOUBLE(elem);
                            memcpy((char*)buffer + i * sizeof(float), &f, sizeof(float));
                        }
                    }
                }

                Py_DECREF(val);
            }
        }
    }

    void ScriptObject::SetFieldRaw(const char* name, const void* buffer) const
    {
        GILGuard gil;
        PyErrorSaver saver;
        if (m_Ref.IsValid())
        {
            PyObject* existing = PyObject_GetAttrString(ToPy(m_Ref.Get()), name);
            PyObject* val = nullptr;

            if (existing)
            {
                if (PyFloat_Check(existing))
                    val = PyFloat_FromDouble(*(const float*)buffer);
                else if (PyLong_Check(existing))
                    val = PyLong_FromLong(*(const int32_t*)buffer);
                else if (PyUnicode_Check(existing))
                    val = PyUnicode_FromString((const char*)buffer);
                else if (PyTuple_Check(existing))
                {
                    Py_ssize_t sz = PyTuple_Size(existing);
                    val = PyTuple_New(sz);
                    for (Py_ssize_t i = 0; i < sz; i++)
                        PyTuple_SetItem(val, i, PyFloat_FromDouble(*(const float*)((const char*)buffer + i * sizeof(float))));
                }
            }
            else
            {
                val = PyFloat_FromDouble(*(const float*)buffer);
            }

            Py_XDECREF(existing);
            if (val)
            {
                PyObject_SetAttrString(ToPy(m_Ref.Get()), name, val);
                Py_DECREF(val);
            }
        }
    }

    ScriptRef FloatToValue(float v)
    {
        return ScriptRef::Adopt(ToSV(PyFloat_FromDouble(v)));
    }

    ScriptRef IntToValue(int32_t v)
    {
        return ScriptRef::Adopt(ToSV(PyLong_FromLong(v)));
    }

    ScriptRef UInt64ToValue(uint64_t v)
    {
        return ScriptRef::Adopt(ToSV(PyLong_FromUnsignedLongLong(v)));
    }

    ScriptRef StringToValue(const std::string_view v)
    {
        return ScriptRef::Adopt(ToSV(PyUnicode_FromStringAndSize(v.data(), (Py_ssize_t)v.size())));
    }

    ScriptRef BoolToValue(bool v)
    {
        PyObject* val = v ? Py_True : Py_False;
        Py_INCREF(val);
        return ScriptRef::Adopt(ToSV(val));
    }

    ScriptRef NoneValue()
    {
        Py_INCREF(Py_None);
        return ScriptRef::Adopt(ToSV(Py_None));
    }

    float ValueToFloat(const ScriptRef& v)
    {
        if (!v.IsValid() || v.IsNone()) return 0.0f;
        GILGuard gil;
        float result = (float)PyFloat_AsDouble(ToPy(v.Get()));
        return result;
    }

    int32_t ValueToInt(const ScriptRef& v)
    {
        if (!v.IsValid() || v.IsNone()) return 0;
        GILGuard gil;
        int32_t result = (int32_t)PyLong_AsLong(ToPy(v.Get()));
        return result;
    }

    uint64_t ValueToUInt64(const ScriptRef& v)
    {
        if (!v.IsValid() || v.IsNone()) return 0;
        GILGuard gil;
        uint64_t result = PyLong_AsUnsignedLongLong(ToPy(v.Get()));
        return result;
    }

    std::string ValueToString(const ScriptRef& v)
    {
        if (!v.IsValid() || v.IsNone()) return {};
        GILGuard gil;
        const char* str = PyUnicode_AsUTF8(ToPy(v.Get()));
        std::string result = str ? str : "";
        return result;
    }

    bool ValueToBool(const ScriptRef& v)
    {
        if (!v.IsValid() || v.IsNone()) return false;
        GILGuard gil;
        bool result = Py_IsTrue(ToPy(v.Get()));
        return result;
    }

    ScriptRef MakeTuple(const ScriptRef* elements, uint32_t count)
    {
        GILGuard gil;
        PyObject* tuple = PyTuple_New((Py_ssize_t)count);
        for (uint32_t i = 0; i < count; i++)
        {
            PyObject* item = ToPy(elements[i].Get());
            Py_INCREF(item);
            PyTuple_SetItem(tuple, (Py_ssize_t)i, item);
        }
        ScriptRef result = ScriptRef::Adopt(ToSV(tuple));
        return result;
    }

    uint32_t GetTupleSize(const ScriptRef& tuple)
    {
        if (!tuple.IsValid()) return 0;
        GILGuard gil;
        uint32_t result = (uint32_t)PyTuple_Size(ToPy(tuple.Get()));
        return result;
    }

    ScriptRef GetTupleElement(const ScriptRef& tuple, uint32_t index)
    {
        if (!tuple.IsValid()) return {};
        GILGuard gil;
        PyErrorSaver saver;
        ScriptRef result;
        PyObject* item = PyTuple_GetItem(ToPy(tuple.Get()), (Py_ssize_t)index);
        if (item)
        {
            Py_INCREF(item);
            result = ScriptRef::Adopt(ToSV(item));
        }
        return result;
    }

    NativeModule::NativeModule(const char* name)
        : m_Name(name)
    {
    }

    NativeModule::~NativeModule()
    {
    }

    void NativeModule::AddFunction(const char* name, NativeFunction func, const char* doc)
    {
        m_Functions.push_back({ name, func, doc ? doc : "" });
    }

    void NativeModule::Register()
    {
        if (m_Functions.empty())
            return;

        GILGuard gil;

        auto data = std::make_unique<PersistentModuleData>();
        data->Name = m_Name;

        data->Methods.reserve(m_Functions.size() + 1);
        data->MethodNames.reserve(m_Functions.size());
        data->MethodDocs.reserve(m_Functions.size());
        for (auto& entry : m_Functions)
        {
            PyCFunction fn = reinterpret_cast<PyCFunction>(entry.Func);
            data->MethodNames.push_back(entry.Name);
            data->MethodDocs.push_back(entry.Doc.empty() ? entry.Doc : "");
            data->Methods.push_back({ data->MethodNames.back().c_str(), fn, METH_VARARGS, data->MethodDocs.back().c_str() });
        }
        data->Methods.push_back({ nullptr, nullptr, 0, nullptr });

        data->Def = {};
        data->Def.m_base = PyModuleDef_HEAD_INIT;
        data->Def.m_name = data->Name.c_str();
        data->Def.m_doc = nullptr;
        data->Def.m_size = -1;
        data->Def.m_methods = data->Methods.data();

        PyObject* mod = PyModule_Create(&data->Def);
        if (mod)
        {
            PyObject* sysModules = PyImport_GetModuleDict();
            PyDict_SetItemString(sysModules, m_Name.c_str(), mod);
            Py_DECREF(mod);
        }

        s_ModuleRegistry.push_back(std::move(data));
    }

} // namespace Prism::Python
