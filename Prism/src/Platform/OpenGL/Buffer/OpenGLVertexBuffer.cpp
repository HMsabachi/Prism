#include "prpch.h"
#include "OpenGLVertexBuffer.h"
#include "OpenGLBufferData.h"

#include <Glad/glad.h>

#include "Prism/Renderer/Renderer.h"

namespace Prism
{

    OpenGLVertexBuffer::OpenGLVertexBuffer(void* data, uint32_t size, BufferUsage usage)
        : m_Size(size), m_Usage(usage)
    {
        m_LocalData = Buffer::Copy(data, size);
        Ref<OpenGLVertexBuffer> instance = this;
        Renderer::Submit([instance]() mutable {
            glCreateBuffers(1, &instance->m_RendererID);
            glNamedBufferData(instance->m_RendererID, instance->m_Size, instance->m_LocalData.Data, OpenGLUsage(instance->m_Usage));
        });
    }

    OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size, BufferUsage usage)
        : m_Size(size), m_Usage(usage)
    {
        Ref<OpenGLVertexBuffer> instance = this;
        Renderer::Submit([instance]() mutable {
            glCreateBuffers(1, &instance->m_RendererID);
            glNamedBufferData(instance->m_RendererID, instance->m_Size, nullptr, OpenGLUsage(instance->m_Usage));
        });
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer()
    {
        GLuint id = m_RendererID;
        Renderer::Submit([id]() {
            glDeleteBuffers(1, &id);
        });
    }

    void OpenGLVertexBuffer::SetData(void* data, uint32_t size, uint32_t offset)
    {
        m_LocalData = Buffer::Copy(data, size);
        m_Size = size;
        Ref<OpenGLVertexBuffer> instance = this;
        Renderer::Submit([instance, offset]() {
            glNamedBufferSubData(instance->m_RendererID, offset, instance->m_LocalData.Size, instance->m_LocalData.Data);
        });

    }

    void OpenGLVertexBuffer::Bind() const
    {
        Ref<const OpenGLVertexBuffer> instance = this;
        Renderer::Submit([instance]() {
            glBindBuffer(GL_ARRAY_BUFFER, instance->m_RendererID);
        });
    }


	void OpenGLVertexBuffer::RT_Bind() const
	{
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

}
