#pragma once
#include <pybind11/pybind11.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Prism::PythonScript {

inline pybind11::object& get_pyglm_module()
{
    static pybind11::object mod = pybind11::module::import("glm");
    return mod;
}

template <typename T, int N>
inline bool load_vecN_via_buffer(pybind11::handle src, T& out)
{
    if (!PyObject_CheckBuffer(src.ptr()))
        return false;
    Py_buffer view;
    if (PyObject_GetBuffer(src.ptr(), &view, PyBUF_SIMPLE) != 0)
        return false;
    bool ok = (view.len >= (Py_ssize_t)(N * sizeof(float))) && (view.itemsize == sizeof(float));
    if (ok)
    {
        float* f = static_cast<float*>(view.buf);
        float* outPtr = glm::value_ptr(out);
        for (int i = 0; i < N; ++i)
            outPtr[i] = f[i];
    }
    PyBuffer_Release(&view);
    return ok;
}

} // namespace Prism::PythonScript

namespace pybind11 {
namespace detail {

template <>
struct type_caster<glm::vec2>
{
    PYBIND11_TYPE_CASTER(glm::vec2, const_name("glm.vec2"));
    bool load(handle src, bool) { return Prism::PythonScript::load_vecN_via_buffer<glm::vec2, 2>(src, value); }
    static handle cast(const glm::vec2& src, return_value_policy, handle)
    {
        return Prism::PythonScript::get_pyglm_module().attr("vec2")(src.x, src.y).release();
    }
};

template <>
struct type_caster<glm::vec3>
{
    PYBIND11_TYPE_CASTER(glm::vec3, const_name("glm.vec3"));
    bool load(handle src, bool) { return Prism::PythonScript::load_vecN_via_buffer<glm::vec3, 3>(src, value); }
    static handle cast(const glm::vec3& src, return_value_policy, handle)
    {
        return Prism::PythonScript::get_pyglm_module().attr("vec3")(src.x, src.y, src.z).release();
    }
};

template <>
struct type_caster<glm::vec4>
{
    PYBIND11_TYPE_CASTER(glm::vec4, const_name("glm.vec4"));
    bool load(handle src, bool) { return Prism::PythonScript::load_vecN_via_buffer<glm::vec4, 4>(src, value); }
    static handle cast(const glm::vec4& src, return_value_policy, handle)
    {
        return Prism::PythonScript::get_pyglm_module().attr("vec4")(src.x, src.y, src.z, src.w).release();
    }
};

template <>
struct type_caster<glm::mat4>
{
    PYBIND11_TYPE_CASTER(glm::mat4, const_name("glm.mat4"));
    bool load(handle src, bool)
    {
        if (!Prism::PythonScript::load_vecN_via_buffer<glm::mat4, 16>(src, value))
            return false;
        value = glm::transpose(value);
        return true;
    }
    static handle cast(const glm::mat4& src, return_value_policy, handle)
    {
        pybind11::object result = Prism::PythonScript::get_pyglm_module().attr("mat4")();
        Py_buffer view;
        if (PyObject_GetBuffer(result.ptr(), &view, PyBUF_WRITABLE) == 0)
        {
            if (view.len >= (Py_ssize_t)sizeof(glm::mat4))
            {
                glm::mat4 colMajor = glm::transpose(src);
                memcpy(view.buf, glm::value_ptr(colMajor), sizeof(glm::mat4));
            }
            PyBuffer_Release(&view);
        }
        return result.release();
    }
};

} // namespace detail
} // namespace pybind11
