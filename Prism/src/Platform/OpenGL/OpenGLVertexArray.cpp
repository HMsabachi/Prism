#include "prpch.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Prism {

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case Prism::ShaderDataType::Float:    return GL_FLOAT;
		case Prism::ShaderDataType::Float2:   return GL_FLOAT;
		case Prism::ShaderDataType::Float3:   return GL_FLOAT;
		case Prism::ShaderDataType::Float4:   return GL_FLOAT;
		case Prism::ShaderDataType::Mat3:     return GL_FLOAT;
		case Prism::ShaderDataType::Mat4:     return GL_FLOAT;
		case Prism::ShaderDataType::Int:      return GL_INT;
		case Prism::ShaderDataType::Int2:     return GL_INT;
		case Prism::ShaderDataType::Int3:     return GL_INT;
		case Prism::ShaderDataType::Int4:     return GL_INT;
		case Prism::ShaderDataType::Bool:     return GL_BOOL;
		}

		PR_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		PR_RENDER_S({
			glCreateVertexArrays(1, &self->m_RendererID);
			});
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		PR_RENDER_S({
			glDeleteVertexArrays(1, &self->m_RendererID);
			});
	}

	void OpenGLVertexArray::Bind() const
	{
		PR_RENDER_S({
			glBindVertexArray(self->m_RendererID);
			});
	}

	void OpenGLVertexArray::Unbind() const
	{
		PR_RENDER_S({
			glBindVertexArray(0);
			});
	}

	void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
	{
		PR_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		this->Bind();
		vertexBuffer->Bind();

		PR_RENDER_S1(vertexBuffer, {
			auto i = BufferElement::DEFAULT_VERTEX_SEMANTICS;
			const auto& layout = vertexBuffer->GetLayout();
			for (const auto& element : layout)
			{
				int index = element.GetIndex() != BufferElement::DEFAULT_VERTEX_SEMANTICS ? element.GetIndex() : i++;
				PR_CORE_INFO("Adding vertex buffer element with index {0} and name {1}", index, element.Name);
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
			self->m_VertexBufferIndex += i - BufferElement::DEFAULT_VERTEX_SEMANTICS;
			});
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
	{
		Bind();
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}

}