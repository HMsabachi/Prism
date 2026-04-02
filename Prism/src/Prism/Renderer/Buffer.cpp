#include "prpch.h"
#include "Buffer.h"
#include "Legacy/RendererAPI_Legacy.h"

#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Prism
{
	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (RendererAPI_Legacy::GetAPI())
		{
		case RendererAPI_Legacy::API::None:    PR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI_Legacy::API::OpenGL:  return new OpenGLVertexBuffer(vertices, size);
		}

		PR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (RendererAPI_Legacy::GetAPI())
		{
		case RendererAPI_Legacy::API::None:    PR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI_Legacy::API::OpenGL:  return new OpenGLIndexBuffer(indices, count);
		}

		PR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}