#pragma once
#include "Prism/Renderer/Buffer/UniformBuffer.h"

#include <Glad/glad.h>

namespace Prism
{

	class PRISM_API OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(uint32_t binding, uint32_t size);
		virtual ~OpenGLUniformBuffer();

		virtual void SetData(const Buffer& buffer) override;
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

		virtual void Bind() const override;

		virtual uint32_t GetBinding() const override { return m_Binding; }
		virtual uint32_t GetSize() const override { return m_Size; }
		virtual RendererID GetRendererID() const override { return m_RendererID; }

	private:
		RendererID m_RendererID = 0;
		uint32_t m_Binding;
		uint32_t m_Size;
	};

}
