#pragma once

#include <functional>
#include <unordered_map>

#include "Prism/Core/Ref.h"
#include "Prism/Renderer/RendererTypes.h"
#include <PrismShaderCore/Pipeline/PipelineState.h>

namespace Prism
{

    class OpenGLPipelineState : public RefCounted
    {
    private:
        inline static uint64_t s_CurrentHash = 0;
    public:
        static void RT_SetupPipelineState(const PrismShaderCompiler::PipelineState& state);
    };

}

