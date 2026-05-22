#pragma once
#include "Scripting/ScriptObject.h"
#include <Rolky/ManagedObject.hpp>

namespace Prism {

class CSharpObject : public ScriptObject
{
public:
    explicit CSharpObject(std::unique_ptr<Rolky::ManagedObject> handle)
        : m_Handle(std::move(handle)) {}

    ~CSharpObject() override
    {
        if (m_Handle)
            m_Handle->Destroy();
    }

    CSharpObject(const CSharpObject&) = delete;
    CSharpObject& operator=(const CSharpObject&) = delete;
    CSharpObject(CSharpObject&&) = default;
    CSharpObject& operator=(CSharpObject&&) = default;

    bool IsValid() const override
    {
        return m_Handle && m_Handle->IsValid();
    }

    virtual bool TryInvokeMethodInternal(std::string_view name, const void** params, const ScriptType* types, size_t length) const override;
    virtual bool TryInvokeMethodRetInternal(std::string_view name, const void** params, const ScriptType* types, size_t length, void* result) const override;
    virtual void InvokeMethodInternal(std::string_view name, const void** params, const ScriptType* types, size_t length) const override;
    virtual void InvokeMethodRetInternal(std::string_view name, const void** params, const ScriptType* types, size_t length, void* result) const override;

    Rolky::ManagedObject* GetHandle() const { return m_Handle.get(); }

private:
    std::unique_ptr<Rolky::ManagedObject> m_Handle;

    friend class CSharpScriptEngine;
};

} // namespace Prism
