#pragma once
#include <string>
#include <cstdint>
#include "Scripting/ScriptEngine.h"

namespace Prism
{

	class PublicField
	{
	public:
		PublicField(const std::string& name, FieldType type);
		PublicField(const PublicField&) = delete;
		PublicField(PublicField&& other) noexcept;
		virtual ~PublicField();

		const std::string& GetName() const { return m_Name; }
		FieldType GetType() const { return m_Type; }

		template<typename T>
		T GetStoredValue() const
		{
			T value;
			GetStoredValue_Internal(&value);
			return value;
		}

		template<typename T>
		void SetStoredValue(const T& value)
		{
			SetStoredValue_Internal(&value);
		}

		template<typename T>
		T GetRuntimeValue() const
		{
			T value;
			GetRuntimeValue_Internal(&value);
			return value;
		}

		template<typename T>
		void SetRuntimeValue(const T& value)
		{
			SetRuntimeValue_Internal(&value);
		}

		void SetStoredValueRaw(const void* src);
		const uint8_t* GetStoredValueBuffer() const { return m_StoredValueBuffer; }

		virtual bool IsRuntimeAvailable() const { return false; }
		virtual void CopyStoredValueToRuntime() {}

		static uint32_t GetFieldSize(FieldType type);

	protected:
		void GetStoredValue_Internal(void* outValue) const;
		void SetStoredValue_Internal(const void* value);
		virtual void GetRuntimeValue_Internal(void* outValue) const {}
		virtual void SetRuntimeValue_Internal(const void* value) {}

		std::string m_Name;
		FieldType m_Type;
		uint8_t* m_StoredValueBuffer = nullptr;

	private:
		uint8_t* AllocateBuffer(FieldType type);
	};

}
