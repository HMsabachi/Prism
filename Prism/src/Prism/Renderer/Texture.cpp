#include "prpch.h"
#include "Texture.h"

#include "Renderer.h"
namespace Prism
{
	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None: PR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		//case RendererAPI::API::OpenGL: return std::make_shared<OpenGLTexture2D>(path);
		}

		PR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}

