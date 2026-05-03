#include "prpch.h"
#include "Parser.h"
#include "Lexer.h"
#include "GLSLParser.h"
#include "Prism/Renderer/Shader/ShaderPropertyDeclaration.h"

namespace Prism
{

Parser::Parser(std::vector<Token> tokens, std::string source)
    : m_Tokens(std::move(tokens))
    , m_Source(std::move(source))
{
}

Token Parser::Peek(size_t ahead) const
{
    size_t index = m_Position + ahead;
    return index < m_Tokens.size() ? m_Tokens[index] : Token{ TokenType::EndOfFile };
}

Token Parser::Advance()
{
    return m_Position < m_Tokens.size() ? m_Tokens[m_Position++] : Token{ TokenType::EndOfFile };
}

Token Parser::Expect(TokenType type)
{
    Token t = Peek();
    if (t.Is(type))
        return Advance();

    m_Errors.push_back({
        "Expected " + std::string(TokenTypeToString(type)) + " but found " + TokenTypeToString(t.Type),
        t.Line, t.Column,
        TokenTypeToString(type),
        TokenTypeToString(t.Type)
    });
    return { TokenType::Invalid };
}

Token Parser::ExpectAny(std::initializer_list<TokenType> types)
{
    Token t = Peek();
    for (auto type : types)
    {
        if (t.Is(type))
            return Advance();
    }

    std::string expected;
    for (auto type : types)
    {
        if (!expected.empty()) expected += " or ";
        expected += TokenTypeToString(type);
    }

    m_Errors.push_back({
        "Expected " + expected + " but found " + TokenTypeToString(t.Type),
        t.Line, t.Column,
        expected, TokenTypeToString(t.Type)
    });
    return { TokenType::Invalid };
}

bool Parser::Match(TokenType type)
{
    if (Peek().Is(type))
    {
        Advance();
        return true;
    }
    return false;
}

void Parser::SkipBalancedBlock()
{
    // Assumes the current token is the opening brace.
    // Consumes tokens until the matching closing brace.
    int depth = 1;
    while (depth > 0 && Peek().IsNot(TokenType::EndOfFile))
    {
        Token t = Advance();
        if (t.Is(TokenType::LeftBrace)) depth++;
        else if (t.Is(TokenType::RightBrace)) depth--;
    }
}

// ======================================================================
// ParseShader — top-level entry point
// ======================================================================
// Grammar:
//   ShaderDefinition ::= "Shader" StringLiteral "{" Properties? RenderCommand? SubShader+ "}"
// ======================================================================

ParseResult Parser::ParseShader()
{
    ParseResult result;
    result.Success = true;

    Expect(TokenType::ShaderKw);
    result.ShaderName = Expect(TokenType::StringLiteral).Value;
    Expect(TokenType::LeftBrace);

    if (Peek().Is(TokenType::PropertiesKw))
    {
        Advance();
        ParsePropertiesBlock(result);
    }

    if (Peek().Is(TokenType::RenderCommandKw))
    {
        Advance();
        ParseRenderCommandBlock(result);
    }

    if (Peek().Is(TokenType::SubShaderKw))
    {
        while (Peek().Is(TokenType::SubShaderKw))
        {
            Advance(); // consume SubShader
            Expect(TokenType::LeftBrace);

            if (Peek().Is(TokenType::RenderCommandKw))
            {
                Advance();
                ParseRenderCommandBlock(result);
            }

            while (Peek().Is(TokenType::PassKw))
                ParsePass(result);

            Expect(TokenType::RightBrace);
        }
    }
    else
    {
        m_Errors.push_back({ "Expected 'SubShader' block", Peek().Line, Peek().Column });
        result.Success = false;
    }

    Expect(TokenType::RightBrace);
    return result;
}

// ======================================================================
// ParsePropertiesBlock
// ======================================================================
// Grammar:
//   PropertiesBlock ::= "{" PropertyDeclaration* "}"
//   PropertyDeclaration ::= Identifier "(" StringLiteral "," PropertyType ")" "=" DefaultValue
// ======================================================================

void Parser::ParsePropertiesBlock(ParseResult& result)
{
    Expect(TokenType::LeftBrace);

    while (Peek().Is(TokenType::Identifier))
    {
        // Error recovery: save position before attempting
        size_t syncPos = m_Position;

        PropertyDescriptor prop;
        prop.Name = Advance().Value; // Identifier

        // Check if this looks like a Property (followed by left paren)
        if (!Peek().Is(TokenType::LeftParen))
        {
            m_Errors.push_back({
                "Expected '(' after property name '" + prop.Name + "'",
                Peek().Line, Peek().Column
            });
            // Skip to next Identifier or closing brace
            while (Peek().IsNot(TokenType::EndOfFile) &&
                   Peek().IsNot(TokenType::RightBrace) &&
                   !Peek().Is(TokenType::Identifier))
                Advance();
            if (Peek().Is(TokenType::RightBrace)) break;
            continue;
        }

        Expect(TokenType::LeftParen);
        prop.DisplayName = Expect(TokenType::StringLiteral).Value;
        Expect(TokenType::Comma);
        ParsePropertyType(prop);
        Expect(TokenType::RightParen);
        Expect(TokenType::Equals);
        ParseDefaultValue(prop);

        result.Properties.push_back(prop);
    }

    Expect(TokenType::RightBrace);
}

// ======================================================================
// ParsePropertyType
// ======================================================================
// Grammar:
//   PropertyType ::= SimpleType | "Range" "(" number "," number ")" | "Enum" "(" Identifier ("," Identifier)* ")"
//   SimpleType  ::= "Bool" | "Color" | "Color3" | "Float" | "Int" | "Vector2" | "Vector3" | "Vector4"
//                 | "Matrix3" | "Matrix4" | "Texture2D" | "Texture2DMS" | "TextureCube"
// ======================================================================

void Parser::ParsePropertyType(PropertyDescriptor& prop)
{
    Token typeToken = Peek();
    if (typeToken.IsNot(TokenType::Identifier))
    {
        m_Errors.push_back({ "Expected property type", typeToken.Line, typeToken.Column,
            "Identifier", TokenTypeToString(typeToken.Type) });
        prop.Type = PropertyDeclarationType::Float;
        return;
    }

    std::string typeStr = Advance().Value;

    // Handle compound types: Range(min, max) and Enum(A, B, ...)
    if (typeStr == "Range")
    {
        Expect(TokenType::LeftParen);
        prop.Min = std::stof(ExpectAny({ TokenType::IntegerLiteral, TokenType::FloatLiteral }).Value);
        Expect(TokenType::Comma);
        prop.Max = std::stof(ExpectAny({ TokenType::IntegerLiteral, TokenType::FloatLiteral }).Value);
        Expect(TokenType::RightParen);
        prop.Type = PropertyDeclarationType::Range;
        return;
    }

    if (typeStr == "Enum")
    {
        Expect(TokenType::LeftParen);
        while (Peek().IsNot(TokenType::RightParen) && Peek().IsNot(TokenType::EndOfFile))
        {
            prop.EnumOptions.push_back(Advance().Value);
            if (Peek().Is(TokenType::Comma))
                Advance();
        }
        Expect(TokenType::RightParen);
        prop.Type = PropertyDeclarationType::Enum;
        return;
    }

    // Simple type — reuse existing string-to-type mapping
    float dummyMin = 0.0f, dummyMax = 1.0f;

    if      (typeStr == "Bool")       prop.Type = PropertyDeclarationType::Bool;
    else if (typeStr == "Color")      prop.Type = PropertyDeclarationType::Color;
    else if (typeStr == "Color3")     prop.Type = PropertyDeclarationType::Color3;
    else if (typeStr == "Float")      prop.Type = PropertyDeclarationType::Float;
    else if (typeStr == "Int")        prop.Type = PropertyDeclarationType::Int;
    else if (typeStr == "Vector2")    prop.Type = PropertyDeclarationType::Vector2;
    else if (typeStr == "Vector3")    prop.Type = PropertyDeclarationType::Vector3;
    else if (typeStr == "Vector4")    prop.Type = PropertyDeclarationType::Vector4;
    else if (typeStr == "Matrix3" || typeStr == "Matrix3X3")  prop.Type = PropertyDeclarationType::Matrix3;
    else if (typeStr == "Matrix4" || typeStr == "Matrix4x4")  prop.Type = PropertyDeclarationType::Matrix4;
    else if (typeStr == "Texture2D")  prop.Type = PropertyDeclarationType::Texture2D;
    else if (typeStr == "Texture2DMS")prop.Type = PropertyDeclarationType::Texture2DMS;
    else if (typeStr == "TextureCube")prop.Type = PropertyDeclarationType::TextureCube;
    else
    {
        m_Errors.push_back({ "Unknown property type '" + typeStr + "', defaulting to Float",
            typeToken.Line, typeToken.Column });
        prop.Type = PropertyDeclarationType::Float;
    }
}

// ======================================================================
// ParseDefaultValue
// ======================================================================
// Grammar:
//   DefaultValue ::= SimpleValue | ParenthesizedTuple | "{" "}"
//   SimpleValue  ::= StringLiteral | Number | BoolLiteral | Identifier
// ======================================================================

void Parser::ParseDefaultValue(PropertyDescriptor& prop)
{
    Token t = Peek();

    if (t.Is(TokenType::LeftParen))
    {
        // Extract parenthesized tuple directly from source to preserve whitespace
        size_t startOff = t.Offset;
        Advance();
        int depth = 1;
        while (depth > 0 && Peek().IsNot(TokenType::EndOfFile))
        {
            Token inner = Advance();
            if (inner.Is(TokenType::LeftParen)) depth++;
            else if (inner.Is(TokenType::RightParen))
            {
                depth--;
                if (depth == 0)
                {
                    size_t endOff = inner.Offset + inner.Length;
                    prop.DefaultValue = m_Source.substr(startOff, endOff - startOff);
                    return;
                }
            }
        }
        // Unterminated tuple
        prop.DefaultValue = m_Source.substr(startOff);
        return;
    }

    if (t.Is(TokenType::StringLiteral))
    {
        // Handle "white" {} pattern for texture defaults
        size_t startOff = t.Offset;
        Advance();
        if (Peek().Is(TokenType::LeftBrace))
        {
            Advance(); // {
            Expect(TokenType::RightBrace); // }
            size_t endOff = m_Tokens[m_Position - 1].Offset + m_Tokens[m_Position - 1].Length;
            prop.DefaultValue = m_Source.substr(startOff, endOff - startOff);
        }
        else
        {
            size_t endOff = t.Offset + t.Length;
            prop.DefaultValue = m_Source.substr(startOff, endOff - startOff);
        }
        return;
    }

    if (t.Is(TokenType::LeftBrace))
    {
        Advance(); // {
        Expect(TokenType::RightBrace); // }
        prop.DefaultValue = "{}";
        return;
    }

    // Single token: number, bool, identifier
    prop.DefaultValue = Advance().Value;
}

// ======================================================================
// ParseRenderCommandBlock
// ======================================================================

void Parser::ParseRenderCommandBlock(ParseResult& result)
{
    Token openBrace = Expect(TokenType::LeftBrace);

    // Extract raw text between braces (handles comments/strings)
    std::string rawContent;
    if (Lexer::ExtractBlockContent(m_Source, openBrace.Offset, rawContent))
        result.RenderCommand = rawContent;

    // Skip tokens inside the block
    SkipBalancedBlock();
}

// ======================================================================
// ParsePass
// ======================================================================
// Grammar:
//   Pass ::= "Pass" "{" Tags? Name? GLSLBlock? "}"
// ======================================================================

void Parser::ParsePass(ParseResult& result)
{
    Expect(TokenType::PassKw);
    Expect(TokenType::LeftBrace);

    PassDescriptor pass;

    if (Match(TokenType::TagsKw))
        ParseTags(pass);

    if (Match(TokenType::NameKw))
        pass.Name = Expect(TokenType::StringLiteral).Value;

    if (Match(TokenType::GLSLKw))
    {
        Token openBrace = Expect(TokenType::LeftBrace);
        pass.GLSLSourceLine = openBrace.Line;
        Lexer::ExtractBlockContent(m_Source, openBrace.Offset, pass.RawGLSL);
        SkipBalancedBlock();

        // Phase 3: 解析 GLSL 内容
        GLSLParser glslParser(pass.RawGLSL);
        pass.GLSL = glslParser.Parse();
    }

    Expect(TokenType::RightBrace);
    result.Passes.push_back(std::move(pass));
}

// ======================================================================
// ParseTags
// ======================================================================
// Grammar:
//   Tags ::= "{" (StringLiteral "=" StringLiteral)* "}"
// ======================================================================

void Parser::ParseTags(PassDescriptor& pass)
{
    Expect(TokenType::LeftBrace);

    while (Peek().Is(TokenType::StringLiteral))
    {
        std::string key = Advance().Value;
        Expect(TokenType::Equals);
        std::string value = Expect(TokenType::StringLiteral).Value;
        pass.Tags[key] = value;
    }

    Expect(TokenType::RightBrace);
}

} // namespace Prism
