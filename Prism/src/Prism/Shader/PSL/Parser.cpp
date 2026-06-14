#include "prpch.h"
#include "Parser.h"
#include "Prism/Utilities/Utilities.h"

namespace Prism::PSL
{

Parser::Parser(TokenStream& stream, DiagnosticCollector* diag)
    : m_Stream(stream), m_Diag(diag)
{
}

AST::ShaderDocument Parser::ParseShader()
{
    AST::ShaderDocument doc;

    Consume(TokenType::ShaderKw, "期望 'Shader' 关键字");
    doc.ShaderName = TokenStr(Consume(TokenType::StringLiteral, "期望 Shader 名称字符串"));
    Consume(TokenType::LeftBrace, "期望 '{'");

    while (!IsAtEnd() && !Check(TokenType::RightBrace))
    {
        if (Check(TokenType::PropertiesKw))
        {
            Advance();
            Consume(TokenType::LeftBrace, "期望 '{'");
            ParseProperties(doc.Uniforms);
            Consume(TokenType::RightBrace, "期望 '}'");
        }
        else if (Check(TokenType::RenderCommandKw))
        {
            Advance();
            Consume(TokenType::LeftBrace, "期望 '{'");
            doc.RenderState = ParseRenderCommand();
            Consume(TokenType::RightBrace, "期望 '}'");
        }
        else if (Check(TokenType::SubShaderKw))
        {
            Advance();
            Consume(TokenType::LeftBrace, "期望 '{'");

            while (!IsAtEnd() && !Check(TokenType::RightBrace))
            {
                if (Check(TokenType::LODKw))
                {
                    Advance();
                    doc.LOD = TokenInt(ConsumeNumber("期望 LOD 数值"));
                }
                else if (Check(TokenType::PassKw))
                {
                    Advance();
                    Consume(TokenType::LeftBrace, "期望 '{'");
                    AST::PassDef pass;
                    ParsePass(pass);
                    doc.Passes.push_back(std::move(pass));
                    Consume(TokenType::RightBrace, "期望 '}'");
                }
                else if (Check(TokenType::RenderCommandKw))
                {
                    // SubShader
                    Advance();
                    Consume(TokenType::LeftBrace, "期望 '{'");
                    doc.RenderState = ParseRenderCommand();
                    Consume(TokenType::RightBrace, "期望 '}'");
                }
                else
                {
                    Error("SubShader 内期望 'Pass' 或 'RenderCommand'");
                    Advance();
                }
            }
            Consume(TokenType::RightBrace, "期望 '}'");
        }
        else
        {
            Error("期望 'Properties', 'RenderCommand' 或 'SubShader'");
            Advance();
        }
    }

    Consume(TokenType::RightBrace, "期望 '}'");

    ParseMaterialLayout(doc);
    return doc;
}


Token& Parser::Current()           { return m_Stream.Current(); }
Token Parser::PeekToken(int off)   { return m_Stream.PeekToken(off); }
Token Parser::Advance()            { return m_Stream.Advance(); }
bool Parser::Check(TokenType t)    { return m_Stream.Check(t); }
bool Parser::Match(TokenType t)    { return m_Stream.Match(t); }
Token Parser::Consume(TokenType type, const std::string& errMsg)
{
    if (Check(type)) return Advance();
    Error(errMsg);
    return Current();
}

Token Parser::ConsumeNumber(const std::string& errMsg)
{
    if (Check(TokenType::FloatLiteral) || Check(TokenType::IntegerLiteral))
        return Advance();
    Error(errMsg);
    return Current();
}

bool Parser::IsAtEnd()             { return m_Stream.IsAtEnd(); }

SourceLocation Parser::CurrentLoc()
{
    return m_Stream.GetSM().GetLocation(Current().Offset);
}

void Parser::Error(const std::string& msg)
{
    if (m_Diag) m_Diag->Error(msg, CurrentLoc());
    PR_CORE_ERROR("[{0}:{1}] {2}", CurrentLoc().Column, CurrentLoc().Line, msg);
    std::string_view code = m_Stream.GetSM().GetView(Current().Offset, Current().Length);
    TokenType t = Current().Type;
    PR_CORE_ERROR("    --> 代码片段: '{0}'", code);
}

bool Parser::IsTypeToken(TokenType t)
{
    return t == TokenType::Identifier
        || t == TokenType::Vec2Kw || t == TokenType::Vec3Kw || t == TokenType::Vec4Kw
        || t == TokenType::Mat3Kw || t == TokenType::Mat4Kw
        || t == TokenType::FloatGLSLKw || t == TokenType::IntGLSLKw || t == TokenType::BoolGLSLKw
        || t == TokenType::Sampler2DKw || t == TokenType::SamplerCubeKw || t == TokenType::Sampler2DMSKw
        || t == TokenType::VoidKw;
}

Token Parser::ConsumeType(const std::string& errMsg)
{
    if (IsTypeToken(Current().Type)) return Advance();
    Error(errMsg);
    return Current();
}

// Token 文本取值
std::string_view Parser::TokenText(const Token& t) const
{
    return m_Stream.GetSM().GetView(t.Offset, t.Length);
}

std::string Parser::TokenStr(const Token& t) const
{
    return std::string(TokenText(t));
}

float Parser::TokenFloat(const Token& t) const
{
    auto sv = TokenText(t);
    char buf[64];
    uint32_t n = sv.size() < 63 ? (uint32_t)sv.size() : 63;
    std::memcpy(buf, sv.data(), n);
    buf[n] = '\0';
    return (float)std::atof(buf);
}

int Parser::TokenInt(const Token& t) const
{
    auto sv = TokenText(t);
    char buf[64];
    uint32_t n = sv.size() < 63 ? (uint32_t)sv.size() : 63;
    std::memcpy(buf, sv.data(), n);
    buf[n] = '\0';
    return std::atoi(buf);
}

void Parser::ParseProperties(std::vector<AST::ShaderUniform>& uniforms)
{
    while (!IsAtEnd() && !Check(TokenType::RightBrace))
    {
        if (Check(TokenType::Identifier))
        {
            uniforms.push_back(ParseProperty());
        }
        else
        {
            Error("期望属性名");
            Advance();
        }
    }
}

AST::ShaderUniform Parser::ParseProperty()
{
    AST::ShaderUniform uniform;
    uniform.Name = TokenStr(Consume(TokenType::Identifier, "期望属性名"));

    Consume(TokenType::LeftParen, "期望 '('");
    uniform.DisplayName = TokenStr(Consume(TokenType::StringLiteral, "期望显示名称"));
    Consume(TokenType::Comma, "期望 ','");

    TypeDesc desc = ParsePropertyType();
    uniform.Type = desc.Type;
    uniform.EnumOptions = std::move(desc.EnumOptions);
    uniform.RangeMin = desc.RangeMin;
    uniform.RangeMax = desc.RangeMax;

    Consume(TokenType::RightParen, "期望 ')'");

    Consume(TokenType::Equals, "期望 '='");
    uniform.DefaultValue = ParseDefaultValue(desc);

    return uniform;
}

TypeDesc Parser::ParsePropertyType()
{
    static const std::unordered_map<TokenType, PropertyType> kTypeMap = {
        {TokenType::BoolKw,        PropertyType::Bool},
        {TokenType::FloatKw,       PropertyType::Float},
        {TokenType::IntKw,         PropertyType::Int},
        {TokenType::ColorKw,       PropertyType::Color},
        {TokenType::Color3Kw,      PropertyType::Color3},
        {TokenType::Vector2Kw,     PropertyType::Vector2},
        {TokenType::Vector3Kw,     PropertyType::Vector3},
        {TokenType::Vector4Kw,     PropertyType::Vector4},
        {TokenType::Matrix3Kw,     PropertyType::Matrix3},
        {TokenType::Matrix4Kw,     PropertyType::Matrix4},
        {TokenType::Texture2DKw,   PropertyType::Texture2D},
        {TokenType::Texture2DMSKw, PropertyType::Texture2DMS},
        {TokenType::TextureCubeKw,  PropertyType::TextureCube},
    };

    auto it = kTypeMap.find(Current().Type);
    if (it != kTypeMap.end())
    {
        Advance();
        return {it->second};
    }

    TypeDesc desc;

    if (Check(TokenType::RangeKw))
    {
        Advance();
        Consume(TokenType::LeftParen, "Range 期望 '('");
        desc.RangeMin = TokenFloat(ConsumeNumber("期望数值"));
        Consume(TokenType::Comma, "期望 ','");
        desc.RangeMax = TokenFloat(ConsumeNumber("期望数值"));
        Consume(TokenType::RightParen, "期望 ')'");
        desc.Type = PropertyType::Range;
        return desc;
    }

    if (Check(TokenType::EnumKw))
    {
        Advance();
        Consume(TokenType::LeftParen, "Enum 期望 '('");
        desc.EnumOptions.push_back(TokenStr(Consume(TokenType::Identifier, "期望选项")));
        while (Check(TokenType::Comma))
        {
            Advance();
            desc.EnumOptions.push_back(TokenStr(Consume(TokenType::Identifier, "期望选项")));
        }
        Consume(TokenType::RightParen, "期望 ')'");
        desc.Type = PropertyType::Enum;
        return desc;
    }

    Error("未知属性类型");
    return desc;
}

std::vector<Scalar> Parser::ParseDefaultValue(const TypeDesc& type)
{
    // 元组: (x, y, z, w) — 用于 Color/Vector/Color3
    auto ParseTuple = [this]() -> std::vector<Scalar> {
        std::vector<Scalar> scalars;
        Consume(TokenType::LeftParen, "期望 '('");
        scalars.push_back(Scalar::FromFloat(TokenFloat(ConsumeNumber("期望数值"))));
        while (Check(TokenType::Comma))
        {
            Advance();
            scalars.push_back(Scalar::FromFloat(TokenFloat(ConsumeNumber("期望数值"))));
        }
        Consume(TokenType::RightParen, "期望 ')'");
        return scalars;
    };

    switch (type.Type)
    {
    case PropertyType::Bool:
        return {Scalar::FromBool(Consume(TokenType::TrueKw, "期望 true/false").Is(TokenType::TrueKw))};
    case PropertyType::Float:
    case PropertyType::Range:
        return {Scalar::FromFloat(TokenFloat(Advance()))};
    case PropertyType::Int:
    case PropertyType::Enum:
        return {Scalar::FromInt(TokenInt(Advance()))};
    case PropertyType::Color:
    case PropertyType::Color3:
    case PropertyType::Vector2:
    case PropertyType::Vector3:
    case PropertyType::Vector4:
        return ParseTuple();
    case PropertyType::Matrix3:
    case PropertyType::Matrix4:
        return {Scalar::FromFloat(TokenFloat(Advance()))};
    case PropertyType::Texture2D:
    case PropertyType::Texture2DMS:
    case PropertyType::TextureCube:
        Consume(TokenType::LeftBrace, "期望 '{}'");
        Consume(TokenType::RightBrace, "期望 '}'");
        return {};
    default:
        return {};
    }
}

void Parser::ParseMaterialLayout(AST::ShaderDocument& doc)
{
    doc.MaterialLayout = PropertyLayout{};
    for (auto& uniform : doc.Uniforms)
    {
        if (PropertyTypeUtil::IsTextureType(uniform.Type))
            continue;
        doc.MaterialLayout.Add(uniform.Name, uniform.Type);
    }
    for (auto& uniform : doc.Uniforms)
    {
        if (PropertyTypeUtil::IsTextureType(uniform.Type))
            continue;
        const auto* member = doc.MaterialLayout.Find(uniform.Name);
        if (member)
        {
            uniform.BufferOffset = member->Offset;
            uniform.BufferSize = member->Size;
        }
    }
}

PipelineState Parser::ParseRenderCommand()
{
    PipelineState state = PipelineState::Default();

    while (!IsAtEnd() && !Check(TokenType::RightBrace))
    {
        if (Check(TokenType::BlendKw))
        {
            Advance();
            if (Check(TokenType::OffKw))
                { Advance(); state.BlendEnabled = false; }
            else
            {
                state.BlendEnabled = true;
                // SrcFactor DstFactor [SrcAlpha DstAlpha]
                if (Check(TokenType::SrcAlphaKw))     { Advance(); state.SrcFactor = BlendFactor::SrcAlpha; }
                else if (Check(TokenType::OneKw))      { Advance(); state.SrcFactor = BlendFactor::One; }
                else if (Check(TokenType::ZeroKw))     { Advance(); state.SrcFactor = BlendFactor::Zero; }
                else if (Check(TokenType::DstAlphaKw)) { Advance(); state.SrcFactor = BlendFactor::DstAlpha; }
                else if (Check(TokenType::OneMinusSrcAlphaKw)) { Advance(); state.SrcFactor = BlendFactor::OneMinusSrcAlpha; }
                else if (Check(TokenType::OneMinusDstAlphaKw)) { Advance(); state.SrcFactor = BlendFactor::OneMinusDstAlpha; }

                if (Check(TokenType::SrcAlphaKw))     { Advance(); state.DstFactor = BlendFactor::SrcAlpha; }
                else if (Check(TokenType::OneKw))      { Advance(); state.DstFactor = BlendFactor::One; }
                else if (Check(TokenType::ZeroKw))     { Advance(); state.DstFactor = BlendFactor::Zero; }
                else if (Check(TokenType::DstAlphaKw)) { Advance(); state.DstFactor = BlendFactor::DstAlpha; }
                else if (Check(TokenType::OneMinusSrcAlphaKw)) { Advance(); state.DstFactor = BlendFactor::OneMinusSrcAlpha; }
                else if (Check(TokenType::OneMinusDstAlphaKw)) { Advance(); state.DstFactor = BlendFactor::OneMinusDstAlpha; }

                // 可选的独立 alpha blend 参数
                if (!Check(TokenType::RightBrace) && !Check(TokenType::CullKw)
                    && !Check(TokenType::ZTestKw) && !Check(TokenType::ZWriteKw) && !Check(TokenType::BlendKw)
                    && !Check(TokenType::ColorMaskKw) && !Check(TokenType::OffsetKw))
                {
                    if (Check(TokenType::SrcAlphaKw))     { Advance(); state.SrcAlpha = BlendFactor::SrcAlpha; }
                    else if (Check(TokenType::OneKw))      { Advance(); state.SrcAlpha = BlendFactor::One; }
                    else if (Check(TokenType::ZeroKw))     { Advance(); state.SrcAlpha = BlendFactor::Zero; }
                    else if (Check(TokenType::DstAlphaKw)) { Advance(); state.SrcAlpha = BlendFactor::DstAlpha; }
                    else if (Check(TokenType::OneMinusSrcAlphaKw)) { Advance(); state.SrcAlpha = BlendFactor::OneMinusSrcAlpha; }
                    else if (Check(TokenType::OneMinusDstAlphaKw)) { Advance(); state.SrcAlpha = BlendFactor::OneMinusDstAlpha; }

                    if (Check(TokenType::SrcAlphaKw))     { Advance(); state.DstAlpha = BlendFactor::SrcAlpha; }
                    else if (Check(TokenType::OneKw))      { Advance(); state.DstAlpha = BlendFactor::One; }
                    else if (Check(TokenType::ZeroKw))     { Advance(); state.DstAlpha = BlendFactor::Zero; }
                    else if (Check(TokenType::DstAlphaKw)) { Advance(); state.DstAlpha = BlendFactor::DstAlpha; }
                    else if (Check(TokenType::OneMinusSrcAlphaKw)) { Advance(); state.DstAlpha = BlendFactor::OneMinusSrcAlpha; }
                    else if (Check(TokenType::OneMinusDstAlphaKw)) { Advance(); state.DstAlpha = BlendFactor::OneMinusDstAlpha; }
                }
            }
        }
        else if (Check(TokenType::CullKw))
        {
            Advance();
            if (Check(TokenType::OffKw))    { Advance(); state.Cull = CullMode::Off; }
            else if (Check(TokenType::BackKw))  { Advance(); state.Cull = CullMode::Back; }
            else if (Check(TokenType::FrontKw)) { Advance(); state.Cull = CullMode::Front; }
        }
        else if (Check(TokenType::ZTestKw))
        {
            Advance();
            if (Check(TokenType::OffKw))       { Advance(); state.DepthTest = false; }
            else if (Check(TokenType::NeverKw))     { Advance(); state.DepthCompare = DepthFunc::Never; }
            else if (Check(TokenType::LessKw))      { Advance(); state.DepthCompare = DepthFunc::Less; }
            else if (Check(TokenType::EqualKw))     { Advance(); state.DepthCompare = DepthFunc::Equal; }
            else if (Check(TokenType::LEqualKw))    { Advance(); state.DepthCompare = DepthFunc::LEqual; }
            else if (Check(TokenType::GreaterKw))   { Advance(); state.DepthCompare = DepthFunc::Greater; }
            else if (Check(TokenType::NotEqualKw))  { Advance(); state.DepthCompare = DepthFunc::NotEqual; }
            else if (Check(TokenType::GEqualKw))   { Advance(); state.DepthCompare = DepthFunc::GEqual; }
            else if (Check(TokenType::AlwaysKw))    { Advance(); state.DepthCompare = DepthFunc::Always; }
        }
        else if (Check(TokenType::ZWriteKw))
        {
            Advance();
            if (Check(TokenType::OnKw)) { Advance(); state.DepthWrite = true; }
            else { Advance(); state.DepthWrite = false; }
        }
        else if (Check(TokenType::ColorMaskKw))
        {
            Advance();
            Token t = Advance();
            std::string v = TokenStr(t);
            if (v == "RGBA")           state.WriteMask = ColorMask::RGBA;
            else if (v == "RGB")       state.WriteMask = ColorMask::RGB;
            else if (v == "R")         state.WriteMask = ColorMask::R;
            else if (v == "G")         state.WriteMask = ColorMask::G;
            else if (v == "B")         state.WriteMask = ColorMask::B;
            else if (v == "A")         state.WriteMask = ColorMask::A;
            else if (v == "0")         state.WriteMask = ColorMask::None;
            else                       Error("非法的 ColorMask 值: " + v);
        }
        else if (Check(TokenType::OffsetKw))
        {
            Advance();
            state.DepthBiasFactor = TokenFloat(ConsumeNumber("期望 factor"));
            Match(TokenType::Comma);
            state.DepthBiasUnits = TokenFloat(ConsumeNumber("期望 units"));
        }
        else
            Advance();
    }

    return state;
}

void Parser::ParsePass(AST::PassDef& pass)
{
    while (!IsAtEnd() && !Check(TokenType::RightBrace))
    {
        if (Check(TokenType::NameKw))
        {
            Advance();
            pass.Name = TokenStr(Consume(TokenType::StringLiteral, "期望 Pass 名称"));
        }
        else if (Check(TokenType::TagsKw))
        {
            Advance();
            Consume(TokenType::LeftBrace, "期望 '{'");
            ParseTags(pass.Tags);
            Consume(TokenType::RightBrace, "期望 '}'");
        }
        else if (Check(TokenType::RenderCommandKw))
        {
            Advance();
            Consume(TokenType::LeftBrace, "期望 '{'");
            pass.RenderState = ParseRenderCommand();
            Consume(TokenType::RightBrace, "期望 '}'");
        }
        else if (Check(TokenType::GLSLKw))
        {
            Advance();
            ParseGLSLBlock(pass.Glsl);
        }
        else
        {
            Error("Pass 内期望 'Name', 'Tags', 'RenderCommand' 或 'GLSL'");
            Advance();
        }
    }
}

void Parser::ParseTags(std::unordered_map<std::string, std::string>& tags)
{
    while (!IsAtEnd() && !Check(TokenType::RightBrace))
    {
        std::string key = TokenStr(Consume(TokenType::StringLiteral, "期望 Tag 键"));
        Consume(TokenType::Equals, "期望 '='");
        std::string value = TokenStr(Consume(TokenType::StringLiteral, "期望 Tag 值"));
        tags[key] = value;
    }
}

void Parser::ParseGLSLBlock(AST::GLSLCode& glsl)
{
    Consume(TokenType::LeftBrace, "期望 '{'");
    glsl.Loc = CurrentLoc();
    int depth = 1;
    uint32_t nextID = 0;
    uint32_t sharedStart = Current().Offset;

    while (!IsAtEnd() && depth > 0)
    {
        if (Check(TokenType::Hash))
        {
            FlushSharedChunk(glsl.SharedSource, sharedStart);
            ParseGLSLDirective(glsl, nextID++);
            sharedStart = Current().Offset;
        }
        else if (Check(TokenType::AttributeKw))
        {
            FlushSharedChunk(glsl.SharedSource, sharedStart);
            ParseGLSLAttribute(glsl, nextID++);
            sharedStart = Current().Offset;
        }
        else if (Check(TokenType::VaryingKw))
        {
            FlushSharedChunk(glsl.SharedSource, sharedStart);
            ParseGLSLVarying(glsl, nextID++);
            sharedStart = Current().Offset;

        }
        else if (Check(TokenType::VoidKw))
        {
            FlushSharedChunk(glsl.SharedSource, sharedStart);
            ParserGLSLVoid(glsl);
            sharedStart = Current().Offset;
        }
        else if (Check(TokenType::LeftBrace))
        {
            Advance();
            depth++;
        }
        else if (Check(TokenType::RightBrace))
        {
            depth--;
            if (depth == 0)
            {
                FlushSharedChunk(glsl.SharedSource, sharedStart);
                break;
            }
            Advance();
        }
        else
        {
            Token t = Advance();
        }
    }
}

void Parser::ParserGLSLVoid(AST::GLSLCode& glsl)
{
    Token next = PeekToken(1);
    if (next.IsNot(TokenType::VertKw) && next.IsNot(TokenType::FragKw))
        return;

    bool isVert = next.Is(TokenType::VertKw);
    (isVert ? glsl.Vertex : glsl.Fragment).Loc = CurrentLoc();

    Advance(); // void
    Advance(); // vert / frag
    Consume(TokenType::LeftParen, "期望 '('");
    Consume(TokenType::RightParen, "期望 ')'");
    Token openBrace = Consume(TokenType::LeftBrace, "期望 '{'");

    int funcDepth = 1;
    while (!IsAtEnd() && funcDepth > 0)
    {
        Token ft = Advance();
        if (ft.Is(TokenType::LeftBrace))
            funcDepth++;
        else if (ft.Is(TokenType::RightBrace))
        {
            funcDepth--;
            if (funcDepth == 0)
            {
                std::string& target = isVert ? glsl.Vertex.Source : glsl.Fragment.Source;
                uint32_t len = ft.Offset - openBrace.Offset + ft.Length;
                target = std::string(m_Stream.GetSM().GetView(openBrace.Offset, len));
                break;
            }
        }
    }
}

void Parser::ParseGLSLAttribute(AST::GLSLCode& glsl, uint32_t id)
{
    glsl.SharedSource += "[Prism::Insert:" + std::to_string(id) + "]";
    Advance();
    AST::VertexAttribute attr;
    attr.InsertID = id;
    attr.Loc = CurrentLoc();
    attr.Type = TokenStr(ConsumeType("期望 attribute 类型"));
    attr.Name = TokenStr(Consume(TokenType::Identifier, "期望 attribute 名称"));
    Consume(TokenType::Colon, "期望 ':'");
    attr.Semantic = Prism::PSL::ParseVertexSemantic(TokenStr(Consume(TokenType::Identifier, "期望语义名称")));
    Consume(TokenType::Semicolon, "期望 ';'");
    glsl.Attributes.push_back(attr);
}

void Parser::ParseGLSLVarying(AST::GLSLCode& glsl, uint32_t id)
{
    glsl.SharedSource += "[Prism::Insert:" + std::to_string(id) + "]";
    Advance();

    AST::VaryingBlock block;
    block.InsertID = id;
    block.Loc = CurrentLoc();
    std::string first = TokenStr(ConsumeType("期望类型或结构体名"));

    if (Check(TokenType::LeftBrace))
    {
        block.IsStruct = true;
        block.StructName = first;
        Consume(TokenType::LeftBrace, "期望 '{'");

        while (!Check(TokenType::RightBrace) && !IsAtEnd())
        {
            if (!IsTypeToken(Current().Type))
            {
                Error("VARYING 成员格式错误，期望类型名");
                SkipTo(TokenType::RightBrace);
                if (Check(TokenType::RightBrace)) break;
                Advance();
                continue;
            }
            std::string memberType = TokenStr(ConsumeType("期望成员类型"));
            std::string memberName = TokenStr(Consume(TokenType::Identifier, "期望成员名称"));
            Consume(TokenType::Semicolon, "期望 ';'");
            block.Members.push_back({ memberType, memberName });
        }
        Consume(TokenType::RightBrace, "期望 '}'");

        block.InstanceName = TokenStr(Consume(TokenType::Identifier, "期望实例名称"));
        Consume(TokenType::Semicolon, "期望 ';'");
    }
    else
    {
        block.IsStruct = false;
        block.Type = first;
        block.InstanceName = TokenStr(Consume(TokenType::Identifier, "期望变量名"));
        Consume(TokenType::Semicolon, "期望 ';'");
    }

    glsl.Varyings.push_back(block);
}

void Parser::ParseGLSLDirective(AST::GLSLCode& glsl, uint32_t id)
{
    glsl.SharedSource += "[Prism::Insert:" + std::to_string(id) + "]";
    Advance();

    if (Check(TokenType::IncludeKw))
    {
        Advance();
        glsl.Includes.push_back({TokenStr(Consume(TokenType::StringLiteral, "期望 include 路径")), id, CurrentLoc()});
    }
    else if (Check(TokenType::PragmaKw))
    {
        Advance();

        AST::PragmaDef pragma;
        pragma.InsertID = id;
        pragma.Loc = CurrentLoc();
        if (Check(TokenType::ShaderFeatureKw))
            { Advance(); pragma.IsShaderFeature = true; }
        else if (Check(TokenType::MultiCompileKw))
            { Advance(); pragma.IsMultiCompile = true; }
        else
            return;

        while (Check(TokenType::Identifier))
            pragma.Keywords.push_back(TokenStr(Advance()));

        if (!pragma.Keywords.empty())
            glsl.Pragmas.push_back(pragma);
    }
}
void Parser::FlushSharedChunk(std::string& out, uint32_t& start)
{
    uint32_t current = m_Stream.Current().Offset;
    if (current > start)
    {
        auto view = m_Stream.GetSM().GetView(start, current - start);
        out += std::string(m_Stream.GetSM().GetView(start, current - start));
        start = current;
    }
}

void Parser::AppendTokenText(std::string& out, const Token& t)
{
    auto text = TokenText(t);
    out.append(text.data(), text.size());
    out += (t.Is(TokenType::Semicolon) || t.Is(TokenType::LeftBrace)) ? '\n' : ' ';
}

void Parser::SkipTo(TokenType type)
{
    while (!IsAtEnd() && !Check(type))
        Advance();
}

} // namespace Prism::PSL
