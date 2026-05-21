#include "prpch.h"
#include "PythonPublicField.h"

namespace Prism
{
	PythonPublicField::PythonPublicField(const std::string& name, FieldType type, Python::ScriptObject* object)
		: PublicField(name, type), m_Object(object)
	{
	}

	bool PythonPublicField::IsRuntimeAvailable() const
	{
		return m_Object && m_Object->IsValid();
	}

	void PythonPublicField::CopyStoredValueToRuntime()
	{
		PR_CORE_ASSERT(m_Object && m_Object->IsValid());
		m_Object->SetFieldRaw(GetName().c_str(), m_StoredValueBuffer);
	}

	void PythonPublicField::GetRuntimeValue_Internal(void* outValue) const
	{
		PR_CORE_ASSERT(m_Object && m_Object->IsValid());
		m_Object->GetFieldRaw(GetName().c_str(), outValue);
	}

	void PythonPublicField::SetRuntimeValue_Internal(const void* value)
	{
		PR_CORE_ASSERT(m_Object && m_Object->IsValid());
		m_Object->SetFieldRaw(GetName().c_str(), value);
	}
}
