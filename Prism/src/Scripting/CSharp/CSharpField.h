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
    }

    const std::string& GetName() const { return m_Name; }
    ScriptFieldType GetType() const { return m_Type; }

    template<typename T>
    T GetValue() const
    {
        if (m_Instance)
            return m_Instance->GetFieldValue<T>(m_Name);
        T val{};
        if (m_Storage.size() >= sizeof(T))
            std::memcpy(&val, m_Storage.data(), sizeof(T));
        return val;
    }

    template<typename T>
    void SetValue(const T& val)
    {
        if (m_Instance)
        {
            m_Instance->SetFieldValue(m_Name, val);
        }
        else
        {
            m_Storage.resize(sizeof(T));
            std::memcpy(m_Storage.data(), &val, sizeof(T));
        }
    }

    void SetInstance(Rolky::ManagedObject* obj) { m_Instance = obj; }
    void ClearInstance() { m_Instance = nullptr; }

    Buffer GetBuffer() const
    {
        return Buffer(m_Storage.empty() ? nullptr : const_cast<byte*>(m_Storage.data()), (uint32_t)m_Storage.size());
    }

    void SetBuffer(const Buffer& buf)
    {
        m_Storage.resize(buf.Size);
        if (buf.Size > 0)
            std::memcpy(m_Storage.data(), buf.Data, buf.Size);
    }

    uint32_t GetBufferSize() const { return (uint32_t)m_Storage.size(); }

    std::string GetStringValue() const
    {
        if (m_Instance)
            return m_Instance->GetFieldValue<std::string>(m_Name);
        if (m_Storage.empty())
            return {};
        return std::string(reinterpret_cast<const char*>(m_Storage.data()), m_Storage.size());
    }

    void SetStringValue(const std::string& val)
    {
        if (m_Instance)
        {
            m_Instance->SetFieldValue(m_Name, val);
        }
        else
        {
            m_Storage.assign(val.begin(), val.end());
            m_Storage.push_back('\0');
        }
    }

private:
    std::string m_Name;
    ScriptFieldType m_Type = ScriptFieldType::None;
    std::vector<uint8_t> m_Storage;
    Rolky::ManagedObject* m_Instance = nullptr;
};

}
