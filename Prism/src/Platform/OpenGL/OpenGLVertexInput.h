#pragma once

#include "Prism/Core/Ref.h"
#include "Prism/Renderer/Buffer/VertexBuffer.h"

namespace Prism {

    class OpenGLVertexInput : public RefCounted
    {
    public:
        OpenGLVertexInput(const VertexBufferLayout& layout);
        virtual ~OpenGLVertexInput();

        void RT_Bind() const;
        void RT_Invalidate();
    private:
        VertexBufferLayout m_Layout;
        uint32_t m_VertexArrayRendererID = 0;
    };

    class OpenGLVertexArrayCache
    {
    public:
        Ref<OpenGLVertexInput>& Get(const VertexBufferLayout& Layout);
    private:
        std::unordered_map<uint64_t, Ref<OpenGLVertexInput>> m_Cache;
    };
}
