#pragma once

#include "RendererTypes.h"
#include "RendererAPI.h"
#include "RenderCommandQueue.h"
#include "Prism/Core/RenderThread.h"

#include <glm/glm.hpp>
#include "Prism/Core/Math/AABB.h"

namespace Prism
{
    class Camera;
    class Mesh;
    class Material;
    class RenderPass;
    class RenderThread;
    class Image;
    class UniformBuffer;
    class ShaderStorageBuffer;
    class RenderCommandQueue;
}

namespace Prism
{
    struct RendererConfig
    {
        uint32_t FramesInFlight = 1;
    };
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
            //if (RenderThread::IsCurrentThreadRT())
                //PR_CORE_ERROR("Renderer::Submit 被非主线程调用!");
            auto renderCmd = [](void* ptr) {
                auto pFunc = (FuncT*)ptr;
                (*pFunc)();
                pFunc->~FuncT();
                };
            auto storageBuffer = GetRenderCommandQueue().Allocate(renderCmd, sizeof(func));
            new (storageBuffer) FuncT(std::forward<FuncT>(func));
        }


        static bool s_Initialized;

        template<typename FuncT>
        static void SubmitResourceFree(FuncT&& func)
        {
            if (!s_Initialized)
                return;

            auto renderCmd = [](void* ptr) {
                auto pFunc = (FuncT*)ptr;
                (*pFunc)();
                pFunc->~FuncT();
                };

            if (RenderThread::IsCurrentThreadRT())
            {
                const uint32_t index = RT_GetCurrentFrameIndex();
                auto storageBuffer = GetRenderResourceReleaseQueue(index).Allocate(renderCmd, sizeof(func));
                new (storageBuffer) FuncT(std::forward<FuncT>((FuncT&&)func));
            }
            else
            {
                const uint32_t index = GetCurrentFrameIndex();
                Submit([renderCmd, func, index]()
                {
                    auto storageBuffer = GetRenderResourceReleaseQueue(index).Allocate(renderCmd, sizeof(func));
                    new (storageBuffer) FuncT(std::forward<FuncT>((FuncT&&)func));
                });
            }
        }

        static void Init();
        static void Shutdown();
        static const RendererConfig& GetConfig();

        // 高层转发（多态分发到 s_RendererAPI）
        static void BeginFrame();
        static void EndFrame();

        static void BeginRenderPass(Ref<RenderPass> renderPass, bool clear = true);
        static void EndRenderPass();
        static void SubmitFullscreenQuad(Ref<Material> material, const PrismShaderCompiler::PipelineState* stateOverride = nullptr);
        static void SetSceneEnvironment(const Ref<SceneEnvironment>& environment);
        static void SetUniformBuffer(uint32_t set, uint32_t binding, Ref<UniformBuffer> ubo);
        static void SetShaderStorageBuffer(uint32_t set, uint32_t binding, Ref<ShaderStorageBuffer> ssbo);
        static void SetTexture(uint32_t set, uint32_t binding, Ref<Image> image);
        static std::pair<Ref<TextureCube>, Ref<TextureCube>> CreateEnvironmentMap(const std::string& filepath);
        static void RenderMesh(Ref<Mesh> mesh, Ref<Material> material, uint32_t submeshIndex, const glm::mat4& transform, uint32_t pass, const PrismShaderCompiler::PipelineState* stateOverride = nullptr);
        static void RenderQuad(Ref<Material> material, const glm::mat4& transform, const PrismShaderCompiler::PipelineState* stateOverride = nullptr);

        static void DispatchCompute(Ref<Shader> kernelShader, const std::vector<ComputeResourceBinding>& bindings, uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ);

        static RenderAPICapabilities& GetCapabilities();

        // 渲染线程相关（Phase 1 骨架）
        static void WaitAndRender();
        static void WaitAndRender(RenderThread* renderThread);
        static void RenderThreadFunc(RenderThread* renderThread);
        static void SwapQueues();
        static uint32_t GetRenderQueueIndex();
        static uint32_t GetRenderQueueSubmissionIndex();
        static uint32_t GetCurrentFrameIndex();
        static uint32_t RT_GetCurrentFrameIndex();

#if 0
        // 依赖 Renderer2D::DrawLine，待跟进 Hazel SceneRenderer 线框方案后恢复
        static void DrawAABB(const AABB& aabb, const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f,0.0f,0.0f,1.0f));
        static void DrawAABB(Ref<Mesh> mesh, const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f));
#endif
    public:
        static RenderCommandQueue& GetRenderCommandQueue();
        static RenderCommandQueue& GetRenderResourceReleaseQueue(uint32_t index);
    };
}
