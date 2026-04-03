#include "prpch.h"
#include "Shader.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Prism {

	Shader* Shader::Create(const std::string& filepath)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None: return nullptr;
		case RendererAPIType::OpenGL: return new OpenGLShader(filepath);
		}
		return nullptr;
	}

}