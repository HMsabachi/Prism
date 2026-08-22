#include "prpch.h"
#include "Renderer.h"

#include "RendererAPI.h"
#include "Buffer/Framebuffer.h"
#include "Prism/Core/RenderThread.h"
#include "Prism/Core/Application.h"
#include "Prism/Renderer/RendererContext.h"
#include "Platform/OpenGL/OpenGLRenderer.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/Vulkan.h"

#include "Camera/Camera.h"

namespace Prism
{
    RendererAPIType RendererAPI::s_CurrentRendererAPI = RendererAPIType::None;

    static RendererAPI* s_RendererAPI = nullptr;
    static RendererConfig s_Config;
    bool Renderer::s_Initialized = false;

    static void InitRendererAPI()
    {
        switch (RendererAPI::Current())
        {
        case RendererAPIType::OpenGL:
            s_RendererAPI = new OpenGLRenderer();
            break;
        case RendererAPIType::Vulkan:
            s_RendererAPI = new VulkanRenderer();
            // 必须显式设为 VulkanFramesInFlight：交换链同步对象/UBO 轮转/释放队列延迟
            // 都按 2 设计，默认值 3 会与交换链 imageCount 取 min 后产生错位
            s_Config.FramesInFlight = VulkanFramesInFlight;
            break;
        default:
            PR_CORE_ASSERT(false, "未支持的 RendererAPI");
        }
    }

    // 双命令队列：主线程写 submissionIndex 槽，渲染线程读 (submissionIndex+1)%2 槽。
    // 两个线程碰不同队列，无需锁。唯一同步点是 atomic 索引翻转（SwapQueues）。
    constexpr static uint32_t s_RenderCommandQueueCount = 2;
    static RenderCommandQueue s_CommandQueue[s_RenderCommandQueueCount];
    static std::atomic<uint32_t> s_RenderCommandQueueSubmissionIndex = 0;

    // 资源释放队列：按帧槽位分隔，延迟若干帧后执行（GL=1，Vulkan=FramesInFlight）。
    // 实际使用数量由 s_Config.FramesInFlight 决定，数组大小取最大可能值。
    constexpr static uint32_t s_ResourceFreeQueueMax = 4;
    static RenderCommandQueue s_ResourceFreeQueue[s_ResourceFreeQueueMax];

    void Renderer::Init()
    {
        InitRendererAPI();
        // Make sure we don't have more frames in flight than swapchain images
        s_Config.FramesInFlight = glm::min<uint32_t>(s_Config.FramesInFlight, Application::Get().GetWindow().GetRenderContext()->GetImageCount());

        s_RendererAPI->Init();
        s_Initialized = true;
    }

    void Renderer::Shutdown()
    {
        s_Initialized = false;

        if (s_RendererAPI)
        {
            uint32_t delayFrames = s_Config.FramesInFlight;
            delayFrames = (delayFrames == 0) ? 1 : delayFrames;
            for (uint32_t i = 0; i < delayFrames && i < s_ResourceFreeQueueMax; i++)
            {
                auto& queue = Renderer::GetRenderResourceReleaseQueue(i);
                queue.Execute();
            }

            s_RendererAPI->Shutdown();
            delete s_RendererAPI;
            s_RendererAPI = nullptr;
        }

        /*delete s_CommandQueue[0];
        delete s_CommandQueue[1];
        s_CommandQueue[0] = s_CommandQueue[1] = nullptr;*/
    }


    const RendererConfig& Renderer::GetConfig() { return s_Config; }

    Renderer::Renderer() {}
    Renderer::~Renderer() {}

    void* Renderer::DataAllocate(const void* data, size_t size)
    {
        return GetRenderCommandQueue().DataAllocate(data, size);
    }

    RendererAPI* Renderer::GetAPI() { return s_RendererAPI; }

    RenderAPICapabilities& Renderer::GetCapabilities() { return s_RendererAPI->GetCapabilities(); }

    // 无参版本：执行当前 submissionIndex 槽（单线程同步路径，兼容旧主循环与 GetData sync）。
    // 保留是因为 Application/OpenGLContext/SSBO 的既有调用点还没切到 Pump/Kick 流程（Phase 3 再改）。
    void Renderer::WaitAndRender()
    {
        s_CommandQueue[GetRenderQueueSubmissionIndex()].Execute();
    }

