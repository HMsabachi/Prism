#pragma once
#include "PythonField.h"
#include <pybind11/pybind11.h>

namespace Prism {

template<typename T>
T PythonField::GetValue() const
{
    auto* inst = static_cast<pybind11::object*>(m_Instance);
    return inst ? inst->attr(m_Name.c_str()).template cast<T>() : m_ValueBuffer.Read<T>();
}

template<typename T>
void PythonField::SetValue(const T& value)
{
    auto* inst = static_cast<pybind11::object*>(m_Instance);
    if (inst)
        inst->attr(m_Name.c_str()) = pybind11::cast(value);
    else
        m_ValueBuffer.Write(&value, sizeof(T));
}

}
