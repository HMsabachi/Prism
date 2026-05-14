#include "prpch.h"
#include "Lexer.h"
#include <unordered_map>

namespace Prism
{

Lexer::Lexer(std::string source)
    : m_Source(std::move(source))
{
}

std::vector<Token> Lexer::Tokenize()
{
    std::vector<Token> tokens;
    while (m_Position < m_Source.size())
    {
        SkipWhitespace();
        if (m_Position >= m_Source.size())
            break;

        if (SkipComment())
            continue;

        char c = m_Source[m_Position];

        if (c == '"')
        {
            tokens.push_back(LexStringLiteral());
        }
        else if (c == '{')
        {
            tokens.push_back({ TokenType::LeftBrace, "{", m_Line, m_Column, m_Position, 1 });
            m_Position++; m_Column++;
        }
        else if (c == '}')
        {
            tokens.push_back({ TokenType::RightBrace, "}", m_Line, m_Column, m_Position, 1 });
            m_Position++; m_Column++;
        }
        else if (c == '(')
        {
            tokens.push_back({ TokenType::LeftParen, "(", m_Line, m_Column, m_Position, 1 });
            m_Position++; m_Column++;
        }
        else if (c == ')')
        {
            tokens.push_back({ TokenType::RightParen, ")", m_Line, m_Column, m_Position, 1 });
            m_Position++; m_Column++;
        }
        else if (c == ',')
        {
            tokens.push_back({ TokenType::Comma, ",", m_Line, m_Column, m_Position, 1 });
            m_Position++; m_Column++;
        }
        else if (c == '=')
        {
            tokens.push_back({ TokenType::Equals, "=", m_Line, m_Column, m_Position, 1 });
            m_Position++; m_Column++;
        }
        else if (c == ':')
        {
            tokens.push_back({ TokenType::Colon, ":", m_Line, m_Column, m_Position, 1 });
            m_Position++; m_Column++;
        }
        else if (c == ';')
        {
            tokens.push_back({ TokenType::Semicolon, ";", m_Line, m_Column, m_Position, 1 });
            m_Position++; m_Column++;
        }
        else if (std::isalpha(c) || c == '_')
        {
            tokens.push_back(LexIdentifierOrKeyword());
        }
        else if (std::isdigit(c) || (c == '-' && m_Position + 1 < m_Source.size() && std::isdigit(m_Source[m_Position + 1])))
        {
            tokens.push_back(LexNumber());
        }
        else
        {
            // Silently skip GLSL-internal characters (# . * / + - [ ] < > ! | & etc.)
            m_Position++; m_Column++;
        }
    }
    tokens.push_back({ TokenType::EndOfFile, "", m_Line, m_Column, m_Position, 0 });
    return tokens;
}

void Lexer::SkipWhitespace()
{
    while (m_Position < m_Source.size())
    {
        char c = m_Source[m_Position];
        if (c == ' ' || c == '\t')
        {
            m_Position++; m_Column++;
        }
        else if (c == '\n')
        {
            m_Position++; m_Line++; m_Column = 1;
        }
        else if (c == '\r')
        {
            m_Position++; // skip CR, let the next char be LF
        }
        else
        {
            break;
        }
    }
}

bool Lexer::SkipComment()
{
    if (m_Position + 1 >= m_Source.size())
        return false;

    char c = m_Source[m_Position];
    char n = m_Source[m_Position + 1];

    if (c == '/' && n == '/')
    {
        m_Position += 2;
        m_Column += 2;
        while (m_Position < m_Source.size() && m_Source[m_Position] != '\n')
        {
            m_Position++; m_Column++;
        }
        return true;
    }

    if (c == '/' && n == '*')
    {
        m_Position += 2;
        m_Column += 2;
        while (m_Position + 1 < m_Source.size())
        {
            if (m_Source[m_Position] == '\n')
            {
                m_Line++; m_Column = 1;
            }
            if (m_Source[m_Position] == '*' && m_Source[m_Position + 1] == '/')
            {
                m_Position += 2;
                m_Column += 2;
                return true;
            }
            m_Position++; m_Column++;
        }
        m_Errors.push_back({ "Unterminated multi-line comment", m_Line, m_Column });
        return true;
    }

    return false;
}

Token Lexer::LexStringLiteral()
{
    size_t startLine = m_Line;
    size_t startCol = m_Column;
    size_t startOffset = m_Position;

    // Skip opening quote
    m_Position++; m_Column++;
    std::string value;

    while (m_Position < m_Source.size())
    {
        char c = m_Source[m_Position];

        if (c == '"')
        {
            m_Position++; m_Column++;
            return { TokenType::StringLiteral, value, startLine, startCol, startOffset, m_Position - startOffset };
        }

        if (c == '\\' && m_Position + 1 < m_Source.size())
        {
            m_Position++; m_Column++;
            char escaped = m_Source[m_Position];
            switch (escaped)
            {
            case 'n':  value += '\n'; break;
            case 't':  value += '\t'; break;
            case '\\': value += '\\'; break;
            case '"':  value += '"';  break;
            default:   value += escaped; break;
            }
            m_Position++; m_Column++;
        }
        else
        {
            if (c == '\n') { m_Line++; m_Column = 1; }
            else m_Column++;
            value += c;
            m_Position++;
        }
    }

    m_Errors.push_back({ "Unterminated string literal", startLine, startCol });
    return { TokenType::StringLiteral, value, startLine, startCol, startOffset, m_Position - startOffset };
}

Token Lexer::LexNumber()
{
    size_t startLine = m_Line;
    size_t startCol = m_Column;
    size_t startOffset = m_Position;
    std::string value;
    bool isFloat = false;

    if (m_Source[m_Position] == '-')
    {
        value += '-';
        m_Position++; m_Column++;
    }

    while (m_Position < m_Source.size() && std::isdigit(m_Source[m_Position]))
    {
        value += m_Source[m_Position];
        m_Position++; m_Column++;
    }

    if (m_Position < m_Source.size() && m_Source[m_Position] == '.')
    {
        isFloat = true;
        value += '.';
        m_Position++; m_Column++;
        while (m_Position < m_Source.size() && std::isdigit(m_Source[m_Position]))
        {
            value += m_Source[m_Position];
            m_Position++; m_Column++;
        }
    }

    return {
        isFloat ? TokenType::FloatLiteral : TokenType::IntegerLiteral,
        value, startLine, startCol, startOffset, m_Position - startOffset
    };
}

Token Lexer::LexIdentifierOrKeyword()
{
    size_t startLine = m_Line;
    size_t startCol = m_Column;
    size_t startOffset = m_Position;
    std::string value;

    while (m_Position < m_Source.size() && (std::isalnum(m_Source[m_Position]) || m_Source[m_Position] == '_'))
    {
        value += m_Source[m_Position];
        m_Position++; m_Column++;
    }

    TokenType type = KeywordType(value);
    return { type, value, startLine, startCol, startOffset, m_Position - startOffset };
}

TokenType Lexer::KeywordType(const std::string& value)
{
    static const std::unordered_map<std::string, TokenType> s_Keywords = {
        {"Shader",          TokenType::ShaderKw},
        {"Properties",      TokenType::PropertiesKw},
        {"RenderCommand",   TokenType::RenderCommandKw},
        {"SubShader",       TokenType::SubShaderKw},
        {"Pass",            TokenType::PassKw},
        {"Tags",            TokenType::TagsKw},
        {"Name",            TokenType::NameKw},
        {"GLSL",            TokenType::GLSLKw},
        {"Cull",            TokenType::CullKw},
        {"ZTest",           TokenType::ZTestKw},
        {"ZWrite",          TokenType::ZWriteKw},
        {"Blend",           TokenType::BlendKw},
        {"On",              TokenType::OnKw},
        {"Off",             TokenType::OffKw},
        {"true",            TokenType::TrueKw},
        {"false",           TokenType::FalseKw},
    };

    auto it = s_Keywords.find(value);
    return it != s_Keywords.end() ? it->second : TokenType::Identifier;
}

bool Lexer::ExtractBlockContent(const std::string& source, size_t openBracePos, std::string& outContent)
{
    if (openBracePos >= source.size() || source[openBracePos] != '{')
        return false;

    bool inSingleLine = false;
    bool inMultiLine = false;
    bool inString = false;
    int depth = 1;
    size_t contentStart = openBracePos + 1;

    for (size_t i = contentStart; i < source.size(); i++)
    {
        char c = source[i];
        char n = (i + 1 < source.size()) ? source[i + 1] : '\0';

        if (inString)
        {
            if (c == '\\')
            {
                i++; // skip escaped character
                continue;
            }
            if (c == '"')
                inString = false;
            continue;
        }
        if (inSingleLine)
        {
            if (c == '\n')
                inSingleLine = false;
            continue;
        }
        if (inMultiLine)
        {
            if (c == '*' && n == '/')
            {
                inMultiLine = false;
                i++;
            }
            continue;
        }

        if (c == '"')
            inString = true;
        else if (c == '/' && n == '/')
        {
            inSingleLine = true;
            i++;
        }
        else if (c == '/' && n == '*')
        {
            inMultiLine = true;
            i++;
        }
        else if (c == '{')
        {
            depth++;
        }
        else if (c == '}')
        {
            depth--;
            if (depth == 0)
            {
                outContent = source.substr(contentStart, i - contentStart);
                return true;
            }
        }
    }

    return false; // Unmatched brace
}

} // namespace Prism
