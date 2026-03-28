#include "prpch.h"
#include "Shader.h"
#include "../RendererAPI.h"
#include "Platform/OpenGL/Shader/OpenGLShader.h"


namespace Prism
{
	Ref<Shader> Shader::Create(const std::string& VertexShader, const std::string& FragmentShader)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None: PR_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLShader>(VertexShader, FragmentShader);
		}
		PR_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
	
