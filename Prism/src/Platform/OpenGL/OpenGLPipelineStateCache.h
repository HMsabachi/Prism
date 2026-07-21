#pragma once

#include <functional>
#include <unordered_map>

#include "Prism/Core/Ref.h"
#include "Prism/Renderer/RendererTypes.h"
#include <PrismShaderCore/Pipeline/PipelineState.h>

namespace Prism
{
    struct PSOKey
    {
        RendererID Program = 0;
        PrismShaderCompiler::PipelineState State;

        bool operator==(const PSOKey& other) const
        {
            return Program == other.Program
                && State.BlendEnabled == other.State.BlendEnabled
                && State.SrcFactor == other.State.SrcFactor
                && State.DstFactor == other.State.DstFactor
                && State.SrcAlpha == other.State.SrcAlpha
                && State.DstAlpha == other.State.DstAlpha
                && State.DepthTest == other.State.DepthTest
                && State.DepthWrite == other.State.DepthWrite
                && State.DepthCompare == other.State.DepthCompare
                && State.WriteMask == other.State.WriteMask
                && State.DepthBiasFactor == other.State.DepthBiasFactor
                && State.DepthBiasUnits == other.State.DepthBiasUnits
                && State.Cull == other.State.Cull
                && State.StencilTest == other.State.StencilTest
                && State.StencilCompare == other.State.StencilCompare
                && State.StencilRef == other.State.StencilRef
                && State.StencilReadMask == other.State.StencilReadMask
                && State.StencilWriteMask == other.State.StencilWriteMask
                && State.StencilFailOp == other.State.StencilFailOp
                && State.StencilDepthFailOp == other.State.StencilDepthFailOp
                && State.StencilPassOp == other.State.StencilPassOp
                && State.FillMode == other.State.FillMode
                && State.LineWidth == other.State.LineWidth;
        }
    };

    class OpenGLPipelineState : public RefCounted
    {
    public:
        OpenGLPipelineState(RendererID program, const PrismShaderCompiler::PipelineState& state);

        void Bind() const;

    private:
        RendererID m_Program;
        PrismShaderCompiler::PipelineState m_State;
    };

    class OpenGLPipelineStateCache
    {
    public:
        static Ref<OpenGLPipelineState> Get(RendererID program, const PrismShaderCompiler::PipelineState& state);
        static void Clear();

    private:
        static std::unordered_map<PSOKey, Ref<OpenGLPipelineState>> s_Cache;
    };
}

namespace std
{
    template<>
    struct hash<Prism::PSOKey>
    {
        size_t operator()(const Prism::PSOKey& key) const noexcept
        {
            const auto& s = key.State;
            size_t seed = std::hash<Prism::RendererID>{}(key.Program);
            auto combine = [&seed](auto v)
            {
                seed ^= std::hash<decltype(v)>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            };
            combine(s.BlendEnabled);
            combine(s.SrcFactor);
            combine(s.DstFactor);
            combine(s.SrcAlpha);
            combine(s.DstAlpha);
            combine(s.DepthTest);
            combine(s.DepthWrite);
            combine(s.DepthCompare);
            combine(s.WriteMask);
            combine(s.DepthBiasFactor);
            combine(s.DepthBiasUnits);
            combine(s.Cull);
            combine(s.StencilTest);
            combine(s.StencilCompare);
            combine(s.StencilRef);
            combine(s.StencilReadMask);
            combine(s.StencilWriteMask);
            combine(s.StencilFailOp);
            combine(s.StencilDepthFailOp);
            combine(s.StencilPassOp);
            combine(s.FillMode);
            combine(s.LineWidth);
            return seed;
        }
    };
}
