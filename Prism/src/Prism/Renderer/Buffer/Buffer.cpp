#include "prpch.h"

#include "Platform/OpenGL/Buffer/OpenGLBuffer.h"
#include "../Renderer.h"

namespace Prism {

	Prism::Ref<Prism::VertexBuffer> VertexBuffer::Create(void* data, uint32_t size /*= 0*/, BufferUsage usage /*= VertexBufferUsage::Dynamic*/)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:   return nullptr;
		case RendererAPIType::OpenGL:  return Ref<OpenGLVertexBuffer>::Create(data, size, usage);
		}
	}

	Prism::Ref<Prism::VertexBuffer> VertexBuffer::Create(uint32_t size /*= 0*/, BufferUsage usage /*= VertexBufferUsage::Dynamic*/)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    return nullptr;
		case RendererAPIType::OpenGL:  return Ref<OpenGLVertexBuffer>::Create(size, usage);
		}
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(void* data, uint32_t size)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    return nullptr;
		case RendererAPIType::OpenGL:  return Ref<OpenGLIndexBuffer>::Create(data, size);
		}
		return nullptr;

	}
	Ref<IndexBuffer> IndexBuffer::Create(uint32_t size)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    return nullptr;
		case RendererAPIType::OpenGL:  return Ref<OpenGLIndexBuffer>::Create(size);
		}
		PR_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

#pragma region 布局相关

	uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:    return 4;
		case ShaderDataType::Float2:   return 4 * 2;
		case ShaderDataType::Float3:   return 4 * 3;
		case ShaderDataType::Float4:   return 4 * 4;
		case ShaderDataType::Mat3:     return 4 * 3 * 3;
		case ShaderDataType::Mat4:     return 4 * 4 * 4;
		case ShaderDataType::Int:      return 4;
		case ShaderDataType::Int2:     return 4 * 2;
		case ShaderDataType::Int3:     return 4 * 3;
		case ShaderDataType::Int4:     return 4 * 4;
		case ShaderDataType::Bool:     return 1;
		}

		PR_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}
	uint32_t BufferElement::GetComponentCount() const
	{
		switch (Type)
		{
		case ShaderDataType::Float:   return 1;
		case ShaderDataType::Float2:  return 2;
		case ShaderDataType::Float3:  return 3;
		case ShaderDataType::Float4:  return 4;
		case ShaderDataType::Mat3:    return 3 * 3;
		case ShaderDataType::Mat4:    return 4 * 4;
		case ShaderDataType::Int:     return 1;
		case ShaderDataType::Int2:    return 2;
		case ShaderDataType::Int3:    return 3;
		case ShaderDataType::Int4:    return 4;
		case ShaderDataType::Bool:    return 1;
		}

		PR_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}
#pragma endregion
}