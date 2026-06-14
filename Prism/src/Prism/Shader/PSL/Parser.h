#pragma once

#include "TokenStream.h"
#include "AST.h"
#include "Diagnostics.h"

#include <string>
#include <vector>

namespace Prism::PSL
{

struct TypeDesc
{
    PropertyType Type = PropertyType::Float;
    float RangeMin = 0, RangeMax = 1;
    std::vector<std::string> EnumOptions;
};

class Parser
{
public:
    Parser(TokenStream& stream, DiagnosticCollector* diag = nullptr);

    AST::ShaderDocument ParseShader();

private:
    void ParseMaterialLayout(AST::ShaderDocument& doc);

private:
    // Token 流辅助
    Token& Current();
    Token PeekToken(int offset = 1);
    Token Advance();
    bool Check(TokenType type);
    bool Match(TokenType type);
    Token Consume(TokenType type, const std::string& errMsg);
    Token ConsumeNumber(const std::string& errMsg);
    bool IsAtEnd();
    SourceLocation CurrentLoc();
    void Error(const std::string& msg);

    static bool IsTypeToken(TokenType t);
    Token ConsumeType(const std::string& errMsg);

    // Token 文本取值（通过 SourceManager）
    std::string_view TokenText(const Token& t) const;
    std::string TokenStr(const Token& t) const;
    float TokenFloat(const Token& t) const;
    int TokenInt(const Token& t) const;

    // PSL 结构
    void ParseProperties(std::vector<AST::ShaderUniform>& uniforms);
    AST::ShaderUniform ParseProperty();
    TypeDesc ParsePropertyType();
    std::vector<Scalar> ParseDefaultValue(const TypeDesc& type);
    PipelineState ParseRenderCommand();
    void ParsePass(AST::PassDef& pass);
    void ParseTags(std::unordered_map<std::string, std::string>& tags);

    // GLSL 块
    void ParseGLSLBlock(AST::GLSLCode& glsl);

    void BuildMaterialLayout(AST::ShaderDocument& doc);

private:
    void ParserGLSLVoid(AST::GLSLCode& glsl);

    void ParseGLSLAttribute(AST::GLSLCode& glsl, uint32_t id);
    void ParseGLSLVarying(AST::GLSLCode& glsl, uint32_t id);
    void ParseGLSLDirective(AST::GLSLCode& glsl, uint32_t id);
    void FlushSharedChunk(std::string& out, uint32_t& start);
    void AppendTokenText(std::string& out, const Token& t);
    void SkipTo(TokenType type);

    TokenStream& m_Stream;
    DiagnosticCollector* m_Diag = nullptr;
};

} // namespace Prism::PSL
