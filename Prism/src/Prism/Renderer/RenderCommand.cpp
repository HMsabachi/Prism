#include "prpch.h"
#include "RenderCommand.h"
#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Prism
{

	unsigned int RenderCommand::Clear(void* datablock)
	{
		float* data = (float*)datablock;

		float r = *data++;
		float g = *data++;
		float b = *data++;
		float a = *data;

		RendererAPI::Clear(r, g, b, a);

		return sizeof(float) * 4;
	}

	RendererAPI_Legacy* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();
}