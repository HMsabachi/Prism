#pragma once

#include <cstdint>
#include <string>

namespace Prism::PSL
{

enum class ColorMask : uint8_t
{
    RGBA,
    RGB,
    R,
    G,
    B,
    A,
    None,  // 对应 "0"
};

enum class CullMode : uint8_t
{
    Off,
    Back,
    Front,
};

enum class DepthFunc : uint8_t
{
    Never,
    Less,
    Equal,
    LEqual,
    Greater,
    NotEqual,
    GEqual,
    Always,
};

enum class BlendFactor : uint8_t
{
    Zero,
    One,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
};

struct PipelineState
{
    bool BlendEnabled = true;
    BlendFactor SrcFactor = BlendFactor::SrcAlpha;
    BlendFactor DstFactor = BlendFactor::OneMinusSrcAlpha;
    BlendFactor SrcAlpha = BlendFactor::One;
    BlendFactor DstAlpha = BlendFactor::OneMinusSrcAlpha;

    bool DepthTest = true;
    bool DepthWrite = true;
    DepthFunc DepthCompare = DepthFunc::LEqual;

    ColorMask WriteMask = ColorMask::RGBA;

    float DepthBiasFactor = 0;
    float DepthBiasUnits = 0;

    CullMode Cull = CullMode::Back;

    void Merge(const PipelineState& passState);
    static PipelineState Default();
};

} // namespace Prism::PSL
