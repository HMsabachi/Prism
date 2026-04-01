#include "prpch.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>


namespace Prism
{
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
	uint32_t OpenGLVertexArray::VertexSemanticToOpenGLLocation(VertexSemantic semantic)
	{
		if(semantic == VertexSemantic::Unknown) return 9999;
		return static_cast<uint32_t>(semantic);
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		PR_PROFILE_FUNCTION();

		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		PR_PROFILE_FUNCTION();

		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		PR_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		PR_PROFILE_FUNCTION();

		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
	{
		PR_PROFILE_FUNCTION();

		PR_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");
		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();
		uint32_t index = 10;
		const auto& layout = vertexBuffer->GetLayout();
		for (auto& element : layout)
		{
			uint32_t location = VertexSemanticToOpenGLLocation(element.Semantic) != 9999 ? VertexSemanticToOpenGLLocation(element.Semantic) : index;
			glEnableVertexAttribArray(location);
			glVertexAttribPointer(
				location,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				reinterpret_cast<const void*> (element.Offset)
			);
			if(element.Semantic == VertexSemantic::Unknown)
				index++;
			
		}
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
	{
		PR_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();
		m_IndexBuffer = indexBuffer;
	}
	
}



