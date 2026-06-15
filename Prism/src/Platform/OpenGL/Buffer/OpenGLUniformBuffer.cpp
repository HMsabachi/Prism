#include "prpch.h"
#include "OpenGLUniformBuffer.h"

#include "Prism/Renderer/Renderer.h"

namespace Prism
{

    OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t binding, uint32_t size)
        : m_Binding(binding), m_Size(size)
    {
        Ref<OpenGLUniformBuffer> instance = this;
        Renderer::Submit([instance]() mutable {
            glCreateBuffers(1, &instance->m_RendererID);
            glNamedBufferData(instance->m_RendererID, instance->m_Size, nullptr, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, instance->m_Binding, instance->m_RendererID);
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
        Ref<OpenGLUniformBuffer> instance = this;
        Buffer localCopy = buffer;
        Renderer::Submit([instance, localCopy]() {
            glNamedBufferSubData(instance->m_RendererID, 0, localCopy.Size, localCopy.Data);
        });
    }

    void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
    {
        Buffer localCopy = Buffer::Copy(data, size);
        Ref<OpenGLUniformBuffer> instance = this;
        Renderer::Submit([instance, localCopy, offset]() {
            glNamedBufferSubData(instance->m_RendererID, offset, localCopy.Size, localCopy.Data);
        });
    }

    void OpenGLUniformBuffer::Bind() const
    {
        Ref<const OpenGLUniformBuffer> instance = this;
        Renderer::Submit([instance]() {
            glBindBufferBase(GL_UNIFORM_BUFFER, instance->m_Binding, instance->m_RendererID);
        });
    }

}
