#pragma once
#include "Prism/Core/Buffer.h"
#include "Scripting/ScriptTypes.h"
#include <string>
#include <vector>
#include <cstring>
#include <Rolky/ManagedObject.hpp>

namespace Rolky { class ManagedObject; }

namespace Prism {

class CSharpField
{
public:
    CSharpField() = default;

    CSharpField(std::string name, ScriptFieldType type)
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
        return m_Instance ? m_Instance->GetFieldValue<T>(m_Name) : m_ValueBuffer.Read<T>();
    }

    template<typename T>
    void SetValue(const T& value)
    {
        if (m_Instance)
        {
            m_Instance->SetFieldValue(m_Name, value);
        }
        else
        {
            m_ValueBuffer.Write(&value, sizeof(T));
        }
    }

    void SetInstance(Rolky::ManagedObject* obj) { m_Instance = obj; }
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
    Rolky::Type* m_ManagedType = nullptr;
    Rolky::ManagedObject* m_Instance = nullptr;
};

}
