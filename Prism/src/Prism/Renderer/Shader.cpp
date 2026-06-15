#include "prpch.h"
#include "Shader.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Prism
{
    std::vector<Shader*> Shader::s_AllShaders;

    Shader* Shader::Create(const std::string& vertexSource, const std::string& fragmentSource)
    {
        auto* result = new OpenGLShader(vertexSource, fragmentSource);
        s_AllShaders.push_back(result);
        return result;
    }

    Shader* Shader::Create(const std::string& computeSource)
    {
        auto* result = new OpenGLShader(computeSource);
        s_AllShaders.push_back(result);
        return result;
    }

}
