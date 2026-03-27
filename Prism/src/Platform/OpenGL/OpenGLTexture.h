#pragma once

#include "Prism/Renderer/Texture.h"

namespace Prism
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D (const std::string& path);
	};
}