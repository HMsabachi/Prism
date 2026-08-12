#pragma once

#include "Prism/Renderer/VertexInput.h"

namespace Prism {

    class OpenGLVertexInput : public VertexInput
    {
    public:
        OpenGLVertexInput(const VertexInputSpecification& spec);
        virtual ~OpenGLVertexInput();

        virtual VertexInputSpecification& GetSpecification() { return m_Specification; }
        virtual const VertexInputSpecification& GetSpecification() const { return m_Specification; }

        virtual void Invalidate() override;

        void RT_Bind() const;
    private:
        VertexInputSpecification m_Specification;
        uint32_t m_VertexArrayRendererID = 0;
    };

}
