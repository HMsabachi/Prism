#include "prpch.h"
#include "TokenStream.h"

#include <cctype>
#include <cstdlib>

namespace Prism::PSL
{
TokenType TokenStream::LookupKeyword(const char* str, uint32_t len)
{
    switch (len)
    {
    case 2:
        if (CmpN(str, "On"))  return TokenType::OnKw;
        if (CmpN(str, "in"))  return TokenType::InKw;
        break;
    case 3:
        if (CmpN(str, "Off")) return TokenType::OffKw;
        if (CmpN(str, "out")) return TokenType::OutKw;
        if (CmpN(str, "Int"))  return TokenType::IntKw;
        if (CmpN(str, "One"))  return TokenType::OneKw;
        if (CmpN(str, "LOD"))  return TokenType::LODKw;
        break;
    case 4:
        if (CmpN(str, "void")) return TokenType::VoidKw;
        if (CmpN(str, "vec2")) return TokenType::Vec2Kw;
        if (CmpN(str, "vec3")) return TokenType::Vec3Kw;
        if (CmpN(str, "vec4")) return TokenType::Vec4Kw;
        if (CmpN(str, "mat3")) return TokenType::Mat3Kw;
        if (CmpN(str, "mat4")) return TokenType::Mat4Kw;
        if (CmpN(str, "true")) return TokenType::TrueKw;
        if (CmpN(str, "main")) return TokenType::VertKw;
        if (CmpN(str, "vert")) return TokenType::VertKw;
        if (CmpN(str, "frag")) return TokenType::FragKw;
        if (CmpN(str, "Bool")) return TokenType::BoolKw;
        if (CmpN(str, "Back")) return TokenType::BackKw;
        if (CmpN(str, "Cull")) return TokenType::CullKw;
        if (CmpN(str, "Enum")) return TokenType::EnumKw;
        if (CmpN(str, "GLSL")) return TokenType::GLSLKw;
        if (CmpN(str, "Less")) return TokenType::LessKw;
        if (CmpN(str, "Name")) return TokenType::NameKw;
        if (CmpN(str, "Pass")) return TokenType::PassKw;
        if (CmpN(str, "Tags")) return TokenType::TagsKw;
        if (CmpN(str, "Zero")) return TokenType::ZeroKw;
        break;
    case 5:
        if (CmpN(str, "Blend"))  return TokenType::BlendKw;
        if (CmpN(str, "ZTest"))  return TokenType::ZTestKw;
        if (CmpN(str, "Never"))  return TokenType::NeverKw;
        if (CmpN(str, "Equal"))  return TokenType::EqualKw;
        if (CmpN(str, "Front"))  return TokenType::FrontKw;
        if (CmpN(str, "Range"))  return TokenType::RangeKw;
        if (CmpN(str, "Float"))  return TokenType::FloatKw;
        if (CmpN(str, "Color"))  return TokenType::ColorKw;
        if (CmpN(str, "false"))  return TokenType::FalseKw;
        if (CmpN(str, "float"))  return TokenType::FloatGLSLKw;
        if (CmpN(str, "inout"))  return TokenType::InOutKw;
        break;
    case 6:
        if (CmpN(str, "Shader"))   return TokenType::ShaderKw;
        if (CmpN(str, "ZWrite"))   return TokenType::ZWriteKw;
        if (CmpN(str, "Always"))   return TokenType::AlwaysKw;
        if (CmpN(str, "LEqual"))   return TokenType::LEqualKw;
        if (CmpN(str, "GEqual"))   return TokenType::GEqualKw;
        if (CmpN(str, "Color3"))   return TokenType::Color3Kw;
        if (CmpN(str, "Offset"))   return TokenType::OffsetKw;
        if (CmpN(str, "pragma"))   return TokenType::PragmaKw;
        if (CmpN(str, "layout"))   return TokenType::LayoutKw;
        break;
    case 7:
        if (CmpN(str, "Vector2"))  return TokenType::Vector2Kw;
        if (CmpN(str, "Vector3"))  return TokenType::Vector3Kw;
        if (CmpN(str, "Vector4"))  return TokenType::Vector4Kw;
        if (CmpN(str, "Matrix3"))  return TokenType::Matrix3Kw;
        if (CmpN(str, "Matrix4"))  return TokenType::Matrix4Kw;
        if (CmpN(str, "uniform"))  return TokenType::UniformKw;
        if (CmpN(str, "Greater"))  return TokenType::GreaterKw;
        if (CmpN(str, "VARYING"))  return TokenType::VaryingKw;
        if (CmpN(str, "include"))  return TokenType::IncludeKw;
        break;
    case 8:
        if (CmpN(str, "SrcAlpha"))   return TokenType::SrcAlphaKw;
        if (CmpN(str, "NotEqual"))   return TokenType::NotEqualKw;
        break;
    case 9:
        if (CmpN(str, "SubShader"))  return TokenType::SubShaderKw;
        if (CmpN(str, "Texture2D"))  return TokenType::Texture2DKw;
        if (CmpN(str, "ColorMask"))  return TokenType::ColorMaskKw;
        if (CmpN(str, "attribute"))  return TokenType::AttributeKw;
        if (CmpN(str, "sampler2D"))  return TokenType::Sampler2DKw;
        break;
    case 10:
        if (CmpN(str, "Properties")) return TokenType::PropertiesKw;
        break;
    case 11:
        if (CmpN(str, "Texture2DMS"))  return TokenType::Texture2DMSKw;
        if (CmpN(str, "TextureCube"))  return TokenType::TextureCubeKw;
        if (CmpN(str, "samplerCube"))  return TokenType::SamplerCubeKw;
        if (CmpN(str, "sampler2DMS"))  return TokenType::Sampler2DMSKw;
        break;
    case 13:
        if (CmpN(str, "RenderCommand"))  return TokenType::RenderCommandKw;
        if (CmpN(str, "multi_compile"))  return TokenType::MultiCompileKw;
        break;
    case 14:
        if (CmpN(str, "shader_feature")) return TokenType::ShaderFeatureKw;
        break;
    case 16:
        if (CmpN(str, "OneMinusSrcAlpha")) return TokenType::OneMinusSrcAlphaKw;
        if (CmpN(str, "OneMinusDstAlpha")) return TokenType::OneMinusDstAlphaKw;
        break;
    }
    return TokenType::Identifier;
}

// ============================================================
// TokenStream
// ============================================================
TokenStream::TokenStream(const SourceManager& sm, DiagnosticCollector* diag)
    : m_SM(sm), m_Diag(diag)
    , m_Ptr(sm.GetBuffer()), m_ByteOffset(0)
{
    // Prime: lex first token
    m_Buffer.push_back(LexSingle());
}

Token& TokenStream::Current()
{
    return m_Buffer[m_Pos];
}

Token TokenStream::Advance()
{
    Token t = m_Buffer[m_Pos];
    m_Pos++;
    if (m_Pos >= m_Buffer.size())
        m_Buffer.push_back(LexSingle());
    return t;
}

Token& TokenStream::PeekToken(int ahead)
{
    EnsureAvailable(static_cast<size_t>(ahead) + 1);
    return m_Buffer[m_Pos + ahead];
}

bool TokenStream::Check(TokenType t)
{
    return Current().Is(t);
}

bool TokenStream::Match(TokenType t)
{
    if (Check(t)) { Advance(); return true; }
    return false;
}

bool TokenStream::IsAtEnd()
{
    return Current().Is(TokenType::EndOfFile);
}

void TokenStream::EnsureAvailable(size_t ahead)
{
    while (m_Pos + ahead > m_Buffer.size())
        m_Buffer.push_back(LexSingle());
}

// ============================================================
// 空白与注释跳过
// ============================================================
void TokenStream::SkipWhitespaceAndComments()
{
    while (true)
    {
        char c = *m_Ptr;
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
        {
            m_Ptr++; m_ByteOffset++;
            continue;
        }
        if (c == '/' && m_Ptr[1] == '/')
        {
            m_Ptr += 2; m_ByteOffset += 2;
            while (*m_Ptr && *m_Ptr != '\n') { m_Ptr++; m_ByteOffset++; }
            if (*m_Ptr == '\n') { m_Ptr++; m_ByteOffset++; }
            continue;
        }

        if (c == '/' && m_Ptr[1] == '*')
        {
            uint32_t commentStart = m_ByteOffset;
            m_Ptr += 2; m_ByteOffset += 2;
            while (*m_Ptr && !(*m_Ptr == '*' && m_Ptr[1] == '/'))
                { m_Ptr++; m_ByteOffset++; }

            if (*m_Ptr == '\0')
            {
                if (m_Diag)
                    m_Diag->Error("未闭合的块注释", m_SM.GetLocation(commentStart));
                break;
            }

            m_Ptr += 2; m_ByteOffset += 2;
            continue;
        }
        break;
    }
}

// ============================================================
// LexSingle — 主分发
// ============================================================
Token TokenStream::LexSingle()
{
    SkipWhitespaceAndComments();

    if (*m_Ptr == '\0')
        return { TokenType::EndOfFile, m_ByteOffset, 0 };

    uint32_t start = m_ByteOffset;
    char c = *m_Ptr;

    // 字符串
    if (c == '"') return LexString();

    // 数字
    if (isdigit(static_cast<unsigned char>(c)))                      return LexNumber();
    if (c == '.' && isdigit(static_cast<unsigned char>(m_Ptr[1])))   return LexNumber();
    if (c == '-' && isdigit(static_cast<unsigned char>(m_Ptr[1])))   return LexNumber();

    // 标识符
    if (isalpha(static_cast<unsigned char>(c)) || c == '_') return LexIdent();

    // 操作符与标点
    return LexOperator(c);
}

// ============================================================
// 子词法
// ============================================================
Token TokenStream::LexString()
{
    uint32_t start = m_ByteOffset;
    m_Ptr++; m_ByteOffset++;
    uint32_t contentStart = m_ByteOffset;

    while (*m_Ptr && *m_Ptr != '"' && *m_Ptr != '\n')
    {
        if (*m_Ptr == '\\' && m_Ptr[1]) { m_Ptr++; m_ByteOffset++; }
        m_Ptr++; m_ByteOffset++;
    }

    uint32_t contentLen = m_ByteOffset - contentStart;

    if (*m_Ptr == '"')
    {
        m_Ptr++; m_ByteOffset++;
    }
    else
    {
        if (*m_Ptr == '\n') { m_Ptr++; m_ByteOffset++; } // 吞掉未闭合字符串的换行
        if (m_Diag)
            m_Diag->Error("未闭合的字符串字面量", m_SM.GetLocation(start));
    }

    return { TokenType::StringLiteral, contentStart, contentLen };
}

Token TokenStream::LexNumber()
{
    uint32_t start = m_ByteOffset;
    bool isFloat = false;

    if (*m_Ptr == '-') { m_Ptr++; m_ByteOffset++; }

    while (isdigit(static_cast<unsigned char>(*m_Ptr))) { m_Ptr++; m_ByteOffset++; }

    if (*m_Ptr == '.')
    {
        isFloat = true;
        m_Ptr++; m_ByteOffset++;
        while (isdigit(static_cast<unsigned char>(*m_Ptr))) { m_Ptr++; m_ByteOffset++; }
    }

    if (*m_Ptr == 'e' || *m_Ptr == 'E')
    {
        isFloat = true;
        m_Ptr++; m_ByteOffset++;
        if (*m_Ptr == '+' || *m_Ptr == '-') { m_Ptr++; m_ByteOffset++; }
        while (isdigit(static_cast<unsigned char>(*m_Ptr))) { m_Ptr++; m_ByteOffset++; }
    }

    if (*m_Ptr == 'f' || *m_Ptr == 'F') { isFloat = true; m_Ptr++; m_ByteOffset++; }
    else if (*m_Ptr == 'u' || *m_Ptr == 'U') { m_Ptr++; m_ByteOffset++; }

    uint32_t len = m_ByteOffset - start;
    return { isFloat ? TokenType::FloatLiteral : TokenType::IntegerLiteral, start, len };
}

Token TokenStream::LexIdent()
{
    uint32_t start = m_ByteOffset;
    while (isalnum(static_cast<unsigned char>(*m_Ptr)) || *m_Ptr == '_')
        { m_Ptr++; m_ByteOffset++; }

    uint32_t len = m_ByteOffset - start;
    TokenType type = LookupKeyword(m_SM.GetBuffer() + start, len);
    return { type, start, len };
}

Token TokenStream::LexOperator(char c)
{
    uint32_t start = m_ByteOffset;
    m_Ptr++; m_ByteOffset++;

    switch (c)
    {
    case '{': return { TokenType::LeftBrace,   start, 1 };
    case '}': return { TokenType::RightBrace,  start, 1 };
    case '(': return { TokenType::LeftParen,   start, 1 };
    case ')': return { TokenType::RightParen,  start, 1 };
    case '[': return { TokenType::LeftBracket, start, 1 };
    case ']': return { TokenType::RightBracket,start, 1 };
    case ',': return { TokenType::Comma,       start, 1 };
    case ':': return { TokenType::Colon,       start, 1 };
    case ';': return { TokenType::Semicolon,   start, 1 };
    case '?': return { TokenType::Question,    start, 1 };
    case '~': return { TokenType::Tilde,       start, 1 };
    case '#': return { TokenType::Hash,        start, 1 };
    case '.': return { TokenType::Dot,         start, 1 };

    case '=':
        if (*m_Ptr == '=') { m_Ptr++; m_ByteOffset++; return { TokenType::EqualEqual, start, 2 }; }
        return { TokenType::Equals, start, 1 };
    case '!':
        if (*m_Ptr == '=') { m_Ptr++; m_ByteOffset++; return { TokenType::NotEqual, start, 2 }; }
        return { TokenType::Not, start, 1 };
    case '<':
        if (*m_Ptr == '=') { m_Ptr++; m_ByteOffset++; return { TokenType::LessEqual, start, 2 }; }
        if (*m_Ptr == '<') { m_Ptr++; m_ByteOffset++; return { TokenType::LeftShift, start, 2 }; }
        return { TokenType::Less, start, 1 };
    case '>':
        if (*m_Ptr == '=') { m_Ptr++; m_ByteOffset++; return { TokenType::GreaterEqual, start, 2 }; }
        if (*m_Ptr == '>') { m_Ptr++; m_ByteOffset++; return { TokenType::RightShift,  start, 2 }; }
        return { TokenType::Greater, start, 1 };
    case '&':
        if (*m_Ptr == '&') { m_Ptr++; m_ByteOffset++; return { TokenType::AndAnd,         start, 2 }; }
        if (*m_Ptr == '=') { m_Ptr++; m_ByteOffset++; return { TokenType::AmpersandEqual, start, 2 }; }
        return { TokenType::Ampersand, start, 1 };
    case '|':
        if (*m_Ptr == '|') { m_Ptr++; m_ByteOffset++; return { TokenType::OrOr,       start, 2 }; }
        if (*m_Ptr == '=') { m_Ptr++; m_ByteOffset++; return { TokenType::PipeEqual,  start, 2 }; }
        return { TokenType::Pipe, start, 1 };
    case '+':
        if (*m_Ptr == '+') { m_Ptr++; m_ByteOffset++; return { TokenType::PlusPlus,  start, 2 }; }
        if (*m_Ptr == '=') { m_Ptr++; m_ByteOffset++; return { TokenType::PlusEqual, start, 2 }; }
        return { TokenType::Plus, start, 1 };
    case '-':
        if (*m_Ptr == '-') { m_Ptr++; m_ByteOffset++; return { TokenType::MinusMinus, start, 2 }; }
        if (*m_Ptr == '=') { m_Ptr++; m_ByteOffset++; return { TokenType::MinusEqual, start, 2 }; }
        return { TokenType::Minus, start, 1 };
    case '*':
        if (*m_Ptr == '=') { m_Ptr++; m_ByteOffset++; return { TokenType::StarEqual, start, 2 }; }
        return { TokenType::Star, start, 1 };
    case '/':
        if (*m_Ptr == '=') { m_Ptr++; m_ByteOffset++; return { TokenType::SlashEqual, start, 2 }; }
        return { TokenType::Slash, start, 1 };
    case '%':
        if (*m_Ptr == '=') { m_Ptr++; m_ByteOffset++; return { TokenType::PercentEqual, start, 2 }; }
        return { TokenType::Percent, start, 1 };
    case '^':
        if (*m_Ptr == '=') { m_Ptr++; m_ByteOffset++; return { TokenType::CaretEqual, start, 2 }; }
        return { TokenType::Caret, start, 1 };
    }

    if (m_Diag)
        m_Diag->Error(std::string("未识别的字符 '") + c + "'", m_SM.GetLocation(start));
    return { TokenType::Invalid, start, 1 };
}

} // namespace Prism::PSL
