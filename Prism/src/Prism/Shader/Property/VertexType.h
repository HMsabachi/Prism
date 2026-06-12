#pragma once
#include <string>
namespace Prism::PSL
{
    enum class VertexSemantic
    {
        Position,
        Normal,
        Tangent,
        Binormal,
        TexCoord0,
        TexCoord1,
        BoneIndices,
        BoneWeights,
        InstanceID,
        Color,
        Index0,
        Index1,
        Other0,
        Other1,
        Other2,
        Unknown
    };
    inline VertexSemantic ParseVertexSemantic(const std::string_view semanticStr)
    {
        if (semanticStr == "POSITION") return VertexSemantic::Position;
        else if (semanticStr == "NORMAL") return VertexSemantic::Normal;
        else if (semanticStr == "TANGENT") return VertexSemantic::Tangent;
        else if (semanticStr == "BINORMAL") return VertexSemantic::Binormal;
        else if (semanticStr == "TEXCOORD0") return VertexSemantic::TexCoord0;
        else if (semanticStr == "TEXCOORD1") return VertexSemantic::TexCoord1;
        else if (semanticStr == "BONEINDICES") return VertexSemantic::BoneIndices;
        else if (semanticStr == "BONEWEIGHTS") return VertexSemantic::BoneWeights;
        else if (semanticStr == "INSTANCEID") return VertexSemantic::InstanceID;
        else if (semanticStr == "COLOR") return VertexSemantic::Color;
        else if (semanticStr == "INDEX0") return VertexSemantic::Index0;
        else if (semanticStr == "INDEX1") return VertexSemantic::Index1;
        else if (semanticStr == "OTHER0") return VertexSemantic::Other0;
        else if (semanticStr == "OTHER1") return VertexSemantic::Other1;
        else if (semanticStr == "OTHER2") return VertexSemantic::Other2;
        else return VertexSemantic::Unknown;
    }
    inline uint32_t SemanticToLocation(VertexSemantic semantic)
    {
        return static_cast<uint32_t>(semantic);
    }
} // namespace Prism::PSL
