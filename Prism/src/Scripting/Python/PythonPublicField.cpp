#include "prpch.h"
#include "PythonPublicField.h"
#include "PythonObject.h"

namespace Prism
{
    PythonPublicField::PythonPublicField(const std::string& name, FieldType type, ScriptGroup* group)
        : PublicField(name, type), m_Group(group)
    {
    }

    bool PythonPublicField::IsRuntimeAvailable() const
    {
        return m_Group && m_Group->Instance && m_Group->Instance->IsValid();
    }

    void PythonPublicField::CopyStoredValueToRuntime()
    {
        PR_CORE_ASSERT(m_Group && m_Group->Instance && m_Group->Instance->IsValid());
        auto& obj = static_cast<PythonObject*>(m_Group->Instance.get())->GetObject();
        obj.SetFieldRaw(GetName().c_str(), m_StoredValueBuffer);
    }

    void PythonPublicField::GetRuntimeValue_Internal(void* outValue) const
    {
        PR_CORE_ASSERT(m_Group && m_Group->Instance && m_Group->Instance->IsValid());
        auto& obj = static_cast<PythonObject*>(m_Group->Instance.get())->GetObject();
        obj.GetFieldRaw(GetName().c_str(), outValue);
    }

    void PythonPublicField::SetRuntimeValue_Internal(const void* value)
    {
        PR_CORE_ASSERT(m_Group && m_Group->Instance && m_Group->Instance->IsValid());
        auto& obj = static_cast<PythonObject*>(m_Group->Instance.get())->GetObject();
        obj.SetFieldRaw(GetName().c_str(), value);
    }
}
