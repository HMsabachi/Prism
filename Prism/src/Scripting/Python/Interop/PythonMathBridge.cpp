#include "prpch.h"
#include "PythonMathBridge.h"
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
#ifdef _DEBUG
#undef _DEBUG
#endif
#include <Python.h>
#include <glm/gtc/type_ptr.hpp>
#include <cstring>

namespace Prism::Python {

    // ── 缓存 PyGLM 类型 ──
    static PyObject* s_pyglm_vec2_type = nullptr;
    static PyObject* s_pyglm_vec3_type = nullptr;
    static PyObject* s_pyglm_vec4_type = nullptr;
    static PyObject* s_pyglm_mat4_type = nullptr;

    void InitializeMathBridge()
    {
        GILGuard gil;

        if (s_pyglm_vec3_type)
            return;

        PyObject* pyglm = PyImport_ImportModule("pyglm.glm");
        if (!pyglm)
        {
            PyErr_Clear();
            pyglm = PyImport_ImportModule("glm");
        }
        if (!pyglm)
        {
            PR_CORE_ERROR("[PythonMathBridge] 无法导入 pyglm.glm，数学转换不可用");
            PyErr_Clear();
            return;
        }

        s_pyglm_vec2_type = PyObject_GetAttrString(pyglm, "vec2");
        s_pyglm_vec3_type = PyObject_GetAttrString(pyglm, "vec3");
        s_pyglm_vec4_type = PyObject_GetAttrString(pyglm, "vec4");
        s_pyglm_mat4_type = PyObject_GetAttrString(pyglm, "mat4");

        Py_DECREF(pyglm);

        if (!s_pyglm_vec3_type)
            PR_CORE_ERROR("[PythonMathBridge] 未找到 PyGLM vec3 类型");
        if (!s_pyglm_mat4_type)
            PR_CORE_ERROR("[PythonMathBridge] 未找到 PyGLM mat4 类型");

        PR_CORE_TRACE("[PythonMathBridge] PyGLM 类型缓存完毕");
    }

    // ══════════════════════════════════════════════════════
    //  内部辅助
    // ══════════════════════════════════════════════════════

    inline PyObject* ToPy(ScriptValue* v) { return reinterpret_cast<PyObject*>(v); }
    inline ScriptValue* ToSV(PyObject* p) { return reinterpret_cast<ScriptValue*>(p); }

    static PyObject* BuildVec2(float x, float y)
    {
        if (!s_pyglm_vec2_type) return nullptr;
        PyObject* args = Py_BuildValue("(ff)", x, y);
        PyObject* result = PyObject_CallObject(s_pyglm_vec2_type, args);
        Py_DECREF(args);
        return result;
    }

    static PyObject* BuildVec3(float x, float y, float z)
    {
        if (!s_pyglm_vec3_type) return nullptr;
        PyObject* args = Py_BuildValue("(fff)", x, y, z);
        PyObject* result = PyObject_CallObject(s_pyglm_vec3_type, args);
        Py_DECREF(args);
        return result;
    }

    static PyObject* BuildVec4(float x, float y, float z, float w)
    {
        if (!s_pyglm_vec4_type) return nullptr;
        PyObject* args = Py_BuildValue("(ffff)", x, y, z, w);
        PyObject* result = PyObject_CallObject(s_pyglm_vec4_type, args);
        Py_DECREF(args);
        return result;
    }

    static PyObject* BuildMat4(const glm::mat4& m)
    {
        if (!s_pyglm_mat4_type) return nullptr;
        const float* data = glm::value_ptr(m);
        PyObject* args = PyTuple_New(16);
        for (int i = 0; i < 16; i++)
            PyTuple_SetItem(args, i, PyFloat_FromDouble(data[i]));
        PyObject* result = PyObject_CallObject(s_pyglm_mat4_type, args);
        Py_DECREF(args);
        return result;
    }

    static bool ReadBuffer(PyObject* obj, void* out, size_t expected_size)
    {
        if (!obj || !out) return false;
        Py_buffer buf;
        if (PyObject_GetBuffer(obj, &buf, PyBUF_SIMPLE) != 0)
            return false;
        bool ok = (static_cast<size_t>(buf.len) >= expected_size);
        if (ok)
            std::memcpy(out, buf.buf, expected_size);
        PyBuffer_Release(&buf);
        return ok;
    }

