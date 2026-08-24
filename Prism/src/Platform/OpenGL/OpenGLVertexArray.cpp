#include "prpch.h"
#include "OpenGLVertexArray.h"

#include "Prism/Renderer/Renderer.h"
#include "Prism/Core/RenderThread.h"
#include "OpenGLVertexBuffer.h"

#include <glad/glad.h>

namespace Prism {

    static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
    {
        switch (type)
        {
        case ShaderDataType::Float:    return GL_FLOAT;
        case ShaderDataType::Float2:   return GL_FLOAT;
        case ShaderDataType::Float3:   return GL_FLOAT;
        case ShaderDataType::Float4:   return GL_FLOAT;
        case ShaderDataType::Mat3:     return GL_FLOAT;
        case ShaderDataType::Mat4:     return GL_FLOAT;
        case ShaderDataType::Int:      return GL_INT;
        case ShaderDataType::Int2:     return GL_INT;
        case ShaderDataType::Int3:     return GL_INT;
        case ShaderDataType::Int4:     return GL_INT;
        case ShaderDataType::Bool:     return GL_BOOL;
        }

        PR_CORE_ASSERT(false, "Unknown ShaderDataType!");
        return 0;
    }

    OpenGLVertexArray::OpenGLVertexArray(std::span<const Ref<VertexBuffer>> vertexBuffers)
    {
        PR_CORE_ASSERT(!vertexBuffers.empty(), "No vertex buffers!");
        PR_CORE_ASSERT(RenderThread::IsCurrentThreadRT(), "OpenGLVertexArray must be created on the render thread!");

        glCreateVertexArrays(1, &m_RendererID);

        uint32_t attribIndex = 0;
        for (uint32_t binding = 0; binding < vertexBuffers.size(); binding++)
        {
            const Ref<VertexBuffer>& vertexBuffer = vertexBuffers[binding];
            PR_CORE_ASSERT(vertexBuffer, "VertexBuffer is null!");

            const VertexBufferLayout& layout = vertexBuffer->GetLayout();
            PR_CORE_ASSERT(layout.GetElements().size(), "Layout is empty!");

            RendererID vboID = vertexBuffer.As<OpenGLVertexBuffer>()->GetRendererID();
            glVertexArrayVertexBuffer(m_RendererID, binding, vboID, 0, layout.GetStride());

            for (const auto& element : layout)
            {
                uint32_t index = element.GetIndex() != VertexBufferElement::DEFAULT_VERTEX_SEMANTICS
                    ? element.GetIndex()
                    : attribIndex++;
                GLenum glBaseType = ShaderDataTypeToOpenGLBaseType(element.Type);
                glEnableVertexArrayAttrib(m_RendererID, index);
                glVertexArrayAttribBinding(m_RendererID, index, binding);
                if (glBaseType == GL_INT)
                {
                    glVertexArrayAttribIFormat(m_RendererID, index,
                        element.GetComponentCount(),
                        glBaseType,
                        element.Offset);
                }
                else
                {
                    glVertexArrayAttribFormat(m_RendererID, index,
                        element.GetComponentCount(),
                        glBaseType,
                        element.Normalized ? GL_TRUE : GL_FALSE,
                        element.Offset);
                }
            }
        }
    }

    OpenGLVertexArray::~OpenGLVertexArray()
    {
        GLuint rendererID = m_RendererID;
        Renderer::SubmitResourceFree([rendererID]()
        {
            glDeleteVertexArrays(1, &rendererID);
        });
    }

    void OpenGLVertexArray::RT_Bind() const
    {
        glBindVertexArray(m_RendererID);
    }

    Ref<OpenGLVertexArray>& OpenGLVertexArrayCache::RT_Get(std::span<const Ref<VertexBuffer>> vertexBuffers)
    {
        constexpr uint64_t FNV_PRIME = 1099511628211ULL;
        uint64_t hash = 14695981039346656037ULL;
        for (const Ref<VertexBuffer>& vertexBuffer : vertexBuffers)
        {
            uint64_t vboID = vertexBuffer.As<OpenGLVertexBuffer>()->GetRendererID();
            hash ^= vboID;                          hash *= FNV_PRIME;
            hash ^= vertexBuffer->GetLayout().GetHash(); hash *= FNV_PRIME;
        }
        hash ^= (uint64_t)vertexBuffers.size();     hash *= FNV_PRIME;

        auto it = m_Cache.find(hash);
        if (it != m_Cache.end()) return it->second;
        m_Cache[hash] = Ref<OpenGLVertexArray>::Create(vertexBuffers);
        return m_Cache[hash];
    }

    Ref<OpenGLVertexArray>& OpenGLVertexArrayCache::RT_Get(const Ref<VertexBuffer>& vertexBuffer)
    {
        return RT_Get(std::span<const Ref<VertexBuffer>>(&vertexBuffer, 1));
    }

}
