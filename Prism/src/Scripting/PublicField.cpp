#include "prpch.h"
#include "PublicField.h"

namespace Prism
{

	PublicField::PublicField(const std::string& name, FieldType type)
		: m_Name(name), m_Type(type)
	{
		m_StoredValueBuffer = AllocateBuffer(type);
	}

	PublicField::PublicField(PublicField&& other) noexcept
		: m_Name(std::move(other.m_Name))
		, m_Type(other.m_Type)
		, m_StoredValueBuffer(other.m_StoredValueBuffer)
	{
		other.m_StoredValueBuffer = nullptr;
	}

	PublicField::~PublicField()
	{
		delete[] m_StoredValueBuffer;
	}

	uint32_t PublicField::GetFieldSize(FieldType type)
	{
		switch (type)
		{
		case FieldType::Float:       return 4;
		case FieldType::Int:         return 4;
		case FieldType::UnsignedInt: return 4;
		case FieldType::String:      return static_cast<uint32_t>(sizeof(std::string));
		case FieldType::Vec2:        return 4 * 2;
		case FieldType::Vec3:        return 4 * 3;
		case FieldType::Vec4:        return 4 * 4;
		}
		PR_CORE_ASSERT(false, "Unknown field type!");
		return 0;
	}

	void PublicField::SetStoredValueRaw(const void* src)
	{
		uint32_t size = GetFieldSize(m_Type);
		memcpy(m_StoredValueBuffer, src, size);
	}

	void PublicField::GetStoredValue_Internal(void* outValue) const
	{
		uint32_t size = GetFieldSize(m_Type);
		memcpy(outValue, m_StoredValueBuffer, size);
	}

	void PublicField::SetStoredValue_Internal(const void* value)
	{
		uint32_t size = GetFieldSize(m_Type);
		memcpy(m_StoredValueBuffer, value, size);
	}

	uint8_t* PublicField::AllocateBuffer(FieldType type)
	{
		uint32_t size = GetFieldSize(type);
		uint8_t* buffer = new uint8_t[size];
		memset(buffer, 0, size);
		return buffer;
	}

}
