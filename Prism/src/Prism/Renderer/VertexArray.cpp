#include "prpch.h"
#include "VertexArray.h"

#include "Legacy/Renderer_Legacy.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Prism
{
	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer_Legacy::GetAPI())
		{
		case RendererAPI_Legacy::API::None:		PR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI_Legacy::API::OpenGL:		return CreateRef<OpenGLVertexArray>();
		}
		PR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}