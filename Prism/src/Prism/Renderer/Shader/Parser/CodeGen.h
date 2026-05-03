#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <set>
#include "ShaderParserData.h"

namespace Prism
{

class CodeGen
{
public:
    explicit CodeGen(std::string includeRoot);

    // Full pipeline: RawGLSL + Properties → fills PassDescriptor outputs
    void ProcessPass(PassDescriptor& pass, const std::vector<PropertyDescriptor>& properties);

private:
    std::string ResolveIncludes(const std::string& source, std::set<std::filesystem::path>& history);
    static std::string GenerateAttributeDeclarations(const std::vector<GLSLAttribute>& attributes, std::vector<VertexAttributeDescriptor>& outAttributes);
    static std::string PropertyTypeToGLSL(PropertyDeclarationType type);
    static std::string GenerateUniformDeclarations(const std::vector<PropertyDescriptor>& properties);
    static int GetLocationBySemantic(const std::string& semantic);
    void RemoveFunction(std::string& code, const std::string& funcName);
    static std::string StripComments(const std::string& source);
    static void FormatCodeInPlace(std::string& code);

    std::string m_IncludeRoot;
    std::string m_VersionHeader;
};

} // namespace Prism
