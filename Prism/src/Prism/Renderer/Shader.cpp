#include "prpch.h"
#include "Shader.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Prism 
{
	std::vector<Shader*> Shader::s_AllShaders;

	Shader* Shader::Create(const std::string& filepath)
	{
		Shader* result = nullptr;
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None: return nullptr;
		case RendererAPIType::OpenGL: result = new OpenGLShader(filepath);
		}
		s_AllShaders.push_back(result);
		return result;
	}

	Shader* Shader::Create(const std::string& name, const std::string& vertexShader, const std::string& fragmentShader)
	{
		Shader* result = nullptr;
		std::string source = "#type vertex\n" + vertexShader + "#type fragment\n" + fragmentShader;
		PR_CORE_WARN(source);
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None: return nullptr;
		case RendererAPIType::OpenGL: result = new OpenGLShader(name, source);
		}
		s_AllShaders.push_back(result);
		return result;
	}

}