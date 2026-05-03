#pragma once
#include <string>
#include <vector>
#include "Token.h"

namespace Prism
{

struct GLSLAttribute
{
    std::string Type;
    std::string Name;
    std::string Semantic;
    size_t Line = 0;
    size_t Column = 0;
};

struct GLSLVaryingMember
{
    std::string Type;
    std::string Name;
};

struct GLSLVarying
{
    bool IsStruct = false;
    std::string StructName; // "VertexOutput" for struct, "" for simple
    std::vector<GLSLVaryingMember> Members;
    std::string Type;  // "vec2" for simple, "" for struct
    std::string Name;  // "vUV" for simple, "vs_Output" for struct
    size_t Line = 0;
};

struct GLSLPragma
{
    std::string Text;
    bool IsMultiCompile = false;
    bool IsShaderFeature = false;
    std::vector<std::string> Keywords;
    size_t Line = 0;
};

struct GLSLFunction
{
    std::string ReturnType;
    std::string Name;
    size_t OpenBraceOffset = 0;
    size_t CloseBraceOffset = 0;
    size_t Line = 0;
};

struct GLSLParseResult
{
    std::vector<GLSLAttribute> Attributes;
    std::vector<GLSLVarying> Varyings;
    std::vector<GLSLPragma> Pragmas;
    std::vector<GLSLFunction> Functions;
    std::vector<std::string> Includes;
    std::string RemainingCode;
    std::vector<std::string> Errors;
};

class GLSLParser
{
public:
    explicit GLSLParser(const std::string& source);
    GLSLParseResult Parse();

private:
    std::string StripComments(const std::string& src) const;
    void ParseIncludes(const std::string& clean);
    void ParseAttributes(const std::string& clean);
    void ParseVaryings(const std::string& clean);
    void ParsePragmas(const std::string& clean);
    void ParseFunctions(const std::string& clean);

    const std::string& m_Source;
    GLSLParseResult m_Result;
};

} // namespace Prism
