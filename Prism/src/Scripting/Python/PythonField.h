#pragma once
#include "Prism/Core/Buffer.h"
#include "Scripting/ScriptTypes.h"
#include "Scripting/Python/PythonScriptCore.h"
#include <string>
#include <cstring>

namespace Prism::Python { class ScriptObject; }

namespace Prism {

class PythonField
{
public:
    PythonField() = default;

    PythonField(std::string name, ScriptFieldType type)
        : m_Name(std::move(name))
        , m_Type(type)
    {
        m_ValueBuffer.Allocate(DataTypeSize(type));
    }

    const std::string& GetName() const { return m_Name; }
    ScriptFieldType GetType() const { return m_Type; }

    template<typename T>
    T GetValue() const
    {
        if (m_Instance)
            return m_Instance->GetField<T>(m_Name.c_str());
        return m_ValueBuffer.Read<T>();
    }

    template<typename T>
    void SetValue(const T& value)
    {
        if (m_Instance)
        {
            m_Instance->SetField(m_Name.c_str(), value);
        }
        else
        {
            m_ValueBuffer.Write(&value, sizeof(T));
        }
    }

    void SetInstance(Python::ScriptObject* obj) { m_Instance = obj; }
    void ClearInstance() { m_Instance = nullptr; }

    Buffer& GetBuffer()
    {
        return m_ValueBuffer;
    }
    void SetBuffer(const Buffer& buffer)
    {
        m_ValueBuffer = buffer;
    }

    uint32_t GetSize() const { return (uint32_t)m_ValueBuffer.GetSize(); }

private:
    std::string m_Name;
    ScriptFieldType m_Type = ScriptFieldType::None;
    Buffer m_ValueBuffer;
    Python::ScriptObject* m_Instance = nullptr;
};

}
