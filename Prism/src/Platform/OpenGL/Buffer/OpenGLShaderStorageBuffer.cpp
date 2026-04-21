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
		Ref<OpenGLShaderStorageBuffer> instance = this;
		Renderer::Submit([instance]() mutable {
			glCreateBuffers(1, &instance->m_RendererID);
			glNamedBufferData(instance->m_RendererID, instance->m_Size, nullptr, OpenGLUsage(instance->m_Usage));
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
		Ref<const OpenGLShaderStorageBuffer> instance = this;
		Renderer::Submit([instance, bindingPoint]() {
			instance->m_BindingPoint = bindingPoint;
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, instance->m_RendererID);
		});
	}

	void OpenGLShaderStorageBuffer::Unbind() const
	{
		Ref<const OpenGLShaderStorageBuffer> instance = this;
		Renderer::Submit([instance]() {
			instance->m_BindingPoint = 0;
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		});
	}

	void OpenGLShaderStorageBuffer::SetData(const void* data, size_t size, size_t offset /*= 0*/)
	{
		Ref<const OpenGLShaderStorageBuffer> instance = this;
		Renderer::Submit([=]() {
			if (data == nullptr || size == 0)
				return;
			glNamedBufferSubData(instance->m_RendererID, static_cast<GLintptr>(offset), size, data);
		});
	}
	void OpenGLShaderStorageBuffer::GetData(void* data, size_t size, size_t offset, bool sync) const
	{
		Ref<const OpenGLShaderStorageBuffer> instance = this;
		Renderer::Submit([=]() {
			if (data == nullptr || size == 0) return;
			glGetNamedBufferSubData(instance->m_RendererID, static_cast<GLintptr>(offset), size, data);
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