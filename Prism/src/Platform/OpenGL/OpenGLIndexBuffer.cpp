#include "prpch.h"
#include "OpenGLIndexBuffer.h"

#include <Glad/glad.h>

#include "Prism/Renderer/Renderer.h"
#include "Prism/Core/RenderThread.h"

namespace Prism
{

    OpenGLIndexBuffer::OpenGLIndexBuffer(void* data, uint32_t size)
        : m_RendererID(0), m_Size(size)
    {
        m_LocalData = Buffer::Copy(data, size);

        if (RenderThread::IsCurrentThreadRT())
        {
            RT_Init();
        }
        else
        {
            Ref<OpenGLIndexBuffer> instance = this;
            Renderer::Submit([instance]() mutable { instance->RT_Init(); });
        }
    }

    OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t size)
        : m_Size(size)
    {
        if (RenderThread::IsCurrentThreadRT())
        {
            RT_Init();
        }
        else
        {
            Ref<OpenGLIndexBuffer> instance = this;
            Renderer::Submit([instance]() mutable { instance->RT_Init(); });
        }
    }

    void OpenGLIndexBuffer::RT_Init()
    {
        glCreateBuffers(1, &m_RendererID);
        glNamedBufferData(m_RendererID, m_Size, m_LocalData.Data, m_LocalData ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer()
    {
        GLuint id = m_RendererID;
        Renderer::SubmitResourceFree([id]() {
            glDeleteBuffers(1, &id);
        });
    }

    void OpenGLIndexBuffer::SetData(void* data, uint32_t size, uint32_t offset)
    {
        m_Size = size;
        m_LocalData = Buffer::Copy(data, size);
        Ref<OpenGLIndexBuffer> instance = this;
        Renderer::Submit([instance, offset]() {
            glNamedBufferSubData(instance->m_RendererID, offset, instance->m_Size, instance->m_LocalData.Data);
        });
    }

    void OpenGLIndexBuffer::Bind() const
    {
        Ref<const OpenGLIndexBuffer> instance = this;
        Renderer::Submit([instance]() {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance->m_RendererID);
        });
    }


	void OpenGLIndexBuffer::RT_Bind() const
	{
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

}
