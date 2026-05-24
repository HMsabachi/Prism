#pragma once
#include "Prism/Core/Buffer.h"
#include "Scripting/ScriptTypes.h"
#include "Scripting/Python/PythonScriptCore.h"
#include <string>
#include <vector>
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
    }

    const std::string& GetName() const { return m_Name; }
    ScriptFieldType GetType() const { return m_Type; }

    template<typename T>
    T GetValue() const
    {
        if (m_Instance)
            return m_Instance->GetField<T>(m_Name.c_str());
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
            m_Instance->SetField(m_Name.c_str(), val);
        }
        else
        {
            m_Storage.resize(sizeof(T));
            std::memcpy(m_Storage.data(), &val, sizeof(T));
        }
    }

    void SetInstance(Python::ScriptObject* obj) { m_Instance = obj; }
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
            return m_Instance->GetField<std::string>(m_Name.c_str());
        if (m_Storage.empty())
            return {};
        return std::string(reinterpret_cast<const char*>(m_Storage.data()), m_Storage.size());
    }

    void SetStringValue(const std::string& val)
    {
        if (m_Instance)
        {
            m_Instance->SetField(m_Name.c_str(), val);
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
    Python::ScriptObject* m_Instance = nullptr;
};

}
