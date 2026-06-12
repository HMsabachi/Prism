#pragma once

#include <cstdint>
#include <string_view>
#include <string>

namespace Prism::PSL
{

class SourceManager;

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
    LeftBrace,          // '{'
    RightBrace,         // '}'
    LeftParen,          // '('
    RightParen,         // ')'
    LeftBracket,        // '['
    RightBracket,       // ']'
    Comma,              // ','
    Dot,                // '.'
    Equals,             // '='
    Colon,              // ':'
    Semicolon,          // ';'
    Hash,               // '#'

    // GLSL operators — 单字符
    Plus,               // '+'
    Minus,              // '-'
    Star,               // '*'
    Slash,              // '/'
    Percent,            // '%'
    Less,               // '<'
    Greater,            // '>'
    Not,                // '!'
    Question,           // '?'
    Ampersand,          // '&'
    Pipe,               // '|'
    Caret,              // '^'
    Tilde,              // '~'

    // GLSL operators — 复合
    PlusPlus,           // '++'
    MinusMinus,         // '--'
    PlusEqual,          // '+='
    MinusEqual,         // '-='
    StarEqual,          // '*='
    SlashEqual,         // '/='
    PercentEqual,       // '%='
    LessEqual,          // '<='
    GreaterEqual,       // '>='
    EqualEqual,         // '=='
    NotEqual,           // '!='
    AndAnd,             // '&&'
    OrOr,               // '||'
    LeftShift,          // '<<'
    RightShift,         // '>>'
    AmpersandEqual,     // '&='
    PipeEqual,          // '|='
    CaretEqual,         // '^='

    // PSL structural keywords
    ShaderKw,
    PropertiesKw,
    RenderCommandKw,
    LODKw,
    SubShaderKw,
    PassKw,
    TagsKw,
    NameKw,
    GLSLKw,

    // Property type keywords
    BoolKw,
    FloatKw,
    IntKw,
    ColorKw,
    Color3Kw,
    Vector2Kw,
    Vector3Kw,
    Vector4Kw,
    Matrix3Kw,
    Matrix4Kw,
    Texture2DKw,
    Texture2DMSKw,
    TextureCubeKw,
    RangeKw,
    EnumKw,

    // Render command keywords
    BlendKw,
    ColorMaskKw,
    CullKw,
    OffsetKw,
    ZTestKw,
    ZWriteKw,
    OnKw,
    OffKw,
    BackKw,
    FrontKw,
    NeverKw,
    LessKw,
    EqualKw,
    LEqualKw,
    GreaterKw,
    NotEqualKw,
    GEqualKw,
    AlwaysKw,
    SrcAlphaKw,
    OneMinusSrcAlphaKw,
    DstAlphaKw,
    OneMinusDstAlphaKw,
    ZeroKw,
    OneKw,

    // Shader entry points
    VertKw,
    FragKw,

    // GLSL keywords
    AttributeKw,
    VaryingKw,
    LayoutKw,
    InKw,
    OutKw,
    InOutKw,
    UniformKw,
    Sampler2DKw,
    SamplerCubeKw,
    Sampler2DMSKw,
    Vec2Kw,
    Vec3Kw,
    Vec4Kw,
    Mat3Kw,
    Mat4Kw,
    VoidKw,
    BoolGLSLKw,
    FloatGLSLKw,
    IntGLSLKw,

    IncludeKw,
    PragmaKw,
    ShaderFeatureKw,
    MultiCompileKw,

    TrueKw,
    FalseKw,
};

struct Token
{
    TokenType Type = TokenType::Invalid;
    uint32_t Offset = 0;   
    uint32_t Length = 0;   

    bool Is(TokenType t) const { return Type == t; }
    bool IsNot(TokenType t) const { return Type != t; }

    // 取值（需要 SourceManager 提供源文本）
    std::string_view View(const SourceManager& sm) const;
    std::string ToString(const SourceManager& sm) const { return std::string(View(sm)); }
};

const char* TokenTypeToString(TokenType type);

} // namespace Prism::PSL
