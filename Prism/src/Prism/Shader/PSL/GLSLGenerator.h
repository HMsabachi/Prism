#pragma once

#include "AST.h"
#include "Prism/Shader/Property/PropertyType.h"

#include <string>
#include <vector>
#include <sstream>

namespace Prism::PSL
{
namespace GLSLGen
{
    struct Output { std::string Vertex; std::string Fragment; };
    struct Setting
    {
        std::string IncludeRoot = "Assets/Shaders/Include";
        std::string EngineRoot = "Assets/Shaders/Engine";
    };

    void PRISM_API SetSetting(const Setting& setting);

    Output PRISM_API Generate(
        const AST::GLSLCode& glsl,
        const std::vector<AST::ShaderUniform>& uniforms,
        const std::string& filePath
    );
}

} // namespace Prism::PSL
