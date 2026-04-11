#pragma once
#include "Prism/Core/Core.h"

namespace Prism {

	struct PRISM_API Buffer
	{
		mutable bool ReadOnly;
		byte* Data;
		uint32_t Size;

		Buffer()
			: Data(nullptr), Size(0), ReadOnly(false)
		{
		}

		Buffer(byte* data, uint32_t size)
			: Data(data), Size(size), ReadOnly(false)
		{
		}
		void SetReadOnly(bool readOnly) const { ReadOnly = readOnly; }
		
		void Free()
		{
			delete[] Data;
			Data = nullptr;
			Size = 0;
		}
		Buffer Copy() const
		{
			Buffer copy;
			copy.Allocate(Size);
			memcpy(copy.Data, Data, Size);
			return copy;
		}
		static Buffer Copy(void* data, uint32_t size)
		{
			Buffer buffer;
			buffer.Allocate(size);
			memcpy(buffer.Data, data, size);
			return buffer;
		}
		void Allocate(uint32_t size)
		{
			PR_CORE_ASSERT(!ReadOnly, "Cannot allocate a read-only buffer! 无法分配只读缓冲区");
			delete[] Data;
			Data = nullptr;

			if (size == 0)
				return;

			Data = new byte[size];
			Size = size;
		}

		void ZeroInitialize()
		{
			PR_CORE_ASSERT(!ReadOnly, "Cannot zero initialize a read-only buffer! 无法对只读缓冲区进行零初始化");
			if (Data)
				memset(Data, 0, Size);
		}

		void Write(byte* data, uint32_t size, uint32_t offset = 0)
		{
			PR_CORE_ASSERT(!ReadOnly, "Cannot write to a read-only buffer! 无法写入只读缓冲区");
			PR_CORE_ASSERT(offset + size <= Size, "Buffer overflow! 缓冲区溢出");
			memcpy(Data + offset, data, size);
		}

		operator bool() const
		{
			return Data;
		}

		byte& operator[](int index)
		{
			return Data[index];
		}

		byte operator[](int index) const
		{
			return Data[index];
		}

		template<typename T>
		T* As()
		{
			return (T*)Data;
		}

		inline uint32_t GetSize() const { return Size; }
	};

}