    // ══════════════════════════════════════════════════════
    //  标量 C++ → Python
    // ══════════════════════════════════════════════════════

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

    // ══════════════════════════════════════════════════════
    //  标量 Python → C++
    // ══════════════════════════════════════════════════════

    float ValueToFloat(const ScriptRef& v)
    {
        if (!v.IsValid() || v.IsNone()) return 0.0f;
        GILGuard gil;
        return (float)PyFloat_AsDouble(ToPy(v.Get()));
    }

    int32_t ValueToInt(const ScriptRef& v)
    {
        if (!v.IsValid() || v.IsNone()) return 0;
        GILGuard gil;
        return (int32_t)PyLong_AsLong(ToPy(v.Get()));
    }

    uint64_t ValueToUInt64(const ScriptRef& v)
    {
        if (!v.IsValid() || v.IsNone()) return 0;
        GILGuard gil;
        return PyLong_AsUnsignedLongLong(ToPy(v.Get()));
    }

    std::string ValueToString(const ScriptRef& v)
    {
        if (!v.IsValid() || v.IsNone()) return {};
        GILGuard gil;
        const char* str = PyUnicode_AsUTF8(ToPy(v.Get()));
        return str ? str : "";
    }

    bool ValueToBool(const ScriptRef& v)
    {
        if (!v.IsValid() || v.IsNone()) return false;
        GILGuard gil;
        return Py_IsTrue(ToPy(v.Get()));
    }

    // ══════════════════════════════════════════════════════
    //  向量 C++ → Python
    // ══════════════════════════════════════════════════════

    ScriptRef Vec2ToValue(const glm::vec2& v)
    {
        PyObject* obj = BuildVec2(v.x, v.y);
        return obj ? ScriptRef::Adopt(ToSV(obj)) : NoneValue();
    }

    ScriptRef Vec3ToValue(const glm::vec3& v)
    {
        PyObject* obj = BuildVec3(v.x, v.y, v.z);
        return obj ? ScriptRef::Adopt(ToSV(obj)) : NoneValue();
    }

    ScriptRef Vec4ToValue(const glm::vec4& v)
    {
        PyObject* obj = BuildVec4(v.x, v.y, v.z, v.w);
        return obj ? ScriptRef::Adopt(ToSV(obj)) : NoneValue();
    }

    ScriptRef Mat4ToValue(const glm::mat4& m)
    {
        PyObject* obj = BuildMat4(m);
        return obj ? ScriptRef::Adopt(ToSV(obj)) : NoneValue();
    }

    // ══════════════════════════════════════════════════════
    //  向量 Python → C++（Buffer Protocol）
    // ══════════════════════════════════════════════════════

    glm::vec2 ValueToVec2(const ScriptRef& obj)
    {
        glm::vec2 out{};
        ReadBuffer(ToPy(obj.Get()), &out, sizeof(glm::vec2));
        return out;
    }

    glm::vec3 ValueToVec3(const ScriptRef& obj)
    {
        glm::vec3 out{};
        ReadBuffer(ToPy(obj.Get()), &out, sizeof(glm::vec3));
        return out;
    }

    glm::vec4 ValueToVec4(const ScriptRef& obj)
    {
        glm::vec4 out{};
        ReadBuffer(ToPy(obj.Get()), &out, sizeof(glm::vec4));
        return out;
    }

    glm::mat4 ValueToMat4(const ScriptRef& obj)
    {
        glm::mat4 out{};
        ReadBuffer(ToPy(obj.Get()), &out, sizeof(glm::mat4));
        return out;
    }

    // ══════════════════════════════════════════════════════
    //  Tuple 操作
    // ══════════════════════════════════════════════════════

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
        return ScriptRef::Adopt(ToSV(tuple));
    }

    uint32_t GetTupleSize(const ScriptRef& tuple)
    {
        if (!tuple.IsValid()) return 0;
        GILGuard gil;
        return (uint32_t)PyTuple_Size(ToPy(tuple.Get()));
    }

    ScriptRef GetTupleElement(const ScriptRef& tuple, uint32_t index)
    {
        if (!tuple.IsValid()) return {};
        GILGuard gil;
        PyErrorSaver saver;
        PyObject* item = PyTuple_GetItem(ToPy(tuple.Get()), (Py_ssize_t)index);
        if (item)
        {
            Py_INCREF(item);
            return ScriptRef::Adopt(ToSV(item));
        }
        return {};
    }

} // namespace Prism::Python
