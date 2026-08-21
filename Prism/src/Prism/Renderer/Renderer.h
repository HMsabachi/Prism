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
        uint32_t FramesInFlight = 3;
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

        static RendererAPI* GetAPI();
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
