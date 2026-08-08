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
    size_t OpenGLShaderStorageBuffer::GetSize() const
    {
        return m_Size;
    }

}
