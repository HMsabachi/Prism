#include "prpch.h"
#include "OpenGLShaderStorageBuffer.h"
#include "OpenGLBufferData.h"

#include "Prism/Renderer/Renderer.h"

#include <glad/glad.h>

namespace Prism
{

	OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(size_t size, BufferUsage usage)
		:m_Size(size), m_Usage(usage), m_RendererID(0)
	{
		Renderer::Submit([this]() {
			glCreateBuffers(1, &m_RendererID);
			glNamedBufferData(m_RendererID, m_Size, nullptr, OpenGLUsage(m_Usage));
		});
	}

	OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer()
	{
		GLuint id = m_RendererID;
		Renderer::Submit([id]() {
			if (id) glDeleteBuffers(1, &id);
		});
	}

	void OpenGLShaderStorageBuffer::Bind(uint32_t bindingPoint) const
	{
		Renderer::Submit([this, bindingPoint]() {
			m_BindingPoint = bindingPoint;
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, m_RendererID);
		});
	}

	void OpenGLShaderStorageBuffer::Unbind() const
	{
		Renderer::Submit([this]() {
			m_BindingPoint = 0;
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		});
	}

	void OpenGLShaderStorageBuffer::SetData(const void* data, size_t size, size_t offset /*= 0*/)
	{
		Renderer::Submit([=]() {
			if (data == nullptr || size == 0)
				return;
			glNamedBufferSubData(m_RendererID, static_cast<GLintptr>(offset), size, data);
		});
	}
	void OpenGLShaderStorageBuffer::GetData(void* data, size_t size, size_t offset, bool sync) const
	{
		Renderer::Submit([=]() {
			if (data == nullptr || size == 0) return;
			glGetNamedBufferSubData(m_RendererID, static_cast<GLintptr>(offset), size, data);
		});

		// TODO: 会卡住渲染线程
		if(sync)
			Renderer::WaitAndRender();
	}

	RendererID OpenGLShaderStorageBuffer::GetRendererID() const
	{
		return m_RendererID;
	}
	uint32_t OpenGLShaderStorageBuffer::GetSize() const
	{
		return m_Size;
	}
	uint32_t OpenGLShaderStorageBuffer::GetBindingPoint() const
	{
		return m_BindingPoint;
	}

}