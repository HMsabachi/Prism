#pragma once

#include <span>
#include <unordered_map>

#include "Prism/Core/Ref.h"
#include "Prism/Renderer/Buffer/VertexBuffer.h"

namespace Prism {

    class OpenGLVertexArray : public RefCounted
    {
    public:
        OpenGLVertexArray(std::span<const Ref<VertexBuffer>> vertexBuffers);
        virtual ~OpenGLVertexArray();

        void RT_Bind() const;
    private:
        uint32_t m_RendererID = 0;
    };

    class OpenGLVertexArrayCache
    {
    public:
        Ref<OpenGLVertexArray>& RT_Get(std::span<const Ref<VertexBuffer>> vertexBuffers);
        Ref<OpenGLVertexArray>& RT_Get(const Ref<VertexBuffer>& vertexBuffer);
    private:
        std::unordered_map<uint64_t, Ref<OpenGLVertexArray>> m_Cache;
    };
}
