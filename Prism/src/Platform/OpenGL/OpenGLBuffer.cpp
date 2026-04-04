#include "prpch.h"
#include "OpenGLBuffer.h"

#include <Glad/glad.h>

namespace Prism
{

	//////////////////////////////////////////////////////////////////////////////////
	// VertexBuffer
	//////////////////////////////////////////////////////////////////////////////////

	OpenGLVertexBuffer::OpenGLVertexBuffer(unsigned int size)
		: m_RendererID(0), m_Size(size)
	{
		PR_RENDER_S({
			glGenBuffers(1, &self->m_RendererID);
			});
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		PR_RENDER_S({
			glDeleteBuffers(1, &self->m_RendererID);
			});
	}

	void OpenGLVertexBuffer::SetData(void* buffer, unsigned int size, unsigned int offset)
	{
		m_Size = size;
		PR_RENDER_S3(buffer, size, offset, {
			glBindBuffer(GL_ARRAY_BUFFER, self->m_RendererID);
			glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW);
		});

	}

	void OpenGLVertexBuffer::Bind() const
	{
		PR_RENDER_S({
			glBindBuffer(GL_ARRAY_BUFFER, self->m_RendererID);

		// TODO: 这只是临时的， 默认情况下提供位置和纹理坐标属性
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (const void*)(3 * sizeof(float)));
			});
	}

	//////////////////////////////////////////////////////////////////////////////////
	// IndexBuffer
	//////////////////////////////////////////////////////////////////////////////////

	OpenGLIndexBuffer::OpenGLIndexBuffer(unsigned int size)
		: m_RendererID(0), m_Size(size)
	{
		PR_RENDER_S({
			glGenBuffers(1, &self->m_RendererID);
			});
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		PR_RENDER_S({
			glDeleteBuffers(1, &self->m_RendererID);
			});
	}

	void OpenGLIndexBuffer::SetData(void* buffer, unsigned int size, unsigned int offset)
	{
		m_Size = size;
		PR_RENDER_S3(buffer, size, offset, {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->m_RendererID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW);
			});
	}

	void OpenGLIndexBuffer::Bind() const
	{
		PR_RENDER_S({
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->m_RendererID);
			});
	}

}