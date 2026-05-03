#pragma once
#include <string>
#include <cstdint>

namespace Prism
{

enum class TokenType : uint8_t
{
    EndOfFile,
    Invalid,

    // Literals
    Identifier,
    StringLiteral,
    IntegerLiteral,
    FloatLiteral,

    // Punctuation
    LeftBrace,      // {
    RightBrace,     // }
    LeftParen,      // (
    RightParen,     // )
    Comma,          // ,
    Equals,         // =
    Colon,          // :
    Semicolon,      // ;

    // Structural keywords
    ShaderKw,
    PropertiesKw,
    RenderCommandKw,
    SubShaderKw,
    PassKw,
    TagsKw,
    NameKw,
    GLSLKw,

    // Render command keywords
    CullKw,
    ZTestKw,
    ZWriteKw,
    BlendKw,
    OnKw,
    OffKw,

    // Value keywords
    TrueKw,
    FalseKw,
};

struct Token
{
    TokenType Type = TokenType::Invalid;
    std::string Value;
    size_t Line = 0;
    size_t Column = 0;
    size_t Offset = 0;
    size_t Length = 0;

    bool Is(TokenType t) const { return Type == t; }
    bool IsNot(TokenType t) const { return Type != t; }
};

inline const char* TokenTypeToString(TokenType type)
{
    switch (type)
    {
    case TokenType::EndOfFile:      return "EOF";
    case TokenType::Invalid:        return "Invalid";
    case TokenType::Identifier:     return "Identifier";
    case TokenType::StringLiteral:  return "StringLiteral";
    case TokenType::IntegerLiteral: return "IntegerLiteral";
    case TokenType::FloatLiteral:   return "FloatLiteral";
    case TokenType::LeftBrace:      return "'{'";
    case TokenType::RightBrace:     return "'}'";
    case TokenType::LeftParen:      return "'('";
    case TokenType::RightParen:     return "')'";
    case TokenType::Comma:          return "','";
    case TokenType::Equals:         return "'='";
    case TokenType::Colon:          return "':'";
    case TokenType::Semicolon:      return "';'";
    case TokenType::ShaderKw:       return "'Shader'";
    case TokenType::PropertiesKw:   return "'Properties'";
    case TokenType::RenderCommandKw:return "'RenderCommand'";
    case TokenType::SubShaderKw:    return "'SubShader'";
    case TokenType::PassKw:         return "'Pass'";
    case TokenType::TagsKw:         return "'Tags'";
    case TokenType::NameKw:         return "'Name'";
    case TokenType::GLSLKw:         return "'GLSL'";
    case TokenType::CullKw:         return "'Cull'";
    case TokenType::ZTestKw:        return "'ZTest'";
    case TokenType::ZWriteKw:       return "'ZWrite'";
    case TokenType::BlendKw:        return "'Blend'";
    case TokenType::OnKw:           return "'On'";
    case TokenType::OffKw:          return "'Off'";
    case TokenType::TrueKw:         return "'true'";
    case TokenType::FalseKw:        return "'false'";
    }
    return "Unknown";
}

} // namespace Prism
