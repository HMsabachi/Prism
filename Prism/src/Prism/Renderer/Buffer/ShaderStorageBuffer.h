#pragma once
#include "BufferData.h"
#include "../RendererAPI.h"

namespace Prism
{

	class PRISM_API ShaderStorageBuffer
	{
	public:
		static Ref<ShaderStorageBuffer> Create(uint32_t size, BufferUsage usage = BufferUsage::Dynamic);

	public:
		
		virtual ~ShaderStorageBuffer() = default;

		virtual void Bind(uint32_t bindingPoint) const = 0;
		virtual void Unbind() const = 0;
		virtual uint32_t GetBindingPoint() const = 0;

		virtual void SetData(const void* data, size_t size, size_t offset = 0) = 0;
		// 这里直接调用会卡住渲染进程
		virtual void GetData(void* data, size_t size, size_t offset = 0, bool sync = false) const = 0;

		virtual uint32_t GetSize() const = 0;
		virtual RendererID GetRendererID() const = 0;
	};
}