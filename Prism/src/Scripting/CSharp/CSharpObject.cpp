#include "prpch.h"
#include "CSharpObject.h"

namespace Prism {

void CSharpObject::InvokeMethodInternal(std::string_view name, const void** params, const ScriptType* types, size_t length) const
{
    if (!m_Handle || !m_Handle->IsValid())
        return;
    static_assert(static_cast<int>(ScriptType::Float) == static_cast<int>(Rolky::ManagedType::Float));
    m_Handle->InvokeMethodInternal(name, params, reinterpret_cast<const Rolky::ManagedType*>(types), length);
}

bool CSharpObject::TryInvokeMethodInternal(std::string_view name, const void** params, const ScriptType* types, size_t length) const
{
    if (!m_Handle || !m_Handle->IsValid())
        return false;

    return m_Handle->TryInvokeMethodInternal(name, params, reinterpret_cast<const Rolky::ManagedType*>(types), length);
}

void CSharpObject::InvokeMethodRetInternal(std::string_view name, const void** params, const ScriptType* types, size_t length, void* result) const
{
    if (!m_Handle || !m_Handle->IsValid())
        return;

    m_Handle->InvokeMethodRetInternal(name, params, reinterpret_cast<const Rolky::ManagedType*>(types), length, result);
}

bool CSharpObject::TryInvokeMethodRetInternal(std::string_view name, const void** params, const ScriptType* types, size_t length, void* result) const
{
    if (!m_Handle || !m_Handle->IsValid())
        return false;

    return m_Handle->TryInvokeMethodRetInternal(name, params, reinterpret_cast<const Rolky::ManagedType*>(types), length, result);
}

} // namespace Prism
