#pragma once

#include "Prism/Core/Core.h"
#include "Prism/Renderer/RendererAPI.h"

namespace PrismShaderCompiler { struct PassReflection; }

namespace Prism
{
    class PRISM_API Shader : public RefCounted
    {
    public:
        virtual ~Shader() = default;

        static Ref<Shader> Create(const void* vertexSource, const void* fragmentSource,
            const PrismShaderCompiler::PassReflection& reflection);
        static Ref<Shader> Create(const void* computeSource);
    };

}
