#pragma once
#include "Token.h"
#include "ShaderParserData.h"
#include <string>
#include <vector>

namespace Prism
{

struct ParseError
{
    std::string Message;
    size_t Line = 0;
    size_t Column = 0;
    std::string Expected;
    std::string Got;
};

class Parser
{
public:
    Parser(std::vector<Token> tokens, std::string source);

    ParseResult ParseShader();

    const std::vector<ParseError>& GetErrors() const { return m_Errors; }

private:
    // Token helpers
    Token Peek(size_t ahead = 0) const;
    Token Advance();
    Token Expect(TokenType type);
    Token ExpectAny(std::initializer_list<TokenType> types);
    bool Match(TokenType type);

    // Parse rules
    void ParsePropertiesBlock(ParseResult& result);
    void ParseRenderCommandBlock(ParseResult& result);
    void ParseSubShaders(ParseResult& result);
    void ParsePass(ParseResult& result);
    void ParseTags(PassDescriptor& pass);
    void ParsePropertyType(class PropertyDescriptor& prop);
    void ParseDefaultValue(class PropertyDescriptor& prop);

    // Skip balanced pair in token stream (for GLSL/block extraction)
    void SkipBalancedBlock();

    std::vector<Token> m_Tokens;
    std::string m_Source;
    size_t m_Position = 0;
    std::vector<ParseError> m_Errors;
};

} // namespace Prism
