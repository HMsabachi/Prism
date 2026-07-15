#pragma once

#include "RendererTypes.h"
#include "RendererAPI.h"
#include "RenderCommandQueue.h"

#include <glm/glm.hpp>
#include "Prism/Core/Math/AABB.h"

namespace Prism
{
    class Camera;
    class Mesh;
    class Material;
    class RenderPass;
    class VertexInput;
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
        static void Shutdown();

        // 高层转发（多态分发到 s_RendererAPI）
        static void BeginFrame();
        static void EndFrame();

        static void BeginRenderPass(Ref<RenderPass> renderPass, bool clear = true);
        static void EndRenderPass();
        static void SubmitFullscreenQuad(Ref<VertexInput> vertexInput, Ref<Material> material);
        static void SetSceneEnvironment(const Ref<SceneEnvironment>& environment, const Ref<Image2D>& shadow);
        static std::pair<Ref<TextureCube>, Ref<TextureCube>> CreateEnvironmentMap(const std::string& filepath);
        static void RenderMesh(Ref<VertexInput> vertexInput, Ref<Mesh> mesh, Ref<Material> material, uint32_t submeshIndex, const glm::mat4& transform, uint32_t pass);
        static void RenderQuad(Ref<VertexInput> vertexInput, Ref<Material> material, const glm::mat4& transform);

        static RenderAPICapabilities& GetCapabilities();

        // 低层静态（Phase 6 收敛进 OpenGLRenderer Utils static）
        static void Clear();
        static void Clear(float r, float g, float b, float a = 1.0f);
        static void SetClearColor(float r, float g, float b, float a);
        static void ClearMagenta();

        static void DrawIndexed(uint32_t count, PrimitiveType type, bool depthTest = true);
        static void DrawIndexedBaseVertex(uint32_t count, uint32_t baseIndex, uint32_t baseVertex, PrimitiveType type = PrimitiveType::Triangles);

        // For OpenGL
        static void SetLineThickness(float thickness);
        static void MemoryBarriers(RendererAPI::BarrierFlags flags);

        static void WaitAndRender();

#if 0
        // 依赖 Renderer2D::DrawLine，待跟进 Hazel SceneRenderer 线框方案后恢复
        static void DrawAABB(const AABB& aabb, const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f,0.0f,0.0f,1.0f));
        static void DrawAABB(Ref<Mesh> mesh, const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f));
#endif
    public:
        static RenderCommandQueue& GetRenderCommandQueue();
    };
}