    void Renderer::WaitAndRender(RenderThread* renderThread)
    {
        PR_PROFILE_FUNCTION();
        if (renderThread)
        {
            renderThread->WaitAndSet(RenderThread::State::Kick, RenderThread::State::Busy);
        }

        s_CommandQueue[GetRenderQueueIndex()].Execute();

        if (renderThread)
        {
            renderThread->Set(RenderThread::State::Idle);
        }
    }

    void Renderer::RenderThreadFunc(RenderThread* renderThread)
    {
        PR_PROFILE_THREAD("Render Thread");
        PR_CORE_TRACE("Render Thread is running");
        while (renderThread->IsRunning())
        {
            WaitAndRender(renderThread);
        }
    }

    void Renderer::SwapQueues()
    {
        s_RenderCommandQueueSubmissionIndex = (s_RenderCommandQueueSubmissionIndex + 1) % s_RenderCommandQueueCount;
    }

    uint32_t Renderer::GetRenderQueueIndex()
    {
        // 渲染线程读取的槽 = 主线程写入槽的另一侧
        return (s_RenderCommandQueueSubmissionIndex + 1) % s_RenderCommandQueueCount;
    }

    uint32_t Renderer::GetRenderQueueSubmissionIndex()
    {
        return s_RenderCommandQueueSubmissionIndex;
    }

    uint32_t Renderer::GetCurrentFrameIndex()
    {
        return Application::Get().GetCurrentFrameIndex();
    }

    uint32_t Renderer::RT_GetCurrentFrameIndex()
    {
        return Application::Get().GetWindow().GetRenderContext()->GetCurrentFrameIndex();
    }


#if 0
    void Renderer::DrawAABB(Ref<Mesh> mesh, const glm::mat4& transform, const glm::vec4& color)
    {
        for (Submesh& submesh : mesh->m_Submeshes)
        {
            auto& aabb = submesh.BoundingBox;
            auto aabbTransform = transform * submesh.Transform;
            DrawAABB(aabb, aabbTransform);
        }
    }

    void Renderer::DrawAABB(const AABB& aabb, const glm::mat4& transform, const glm::vec4& color)
    {
        glm::vec4 min = { aabb.Min.x, aabb.Min.y, aabb.Min.z, 1.0f };
        glm::vec4 max = { aabb.Max.x, aabb.Max.y, aabb.Max.z, 1.0f };

        glm::vec4 corners[8] =
        {
            transform * glm::vec4 { aabb.Min.x, aabb.Min.y, aabb.Max.z, 1.0f },
            transform * glm::vec4 { aabb.Min.x, aabb.Max.y, aabb.Max.z, 1.0f },
            transform * glm::vec4 { aabb.Max.x, aabb.Max.y, aabb.Max.z, 1.0f },
            transform * glm::vec4 { aabb.Max.x, aabb.Min.y, aabb.Max.z, 1.0f },

            transform * glm::vec4 { aabb.Min.x, aabb.Min.y, aabb.Min.z, 1.0f },
            transform * glm::vec4 { aabb.Min.x, aabb.Max.y, aabb.Min.z, 1.0f },
            transform * glm::vec4 { aabb.Max.x, aabb.Max.y, aabb.Min.z, 1.0f },
            transform * glm::vec4 { aabb.Max.x, aabb.Min.y, aabb.Min.z, 1.0f }
        };
        for (uint32_t i = 0; i < 4; i++)
            Renderer2D::DrawLine(corners[i], corners[(i + 1) % 4], color);

        for (uint32_t i = 0; i < 4; i++)
            Renderer2D::DrawLine(corners[i + 4], corners[((i + 1) % 4) + 4], color);

        for (uint32_t i = 0; i < 4; i++)
            Renderer2D::DrawLine(corners[i], corners[i + 4], color);
    }
#endif

    RenderCommandQueue& Renderer::GetRenderCommandQueue() { return s_CommandQueue[s_RenderCommandQueueSubmissionIndex]; }

    RenderCommandQueue& Renderer::GetRenderResourceReleaseQueue(uint32_t index)
    {
        PR_CORE_ASSERT(index < s_ResourceFreeQueueMax, "资源释放队列索引越界");
        return s_ResourceFreeQueue[index];
    }
}
