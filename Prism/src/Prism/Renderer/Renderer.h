#pragma once

#include "RendererTypes.h"
#include "RendererAPI.h"
#include "RenderCommandQueue.h"

#include <glm/glm.hpp>
#include "Prism/Core/Math/AABB.h"

namespace Prism
{
    class ShaderLibrary;
    class Camera;
    class Mesh;
    class Material;
    class RenderPass;
}

namespace Prism
{
    class PRISM_API Renderer
    {
    private:

    public:
        typedef void(*RenderCommandFn)(void*);
        Renderer();
        ~Renderer();
        static void* DataAllocate(const void* data, size_t size);
        template<typename FuncT>
        static void Submit(FuncT&& func)
        {
            auto renderCmd = [](void* ptr) {
                auto pFunc = (FuncT*)ptr;
                (*pFunc)();
                pFunc->~FuncT();
                };
            auto storageBuffer = GetRenderCommandQueue().Allocate(renderCmd, sizeof(func));
            new (storageBuffer) FuncT(std::forward<FuncT>(func));
            //renderCmd(&func);
        }
        static void Init();
        static Ref<ShaderLibrary> GetShaderLibrary();

        static void Clear();
        static void Clear(float r, float g, float b, float a = 1.0f);
        static void SetClearColor(float r, float g, float b, float a);
        static void ClearMagenta();

        static void DrawIndexed(uint32_t count, PrimitiveType type, bool depthTest = true);
        static void DrawIndexedBaseVertex(uint32_t count, uint32_t baseIndex, uint32_t baseVertex, PrimitiveType type = PrimitiveType::Triangles);

        // For OpenGL
        static void SetLineThickness(float thickness);
        static void MemoryBarriers(RendererAPI::BarrierFlags flags);


        static void BeginRenderPass(Ref<RenderPass> renderPass, bool clear = true);
        static void EndRenderPass();

        static void WaitAndRender();

        static void DrawAABB(const AABB& aabb, const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f,0.0f,0.0f,1.0f));
        static void DrawAABB(Ref<Mesh> mesh, const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f));
    public:
        static RenderCommandQueue& GetRenderCommandQueue();
    };
}
