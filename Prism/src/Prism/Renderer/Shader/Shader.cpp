#include "prpch.h"
#include "Shader.h"
#include "../Legacy/RendererAPI_Legacy.h"
#include "Platform/OpenGL/Shader/OpenGLShader.h"


namespace Prism
{
	Ref<Shader> Shader::Create(const std::string& VertexShader, const std::string& FragmentShader)
	{
		switch (RendererAPI_Legacy::GetAPI())
		{
		case RendererAPI_Legacy::API::None: PR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI_Legacy::API::OpenGL: return std::make_shared<OpenGLShader>(VertexShader, FragmentShader);
		}
		PR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
	
