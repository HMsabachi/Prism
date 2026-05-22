#pragma once
#include "Scripting/ScriptObject.h"
#include "Scripting/Python/PythonScriptCore.h"

namespace Prism {

class PythonObject : public ScriptObject
{
public:
    explicit PythonObject(Python::ScriptObject obj)
        : m_Object(std::move(obj)) {}

    ~PythonObject() override
    {
        if (m_Object.IsValid() && m_Object.HasAttribute("OnDestroy"))
            m_Object.Invoke("OnDestroy");
    }

    PythonObject(const PythonObject&) = delete;
    PythonObject& operator=(const PythonObject&) = delete;
    PythonObject(PythonObject&&) = default;
    PythonObject& operator=(PythonObject&&) = default;

    bool IsValid() const override
    {
        return m_Object.IsValid();
    }

    // ---- Implement the 4 type-erased Internal methods ----
    bool TryInvokeMethodInternal(std::string_view name, const void** params, const ScriptType* types, size_t length) const override;
    bool TryInvokeMethodRetInternal(std::string_view name, const void** params, const ScriptType* types, size_t length, void* result) const override;
    void InvokeMethodInternal(std::string_view name, const void** params, const ScriptType* types, size_t length) const override;
    void InvokeMethodRetInternal(std::string_view name, const void** params, const ScriptType* types, size_t length, void* result) const override;

    Python::ScriptObject& GetObject() { return m_Object; }

private:
    mutable Python::ScriptObject m_Object;
};

} // namespace Prism
