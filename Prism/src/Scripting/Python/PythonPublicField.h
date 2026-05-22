#pragma once
#include "Scripting/PublicField.h"
#include "Scripting/ScriptStorage.h"

namespace Prism
{
	// 通过 ScriptGroup::Instance (Python::ScriptObject*) 读写运行时值
	class PythonPublicField : public PublicField
	{
	public:
		PythonPublicField(const std::string& name, FieldType type, ScriptGroup* group);

		bool IsRuntimeAvailable() const override;
		void CopyStoredValueToRuntime() override;

	protected:
		void GetRuntimeValue_Internal(void* outValue) const override;
		void SetRuntimeValue_Internal(const void* value) override;

	private:
		ScriptGroup* m_Group = nullptr;
	};
}
