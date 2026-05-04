#pragma once
#include "Prism/Renderer/Buffer/Buffer.h"
#include "Prism/Core/Buffer.h"

namespace Prism
{

	class PRISM_API OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(void* data, uint32_t size, BufferUsage usage = BufferUsage::Static);
		OpenGLVertexBuffer(uint32_t size, BufferUsage usage = BufferUsage::Dynamic);
		virtual ~OpenGLVertexBuffer();

		virtual void SetData(void* data, uint32_t size, uint32_t offset = 0);
		virtual void Bind() const;

		virtual const VertexBufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const VertexBufferLayout& layout) override { m_Layout = layout; }

		virtual uint32_t GetSize() const { return m_Size; }
		virtual RendererID GetRendererID() const { return m_RendererID; }
	private:
		RendererID m_RendererID = 0;
		uint32_t m_Size;
		BufferUsage m_Usage;
		VertexBufferLayout m_Layout;

		Buffer m_LocalData;
	};

}
