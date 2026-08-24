#include "prpch.h"
#include "OpenGLVertexBuffer.h"
#include "OpenGLBufferData.h"

#include <Glad/glad.h>

#include "Prism/Renderer/Renderer.h"
#include "Prism/Core/RenderThread.h"

namespace Prism
{

    OpenGLVertexBuffer::OpenGLVertexBuffer(void* data, uint32_t size, BufferUsage usage)
        : m_Size(size), m_Usage(usage)
    {
        m_LocalData = Buffer::Copy(data, size);

        if (RenderThread::IsCurrentThreadRT())
        {
            RT_Init();
        }
        else
        {
            Ref<OpenGLVertexBuffer> instance = this;
            Renderer::Submit([instance]() mutable { instance->RT_Init(); });
        }
    }

    OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size, BufferUsage usage)
        : m_Size(size), m_Usage(usage)
    {
        if (RenderThread::IsCurrentThreadRT())
        {
            RT_Init();
        }
        else
        {
            Ref<OpenGLVertexBuffer> instance = this;
            Renderer::Submit([instance]() mutable { instance->RT_Init(); });
        }
    }

    void OpenGLVertexBuffer::RT_Init()
    {
        glCreateBuffers(1, &m_RendererID);
        glNamedBufferData(m_RendererID, m_Size, m_LocalData.Data, OpenGLUsage(m_Usage));
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer()
    {
        GLuint id = m_RendererID;
        Renderer::SubmitResourceFree([id]() {
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

}
