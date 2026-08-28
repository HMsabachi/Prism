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

#include "Prism/Renderer/Texture.h"
#include "Prism/Renderer/ComputeShader/ComputeShader.h"
#include "Prism/Renderer/Buffer/UniformBuffer.h"
#include "Camera/Camera.h"

namespace Prism
{
    RendererAPIType RendererAPI::s_CurrentRendererAPI = RendererAPIType::None;

    static RendererAPI* s_RendererAPI = nullptr;
    static RendererConfig s_Config;
    bool Renderer::s_Initialized = false;
    static Ref<ComputeShader> s_EnvironmentShader;

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
        if (s_RendererAPI)
        {
            s_EnvironmentShader = nullptr;
            s_RendererAPI->Shutdown();
            uint32_t delayFrames = s_Config.FramesInFlight;
            delayFrames = (delayFrames == 0) ? 1 : delayFrames;
            for (uint32_t i = 0; i < delayFrames && i < s_ResourceFreeQueueMax; i++)
            {
                auto& queue = Renderer::GetRenderResourceReleaseQueue(i);
                queue.Execute();
            }

            s_Initialized = false;
            delete s_RendererAPI;
            s_RendererAPI = nullptr;
        }
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


	std::pair<Ref<TextureCube>, Ref<TextureCube>> Renderer::CreateEnvironmentMap(const std::string& filepath)
	{
        PR_PROFILE_FUNCTION();
        const uint32_t cubemapSize = 2048;
        const uint32_t irradianceMapSize = 32;

        Ref<TextureCube> envUnfiltered = TextureCube::Create(ImageFormat::RGBA32F, cubemapSize, cubemapSize);
        if (!s_EnvironmentShader)
            s_EnvironmentShader = ComputeShader::Create("Assets/Shaders/Environment.ComputeShader");
        Ref<Texture2D> envEquirect = Texture2D::Create(filepath);
        PR_CORE_ASSERT(envEquirect->GetFormat() == ImageFormat::RGBA32F, "Texture is not HDR!");

        int toCubeKernel = s_EnvironmentShader->FindKernel("CSEquirectToCube");
        s_EnvironmentShader->SetTexture(toCubeKernel, "u_EquirectangularTex", envEquirect);
        s_EnvironmentShader->SetImage(toCubeKernel, "o_OutputCube", envUnfiltered);
        s_EnvironmentShader->Dispatch(toCubeKernel, cubemapSize / 32, cubemapSize / 32, 6);
        envUnfiltered->GetImage()->GenerateMipMap();

        Ref<TextureCube> envFiltered = TextureCube::Create(ImageFormat::RGBA32F, cubemapSize, cubemapSize);
        envUnfiltered->GetImage()->CopyTo(envFiltered->GetImage());

        Ref<UniformBuffer> mipFilterUBO = UniformBuffer::Create(sizeof(float));
        int mipFilter = s_EnvironmentShader->FindKernel("CSMipFilter");
        s_EnvironmentShader->SetTexture(mipFilter, "u_InputCubeMap", envUnfiltered);
        const float deltaRoughness = 1.0f / glm::max((float)(envFiltered->GetMipLevelCount() - 1.0f), 1.0f);
        for (uint32_t level = 1, size = cubemapSize / 2; level < envFiltered->GetMipLevelCount(); level++, size /= 2)
        {
            const uint32_t numGroups = glm::max((uint32_t)1, size / 32);
            s_EnvironmentShader->SetImage(mipFilter, "o_OutputCube", envFiltered, level);
            float roughness = level * deltaRoughness;
            mipFilterUBO->SetData(&roughness, sizeof(float));
            s_EnvironmentShader->SetUniformBuffer(mipFilter, "MipFilterParams", mipFilterUBO);
            s_EnvironmentShader->Dispatch(mipFilter, numGroups, numGroups, 6);
        }

        Ref<TextureCube> irradianceMap = TextureCube::Create(ImageFormat::RGBA32F, irradianceMapSize, irradianceMapSize);
        int irradiance = s_EnvironmentShader->FindKernel("CSIrradiance");
        s_EnvironmentShader->SetTexture(irradiance, "u_InputCubeMap", envFiltered);
        s_EnvironmentShader->SetImage(irradiance, "o_OutputCube", irradianceMap);
        s_EnvironmentShader->Dispatch(irradiance, irradianceMapSize / 32, irradianceMapSize / 32, 6);
        irradianceMap->GetImage()->GenerateMipMap();

        return { envFiltered, irradianceMap };
	}

	// 无参版本：执行当前 submissionIndex 槽（单线程同步路径，兼容旧主循环与 GetData sync）。
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
