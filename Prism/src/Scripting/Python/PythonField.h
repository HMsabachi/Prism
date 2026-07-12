#pragma once
#include "Prism/Core/Buffer.h"
#include "Scripting/ScriptTypes.h"
#include <string>
#include <cstring>

namespace pybind11 { class object; }

namespace Prism {

class PythonField
{
public:
    PythonField() = default;

    PythonField(std::string name, ScriptFieldType type, pybind11::object* pyType)
        : m_Name(std::move(name)), m_Type(type), m_PyType(pyType)
    {
        m_ValueBuffer.Allocate(DataTypeSize(type));
    }

    const std::string& GetName() const { return m_Name; }
    ScriptFieldType GetType() const { return m_Type; }
    pybind11::object* GetPyType() const { return m_PyType; }

    template<typename T> T GetValue() const;
    template<typename T> void SetValue(const T& value);

    void SetInstance(void* obj) { m_Instance = obj; }
    void ClearInstance() { m_Instance = nullptr; }
    bool IsRuntime() const { return m_Instance != nullptr; }

    Buffer& GetBuffer() { return m_ValueBuffer; }
    void SetBuffer(const Buffer& buffer) { m_ValueBuffer = buffer; }
    uint32_t GetSize() const { return (uint32_t)m_ValueBuffer.GetSize(); }

private:
    std::string m_Name;
    ScriptFieldType m_Type = ScriptFieldType::None;
    Buffer m_ValueBuffer;
    void* m_Instance = nullptr;
    pybind11::object* m_PyType = nullptr;
};

}
