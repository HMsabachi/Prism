#include "prpch.h"
#include "RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Prism
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();
}