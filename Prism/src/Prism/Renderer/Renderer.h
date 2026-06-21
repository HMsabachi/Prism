#pragma once

#include "RendererTypes.h"
#include "RenderCommandQueue.h"
#include "RenderPass.h"

#include "Mesh.h"

namespace Prism
{
    class ShaderLibrary;
    class Camera;
    class Mesh;
    class FrameUniformBuffer;
    class Material;
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

        static void SubmitQuad(Ref<Material> material, const glm::mat4& transform = glm::mat4(1.0f));
        static void SubmitFullscreenQuad(Ref<Material> material);
        //static void SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform, Ref<Material> overrideMaterial = nullptr);


        static void DrawAABB(const AABB& aabb, const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f,0.0f,0.0f,1.0f));
        static void DrawAABB(Ref<Mesh> mesh, const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f));
    public:
        static RenderCommandQueue& GetRenderCommandQueue();
        static FrameUniformBuffer& GetFrameUBO();
    };
}
