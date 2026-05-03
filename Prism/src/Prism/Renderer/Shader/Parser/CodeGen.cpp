#include "prpch.h"
#include "CodeGen.h"
#include "Prism/Renderer/Shader/ShaderPropertyDeclaration.h"
#include <regex>
#include <sstream>
#include <fstream>
#include <algorithm>

namespace Prism
{

CodeGen::CodeGen(std::string includeRoot)
    : m_IncludeRoot(std::move(includeRoot))
    , m_VersionHeader("#version 450 core\n")
{
}

void CodeGen::ProcessPass(PassDescriptor& pass, const std::vector<PropertyDescriptor>& properties)
{
    // Step 1: Resolve includes
    std::filesystem::path rootPath = std::filesystem::absolute(m_IncludeRoot);
    std::set<std::filesystem::path> history;
    std::string resolved = ResolveIncludes(pass.RawGLSL, history);

    // Step 2: Strip attribute and #pragma declarations from resolved source
    static const std::regex attrStripRegex(R"prism(attribute\s+\w[\w\d]*\s+\w[\w\d]*\s*:\s*\w[\w\d]*\s*;)prism");
    resolved = std::regex_replace(resolved, attrStripRegex, "");
    static const std::regex pragmaLineRegex(R"prism(#pragma\s+.+)prism");
    resolved = std::regex_replace(resolved, pragmaLineRegex, "");

    // Step 3: Generate attribute layout declarations from GLSLParser data
    pass.Attributes.clear();
    std::string attrDeclarations = GenerateAttributeDeclarations(pass.GLSL.Attributes, pass.Attributes);

    // Step 4: Generate header (attributes + uniforms + VARYING defines + #line)
    std::string header = attrDeclarations;
    header += GenerateUniformDeclarations(properties);
    header += "\n";
    header += "#ifdef PRISM_VERTEX_SHADER\n";
    header += "    #define VARYING out\n";
    header += "#else\n";
    header += "    #define VARYING in\n";
    header += "    layout(location = 0) out vec4 FragColor;\n";
    header += "#endif\n\n";

    // #line directive: map next line back to original .Shader file
    if (pass.GLSLSourceLine > 0)
        header += "#line " + std::to_string(pass.GLSLSourceLine + 1) + "\n";

    std::string baseCode = header + resolved;

    // Step 5: Build VS code (version + define + baseCode - frag())
    std::string vsCode = m_VersionHeader + "#define PRISM_VERTEX_SHADER\n" + baseCode;
    RemoveFunction(vsCode, "frag");
    pass.VertexShaderCode = vsCode;

    // Step 6: Build FS code (version + define + baseCode - main() - attrs + frag→main)
    std::string fsCode = m_VersionHeader + "#define PRISM_FRAGMENT_SHADER\n" + baseCode;
    RemoveFunction(fsCode, "main");

    // Remove layout(location=N) in declarations from fragment shader
    static const std::regex attrCleanupRegex(R"prism(layout\s*\(\s*location\s*=\s*\d+\s*\)\s*in\s+[^;]+;)prism");
    fsCode = std::regex_replace(fsCode, attrCleanupRegex, "");

    // Rename void frag() to void main()
    static const std::regex fragSignatureRegex(R"prism(void\s+frag\s*\(\s*\))prism");
    fsCode = std::regex_replace(fsCode, fragSignatureRegex, "void main()");

    pass.FragmentShaderCode = fsCode;

    // Step 7: Post-process
    pass.VertexShaderCode = StripComments(pass.VertexShaderCode);
    pass.FragmentShaderCode = StripComments(pass.FragmentShaderCode);
    FormatCodeInPlace(pass.VertexShaderCode);
    FormatCodeInPlace(pass.FragmentShaderCode);
}

std::string CodeGen::ResolveIncludes(const std::string& source, std::set<std::filesystem::path>& history)
{
    static std::regex includeRegex(R"prism(#include\s+"([^"]+)")prism");
    std::string processed;
    std::istringstream stream(source);
    std::string line;

    while (std::getline(stream, line))
    {
        std::smatch match;
        if (std::regex_search(line, match, includeRegex))
        {
            std::filesystem::path fileName = match[1].str();
            std::filesystem::path fullPath = std::filesystem::path(m_IncludeRoot) / fileName;

            if (history.find(fullPath) != history.end())
                continue;

            std::ifstream file(fullPath);
            if (!file.is_open())
            {
                processed += "// Error: Include not found: " + fileName.string() + "\n";
                continue;
            }

            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();

            history.insert(fullPath);
            processed += ResolveIncludes(content, history) + "\n";
        }
        else
        {
            processed += line + "\n";
        }
    }
    return processed;
}

std::string CodeGen::GenerateAttributeDeclarations(const std::vector<GLSLAttribute>& attributes, std::vector<VertexAttributeDescriptor>& outAttributes)
{
    std::string result;
    for (const auto& attr : attributes)
    {
        int location = GetLocationBySemantic(attr.Semantic);

        VertexAttributeDescriptor desc;
        desc.Type = attr.Type;
        desc.Name = attr.Name;
        desc.SemanticStr = attr.Semantic;
        desc.Location = location;
        outAttributes.push_back(std::move(desc));

        result += "layout(location = " + std::to_string(location) + ") in " + attr.Type + " " + attr.Name + ";\n";
    }
    return result;
}

std::string CodeGen::PropertyTypeToGLSL(PropertyDeclarationType type)
{
    switch (type)
    {
    case PropertyDeclarationType::Bool:      return "uniform bool";
    case PropertyDeclarationType::Color:     return "uniform vec4";
    case PropertyDeclarationType::Color3:    return "uniform vec3";
    case PropertyDeclarationType::Enum:      return "uniform int";
    case PropertyDeclarationType::Float:     return "uniform float";
    case PropertyDeclarationType::Int:       return "uniform int";
    case PropertyDeclarationType::Vector2:   return "uniform vec2";
    case PropertyDeclarationType::Vector3:   return "uniform vec3";
    case PropertyDeclarationType::Vector4:   return "uniform vec4";
    case PropertyDeclarationType::Texture2D: return "uniform sampler2D";
    case PropertyDeclarationType::Texture2DMS: return "uniform sampler2DMS";
    case PropertyDeclarationType::TextureCube: return "uniform samplerCube";
    case PropertyDeclarationType::Range:     return "uniform float";
    case PropertyDeclarationType::Matrix3:   return "uniform mat3";
    case PropertyDeclarationType::Matrix4:   return "uniform mat4";
    default:                                 return "uniform float";
    }
}

std::string CodeGen::GenerateUniformDeclarations(const std::vector<PropertyDescriptor>& properties)
{
    std::string result;
    for (const auto& prop : properties)
        result += PropertyTypeToGLSL(prop.Type) + " " + prop.Name + ";\n";
    return result;
}

int CodeGen::GetLocationBySemantic(const std::string& semantic)
{
    std::string upper = semantic;
    std::transform(semantic.begin(), semantic.end(), upper.begin(), ::toupper);
    if (upper == "POSITION")    return 0;
    if (upper == "NORMAL")      return 1;
    if (upper == "TANGENT")     return 2;
    if (upper == "BINORMAL")    return 3;
    if (upper == "TEXCOORD0")   return 4;
    if (upper == "TEXCOORD1")   return 5;
    if (upper == "BONEINDICES") return 6;
    if (upper == "BONEWEIGHTS") return 7;
    if (upper == "INSTANCEID")  return 8;
    if (upper == "COLOR")       return 9;
    if (upper == "INDEX0")      return 10;
    if (upper == "INDEX1")      return 11;
    if (upper == "OTHER0")      return 12;
    if (upper == "OTHER1")      return 13;
    if (upper == "OTHER2")      return 14;
    return -1;
}

void CodeGen::RemoveFunction(std::string& code, const std::string& funcName)
{
    std::regex funcHeadRegex("void\\s+" + funcName + "\\s*\\(\\s*\\)\\s*\\{");
    std::smatch match;

    if (std::regex_search(code, match, funcHeadRegex))
    {
        size_t startPos = match.position();
        size_t openBracePos = startPos + match.length() - 1;

        int braceCount = 1;
        size_t endPos = std::string::npos;
        for (size_t i = openBracePos + 1; i < code.size(); ++i)
        {
            if (code[i] == '{') braceCount++;
            else if (code[i] == '}') braceCount--;
            if (braceCount == 0) { endPos = i; break; }
        }

        if (endPos != std::string::npos)
            code.erase(startPos, endPos - startPos + 1);
    }
}

std::string CodeGen::StripComments(const std::string& source)
{
    std::string result;
    result.reserve(source.size());
    bool inSingleLine = false;
    bool inMultiLine = false;
    bool inString = false;

    for (size_t i = 0; i < source.size(); ++i)
    {
        char c = source[i];
        char next = (i + 1 < source.size()) ? source[i + 1] : '\0';

        if (!inSingleLine && !inMultiLine)
        {
            if (c == '"' && (i == 0 || source[i - 1] != '\\'))
                inString = !inString;
        }

        if (inString)
        {
            result += c;
            continue;
        }

        if (!inMultiLine && !inSingleLine && c == '/' && next == '/')
        {
            inSingleLine = true;
            i++; continue;
        }
        if (inSingleLine && c == '\n')
        {
            inSingleLine = false;
            result += c; continue;
        }

        if (!inSingleLine && !inMultiLine && c == '/' && next == '*')
        {
            inMultiLine = true;
            i++; continue;
        }
        if (inMultiLine && c == '*' && next == '/')
        {
            inMultiLine = false;
            i++; continue;
        }

        if (!inSingleLine && !inMultiLine)
            result += c;
        else if (inMultiLine && c == '\n')
            result += c;
    }
    return result;
}

void CodeGen::FormatCodeInPlace(std::string& code)
{
    std::istringstream stream(code);
    std::string line;
    std::string result;
    int indent = 0;

    auto trim = [](const std::string& s) -> std::string {
        auto start = s.find_first_not_of(" \t\n\r");
        auto end = s.find_last_not_of(" \t\n\r");
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    };

    while (std::getline(stream, line))
    {
        line = trim(line);
        if (line.empty()) continue;

        if (line.find('}') != std::string::npos)
            for (char c : line) if (c == '}') indent--;

        for (int i = 0; i < (std::max)(0, indent); ++i) result += "    ";
        result += line + "\n";

        if (line.find('{') != std::string::npos)
            for (char c : line) if (c == '{') indent++;
    }
    code = result;
}

} // namespace Prism
