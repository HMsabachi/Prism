#pragma once
#include "Scripting/PublicField.h"
#include "Scripting/Python/PythonScriptCore.h"

namespace Prism
{
	class PythonPublicField : public PublicField
	{
	public:
		PythonPublicField(const std::string& name, FieldType type, Python::ScriptObject* object);

		bool IsRuntimeAvailable() const override;
		void CopyStoredValueToRuntime() override;

		void SetScriptObject(Python::ScriptObject* object) { m_Object = object; }

	protected:
		void GetRuntimeValue_Internal(void* outValue) const override;
		void SetRuntimeValue_Internal(const void* value) override;

	private:
		Python::ScriptObject* m_Object = nullptr;
	};
}
