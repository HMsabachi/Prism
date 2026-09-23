#pragma once
#include "BufferData.h"
#include "../RendererAPI.h"

namespace Prism
{

	class PRISM_API ShaderStorageBufferReadback : public RefCounted
	{
	public:

		virtual ~ShaderStorageBufferReadback() = default;

		virtual bool IsDone() const = 0;

		virtual size_t GetSize() const = 0;

		virtual void GetData(void* data) const = 0;
	};

	class PRISM_API ShaderStorageBuffer : public RefCounted
	{
	public:
		static Ref<ShaderStorageBuffer> Create(uint32_t size, BufferUsage usage = BufferUsage::Dynamic);

	public:

		virtual ~ShaderStorageBuffer() = default;

		virtual void SetData(const void* data, size_t size, size_t offset = 0) = 0;

		virtual Ref<ShaderStorageBufferReadback> RequestReadback(size_t offset = 0, size_t size = 0) = 0;

		virtual size_t GetSize() const = 0;
	};
}
