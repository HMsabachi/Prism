#pragma once

#include "Prism/Renderer/Buffer/VertexBuffer.h"

namespace Prism {

    class PrismShader;

    struct PipelineSpecification
    {
        Ref<PrismShader> Shader;
        VertexBufferLayout Layout;
    };

    class Pipeline : public RefCounted
    {
    public:
        virtual ~Pipeline() = default;

        virtual PipelineSpecification& GetSpecification() = 0;
        virtual const PipelineSpecification& GetSpecification() const = 0;

        virtual void Invalidate() = 0;

        // TEMP: remove this when render command buffers are a thing
        virtual void Bind() const = 0;

        static Ref<Pipeline> Create(const PipelineSpecification& spec);
    };

}
