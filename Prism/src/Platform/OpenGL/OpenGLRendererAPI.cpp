#include "prpch.h"
#include "Prism/Renderer/RendererAPI.h"

#include <Glad/glad.h>

namespace Prism
{
    // 低层静态方法实现（Phase 6 收敛进 OpenGLRenderer Utils static 对齐 Hazel 后删除此文件）
    static GLbitfield PrismToOpenGLMemoryBarrier(RendererAPI::BarrierFlags flags)
    {
        GLbitfield result = 0;
        if (flags & RendererAPI::Barrier::ShaderStorage) result |= GL_SHADER_STORAGE_BARRIER_BIT;
        if (flags & RendererAPI::Barrier::VertexAttribArray) result |= GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT;
        if (flags & RendererAPI::Barrier::ElementArray) result |= GL_ELEMENT_ARRAY_BARRIER_BIT;
        if (flags & RendererAPI::Barrier::ImageAccess) result |= GL_FRAMEBUFFER_BARRIER_BIT;
        if (flags & RendererAPI::Barrier::TextureFetch) result |= GL_TEXTURE_FETCH_BARRIER_BIT;
        if (flags & RendererAPI::Barrier::TextureUpdate) result |= GL_TEXTURE_UPDATE_BARRIER_BIT;
        if (flags & RendererAPI::Barrier::Framebuffer) result |= GL_FRAMEBUFFER_BARRIER_BIT;
        if (flags & RendererAPI::Barrier::Command) result |= GL_COMMAND_BARRIER_BIT;
        if (flags & RendererAPI::Barrier::All) result |= GL_ALL_BARRIER_BITS;
        return result;
    }

    static GLenum PrismToOpenGLPrimitiveType(PrimitiveType type)
    {
        switch (type)
        {
        case PrimitiveType::None:           PR_CORE_ASSERT(false, "Invalid PrimitiveType");
        case PrimitiveType::Triangles:      return GL_TRIANGLES;
        case PrimitiveType::Lines:          return GL_LINES;
        }
        PR_CORE_ASSERT(false, "Invalid PrimitiveType");
        return 0;
    }

    void RendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        glViewport(x, y, width, height);
    }

    void RendererAPI::Clear(float r, float g, float b, float a)
    {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void RendererAPI::SetClearColor(float r, float g, float b, float a)
    {
        glClearColor(r, g, b, a);
    }

    void RendererAPI::DrawIndexed(uint32_t count, PrimitiveType type, bool depthTest)
    {
        if (!depthTest)
            glDisable(GL_DEPTH_TEST);
        glDrawElements(PrismToOpenGLPrimitiveType(type), count, GL_UNSIGNED_INT, nullptr);
        if (!depthTest)
            glEnable(GL_DEPTH_TEST);
    }

    void RendererAPI::DrawIndexedBaseVertex(uint32_t count, uint32_t baseIndex, uint32_t baseVertex, PrimitiveType type)
    {
        glDrawElementsBaseVertex(PrismToOpenGLPrimitiveType(type), count, GL_UNSIGNED_INT,
            (void*)(sizeof(uint32_t) * baseIndex), baseVertex);
    }

    void RendererAPI::SetLineThickness(float thickness)
    {
        glLineWidth(thickness);
    }

    void RendererAPI::MemoryBarriers(BarrierFlags flags)
    {
        glMemoryBarrier(PrismToOpenGLMemoryBarrier(flags));
    }
}
