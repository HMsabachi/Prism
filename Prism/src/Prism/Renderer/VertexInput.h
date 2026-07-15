#pragma once

#include "Prism/Renderer/Buffer/VertexBuffer.h"

#include <string>

namespace Prism {

    struct VertexInputSpecification
    {
        VertexBufferLayout Layout;
        std::string DebugName;
    };

    class VertexInput : public RefCounted
    {
    public:
        virtual ~VertexInput() = default;

        virtual VertexInputSpecification& GetSpecification() = 0;
        virtual const VertexInputSpecification& GetSpecification() const = 0;

        virtual void Invalidate() = 0;

        // TEMP: remove this when render command buffers are a thing
        virtual void Bind() const = 0;

        static Ref<VertexInput> Create(const VertexInputSpecification& spec);
    };

}
