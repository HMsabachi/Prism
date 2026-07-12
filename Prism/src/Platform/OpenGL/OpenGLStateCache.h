#pragma once

namespace PrismShaderCompiler { struct PipelineState; }

namespace Prism
{

    class OpenGLStateCache
    {
    public:
        static void Init();

        static void Apply(const PrismShaderCompiler::PipelineState& newCommand);

        static void Reset();

    private:
        static void ApplyBlendIfChanged(const PrismShaderCompiler::PipelineState& cmd);
        static void ApplyCullIfChanged(const PrismShaderCompiler::PipelineState& cmd);
        static void ApplyDepthIfChanged(const PrismShaderCompiler::PipelineState& cmd);
        static void ApplyColorMaskIfChanged(const PrismShaderCompiler::PipelineState& cmd);
        static void ApplyDepthBiasIfChanged(const PrismShaderCompiler::PipelineState& cmd);
    };
}
