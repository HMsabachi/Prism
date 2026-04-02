#include "prpch.h"
#include "Texture.h"

#include "Platform/OpenGL/OpenGLTexture.h"
#include "Legacy/RendererAPI_Legacy.h"
namespace Prism
{
	Ref<Texture2D> Texture2D::ErrorTexture = nullptr;
	void Texture2D::Init()
	{
		ErrorTexture = Create("Assets/Textures/Error.png");
	}
	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (RendererAPI_Legacy::GetAPI())
		{
		case RendererAPI_Legacy::API::None: PR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI_Legacy::API::OpenGL: return CreateRef<OpenGLTexture2D>(path);
		}

		PR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Prism::Ref<Prism::Texture2D> Texture2D::Create(const uint32_t width, const uint32_t height)
	{
		switch (RendererAPI_Legacy::GetAPI())
		{
		case RendererAPI_Legacy::API::None: PR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
		case RendererAPI_Legacy::API::OpenGL: return CreateRef<OpenGLTexture2D>(width, height);
		}
	}

}

