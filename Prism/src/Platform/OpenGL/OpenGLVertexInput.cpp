#include "prpch.h"
#include "OpenGLVertexInput.h"

#include "Prism/Renderer/Renderer.h"
#include "Prism/Renderer/Buffer/VertexBuffer.h"

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

    OpenGLVertexInput::OpenGLVertexInput(const VertexInputSpecification& spec)
        : m_Specification(spec)
    {
        Invalidate();
    }

    OpenGLVertexInput::~OpenGLVertexInput()
    {
        GLuint rendererID = m_VertexArrayRendererID;
        Renderer::SubmitResourceFree([rendererID]()
        {
            glDeleteVertexArrays(1, &rendererID);
        });
    }

    void OpenGLVertexInput::Invalidate()
    {
        PR_CORE_ASSERT(m_Specification.Layout.GetElements().size(), "Layout is empty!");

        Ref<OpenGLVertexInput> instance = this;
        Renderer::Submit([instance]() mutable
        {
            auto& vertexArrayRendererID = instance->m_VertexArrayRendererID;

            if (vertexArrayRendererID)
                glDeleteVertexArrays(1, &vertexArrayRendererID);

            glCreateVertexArrays(1, &vertexArrayRendererID);
            glBindVertexArray(0);
        });
    }

    void OpenGLVertexInput::RT_Bind() const
    {
        glBindVertexArray(m_VertexArrayRendererID);
        // Set up vertex attrib pointers using VertexSemantic-based indexing
        const auto& layout = m_Specification.Layout;
        uint32_t attribIndex = 0;
        for (const auto& element : layout)
        {
            int index = element.GetIndex() != VertexBufferElement::DEFAULT_VERTEX_SEMANTICS
                ? element.GetIndex()
                : attribIndex++;
            auto glBaseType = ShaderDataTypeToOpenGLBaseType(element.Type);
            glEnableVertexAttribArray(index);
            if (glBaseType == GL_INT)
            {
                glVertexAttribIPointer(index,
                    element.GetComponentCount(),
                    glBaseType,
                    layout.GetStride(),
                    (const void*)(intptr_t)element.Offset);
            }
            else
            {
                glVertexAttribPointer(index,
                    element.GetComponentCount(),
                    glBaseType,
                    element.Normalized ? GL_TRUE : GL_FALSE,
                    layout.GetStride(),
                    (const void*)(intptr_t)element.Offset);
            }
        }
    }

}
