#include "prpch.h"
#include "OpenGLUniformBuffer.h"

#include "Prism/Renderer/Renderer.h"

namespace Prism
{

    OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size)
        : m_Size(size)
    {
        Ref<OpenGLUniformBuffer> instance = this;
        Renderer::Submit([instance]() mutable {
            glCreateBuffers(1, &instance->m_RendererID);
            glNamedBufferData(instance->m_RendererID, instance->m_Size, nullptr, GL_DYNAMIC_DRAW);
        });
    }

    OpenGLUniformBuffer::~OpenGLUniformBuffer()
    {
        GLuint id = m_RendererID;
        Renderer::Submit([id]() {
            glDeleteBuffers(1, &id);
        });
    }

    void OpenGLUniformBuffer::SetData(const Buffer& buffer)
    {
        const void* copy = Renderer::DataAllocate(buffer.Data, buffer.Size);
        const size_t size = buffer.Size;
        Ref<OpenGLUniformBuffer> instance = this;
        Renderer::Submit([instance, copy, size]() {
            glNamedBufferSubData(instance->m_RendererID, 0, size, copy);
            });
    }

    void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
    {
        //BufferSafe localCopy = BufferSafe::Copy(data, size);
        const void* copy = Renderer::DataAllocate(data, size);
        Ref<OpenGLUniformBuffer> instance = this;
        Renderer::Submit([instance, copy, offset, size]() {
            glNamedBufferSubData(instance->m_RendererID, offset, size, copy);
        });
    }

}
