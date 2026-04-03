#include "prpch.h"
#include "Texture.h"

#include "Prism/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Prism {

	Texture2D* Texture2D::Create(TextureFormat format, unsigned int width, unsigned int height)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None: return nullptr;
		case RendererAPIType::OpenGL: return new OpenGLTexture2D(format, width, height);
		}
		return nullptr;
	}

}