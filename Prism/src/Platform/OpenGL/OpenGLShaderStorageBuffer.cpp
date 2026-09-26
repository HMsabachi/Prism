#include "prpch.h"
#include "OpenGLShaderStorageBuffer.h"
#include "OpenGLBufferData.h"

#include "Prism/Renderer/Renderer.h"
#include "Prism/Core/RenderThread.h"

#include <glad/glad.h>

namespace Prism
{

    OpenGLShaderStorageBufferReadback::OpenGLShaderStorageBufferReadback(size_t size)
        : m_Staging(size)
    {
    }

    bool OpenGLShaderStorageBufferReadback::IsDone() const
    {
        return m_Done.load(std::memory_order_acquire);
    }

    size_t OpenGLShaderStorageBufferReadback::GetSize() const
    {
        return m_Staging.size();
    }

    void OpenGLShaderStorageBufferReadback::GetData(void* data) const
    {
        PR_CORE_ASSERT(IsDone(), "ShaderStorageBufferReadback::GetData 在 IsDone 之前被调用");
        if (data == nullptr || m_Staging.empty())
            return;
        std::memcpy(data, m_Staging.data(), m_Staging.size());
    }

    void OpenGLShaderStorageBufferReadback::RT_ReadFrom(RendererID bufferID, size_t offset)
    {
        if (m_Staging.empty())
            return;
        glGetNamedBufferSubData(bufferID, static_cast<GLintptr>(offset), m_Staging.size(), m_Staging.data());
        m_Done.store(true, std::memory_order_release);
    }

    OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(size_t size, BufferUsage usage)
        :m_Size(size), m_Usage(usage), m_RendererID(0)
    {
        if (RenderThread::IsCurrentThreadRT())
        {
            RT_Init();
        }
        else
        {
            Ref<OpenGLShaderStorageBuffer> instance = this;
            Renderer::Submit([instance]() mutable { instance->RT_Init(); });
        }
    }

    void OpenGLShaderStorageBuffer::RT_Init()
    {
        glCreateBuffers(1, &m_RendererID);
        glNamedBufferData(m_RendererID, m_Size, nullptr, OpenGLUsage(m_Usage));
    }

    OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer()
    {
        GLuint id = m_RendererID;
        Renderer::SubmitResourceFree([id]() {
            if (id) glDeleteBuffers(1, &id);
        });
    }

    void OpenGLShaderStorageBuffer::SetData(const void* data, size_t size, size_t offset /*= 0*/)
    {
        Ref<const OpenGLShaderStorageBuffer> instance = this;
        const void* copy = Renderer::DataAllocate(data, size);
        Renderer::Submit([copy, size, offset, instance]() {
            if (copy == nullptr || size == 0)
                return;
            glNamedBufferSubData(instance->m_RendererID, static_cast<GLintptr>(offset), size, copy);
        });
    }
    Ref<ShaderStorageBufferReadback> OpenGLShaderStorageBuffer::RequestReadback(size_t offset, size_t size)
    {
        if (offset >= m_Size)
        {
            PR_CORE_ERROR("ShaderStorageBuffer::RequestReadback: 起始偏移 {} 超出 buffer 大小 {}", offset, m_Size);
            return nullptr;
        }

        if (size == 0)
        {
            size = m_Size - offset;
        }
        else if (size > m_Size - offset)
        {
            PR_CORE_ERROR("ShaderStorageBuffer::RequestReadback: 读回范围 [{}, {}) 超出 buffer 大小 {}",
                offset, offset + size, m_Size);
            return nullptr;
        }

        Ref<OpenGLShaderStorageBufferReadback> request = Ref<OpenGLShaderStorageBufferReadback>::Create(size);
        Ref<OpenGLShaderStorageBuffer> instance = this;
        Renderer::Submit([instance, request, offset]() mutable
        {
            request->RT_ReadFrom(instance->m_RendererID, offset);
        });
        return request;
    }

    RendererID OpenGLShaderStorageBuffer::GetRendererID() const
    {
        return m_RendererID;
    }
    size_t OpenGLShaderStorageBuffer::GetSize() const
    {
        return m_Size;
    }

}
