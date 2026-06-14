#pragma once
#include "Prism/Renderer/RendererAPI.h"
#include "Prism/Core/Buffer.h"

namespace Prism {

	class PRISM_API UniformBuffer : public RefCounted
	{
	public:
		virtual ~UniformBuffer() {}

		virtual void SetData(const Buffer& buffer) = 0;
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

		virtual void Bind() const = 0;

		virtual uint32_t GetBinding() const = 0;
		virtual uint32_t GetSize() const = 0;
		virtual RendererID GetRendererID() const = 0;

		static Ref<UniformBuffer> Create(uint32_t binding, uint32_t size);
	};

}
