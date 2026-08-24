#include "prpch.h"
#include "VulkanRenderer.h"

#include "VulkanContext.h"
#include "VulkanSwapChain.h"
#include "VulkanDescriptorSet.h"
#include "VulkanFramebuffer.h"
#include "VulkanImage.h"
#include "VulkanMaterialBackend.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanUniformBuffer.h"
#include "VulkanShaderStorageBuffer.h"
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"

#include "Prism/Renderer/Renderer.h"

#include <glm/glm.hpp>
#include <map>

namespace Prism
{

    struct VulkanRendererData
    {
        RenderAPICapabilities RenderCaps;

        VkCommandBuffer ActiveCommandBuffer = nullptr;

        Ref<VulkanVertexBuffer> FullscreenQuadVB;
        Ref<VulkanIndexBuffer> FullscreenQuadIB;


        VulkanDescriptorSet GlobalDescriptorSet;
    };

    static VulkanRendererData* s_Data = nullptr;

    namespace Utils
    {
        static std::string VendorToString(uint32_t vendorID)
        {
            switch (vendorID)
            {
            case 0x10DE: return "NVIDIA";
            case 0x8086: return "Intel";
            case 0x1002: return "AMD";
            case 0x1022: return "AMD";
            case 0x13B5: return "ARM";
            case 0x5143: return "Qualcomm";
            }
            return "Unknown";
        }

        static int MaxSampleCount(VkSampleCountFlags counts)
        {
            for (int i = 31; i >= 0; i--)
            {
                if (counts & (1u << i))
                    return 1 << i;
            }
            return 1;
        }
    }

    void VulkanRenderer::Init()
    {
        PR_CORE_ASSERT(!s_Data, "VulkanRenderer 已初始化!");
        s_Data = new VulkanRendererData();

        const auto& properties = VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetProperties();
        auto& caps = s_Data->RenderCaps;
        caps.Vendor = Utils::VendorToString(properties.vendorID);
        caps.Renderer = properties.deviceName;
        caps.Version = std::to_string(VK_VERSION_MAJOR(properties.apiVersion)) + "."
            + std::to_string(VK_VERSION_MINOR(properties.apiVersion)) + "."
            + std::to_string(VK_VERSION_PATCH(properties.apiVersion));
        caps.MaxSamples = Utils::MaxSampleCount(properties.limits.framebufferColorSampleCounts
            & properties.limits.framebufferDepthSampleCounts);
        caps.MaxAnisotropy = (float)properties.limits.maxSamplerAnisotropy;
        caps.MaxTextureUnits = (int)properties.limits.maxPerStageDescriptorSamplers;
        for (int i = 0; i < 3; i++)
        {
            caps.MaxGroupCount[i] = (int)properties.limits.maxComputeWorkGroupCount[i];
            caps.MaxGroupSize[i] = (int)properties.limits.maxComputeWorkGroupSize[i];
        }
        caps.MaxInvocations = (int)properties.limits.maxComputeWorkGroupInvocations;

        struct QuadVertex
        {
            glm::vec3 Position;
            glm::vec2 TexCoord;
        };
        float x = -1.0f, y = -1.0f, width = 2.0f, height = 2.0f;
        QuadVertex data[4] = {
            { { x,         y,          0.1f }, { 0.0f, 0.0f } },
            { { x + width, y,          0.1f }, { 1.0f, 0.0f } },
            { { x + width, y + height, 0.1f }, { 1.0f, 1.0f } },
            { { x,         y + height, 0.1f }, { 0.0f, 1.0f } },
        };
        s_Data->FullscreenQuadVB = VertexBuffer::Create(data, 4 * sizeof(QuadVertex)).As<VulkanVertexBuffer>();
        VertexBufferLayout layout = {
            { ShaderDataType::Float3, "a_Position",  VertexSemantic::Position },
            { ShaderDataType::Float2, "a_TexCoord",  VertexSemantic::TexCoord0 }
        };
        s_Data->FullscreenQuadVB->SetLayout(layout);
        uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };
        s_Data->FullscreenQuadIB = IndexBuffer::Create(indices, 6 * sizeof(uint32_t)).As<VulkanIndexBuffer>();
    }

    void VulkanRenderer::Shutdown()
    {
        VulkanPipelineCache::Clear();
        delete s_Data;
        s_Data = nullptr;
    }

    RenderAPICapabilities& VulkanRenderer::GetCapabilities()
    {
        return s_Data->RenderCaps;
    }

    VkCommandBuffer VulkanRenderer::GetCurrentCommandBuffer()
    {
        return s_Data ? s_Data->ActiveCommandBuffer : VK_NULL_HANDLE;
    }

    void VulkanRenderer::BeginFrame()
    {
        Renderer::Submit([]()
        {
            // 用 RT 帧槽（acquire 刚推进过，同队列前序）而非主线程计数器：
            // 主线程计数器在 RT 滞后时会读到下一帧的值，提前一帧执行释放队列不满足 FIF 延迟。
            const uint32_t releaseIndex = Renderer::RT_GetCurrentFrameIndex() % Renderer::GetConfig().FramesInFlight;
            Renderer::GetRenderResourceReleaseQueue(releaseIndex).Execute();

            Ref<VulkanContext> context = VulkanContext::Get();
            VulkanSwapChain& swapChain = context->GetSwapChain();

            VkCommandBufferBeginInfo cmdBufInfo = {};
            cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmdBufInfo.pNext = nullptr;

            VkCommandBuffer drawCommandBuffer = swapChain.GetCurrentDrawCommandBuffer();
            s_Data->ActiveCommandBuffer = drawCommandBuffer;
            PR_CORE_ASSERT(s_Data->ActiveCommandBuffer);
            VK_CHECK_RESULT(vkBeginCommandBuffer(drawCommandBuffer, &cmdBufInfo));
        });
    }

    void VulkanRenderer::EndFrame()
    {
        Renderer::Submit([]()
        {
            VK_CHECK_RESULT(vkEndCommandBuffer(s_Data->ActiveCommandBuffer));
            s_Data->ActiveCommandBuffer = nullptr;
        });
    }

    void VulkanRenderer::SetGlobalUniformBuffer(uint32_t binding, Ref<UniformBuffer> ubo)
    {
        Renderer::Submit([binding, ubo]()
        {
            s_Data->GlobalDescriptorSet.SetInput(binding, ubo.As<VulkanUniformBuffer>());
        });
    }

    void VulkanRenderer::SetGlobalShaderStorageBuffer(uint32_t binding, Ref<ShaderStorageBuffer> ssbo)
    {
        Renderer::Submit([binding, ssbo]()
        {
            s_Data->GlobalDescriptorSet.SetInput(binding, ssbo.As<VulkanShaderStorageBuffer>());
        });
    }

    void VulkanRenderer::SetGlobalTexture(uint32_t binding, Ref<Image> image)
    {
        Renderer::Submit([binding, image]()
        {
            if (dynamic_cast<VulkanImage2D*>(const_cast<Image*>(image.Raw())))
                s_Data->GlobalDescriptorSet.SetInput(binding, image.As<VulkanImage2D>());
            else if (dynamic_cast<VulkanImageCube*>(const_cast<Image*>(image.Raw())))
                s_Data->GlobalDescriptorSet.SetInput(binding, image.As<VulkanImageCube>());
        });
    }

    //////////////////////////////////////////////////////////////////////////////////
    // S4/S5（材质/mesh/compute）落地时补齐
    //////////////////////////////////////////////////////////////////////////////////

    void VulkanRenderer::BeginRenderPass(Ref<RenderPass> renderPass, bool clear)
    {
        // TODO: clear 参数当前忽略（loadOp=CLEAR 固化在 framebuffer 的 renderpass 里），clear=false 需求出现时改 vkCmdClearAttachments
        Renderer::Submit([renderPass]()
        {
            Ref<VulkanFramebuffer> framebuffer = renderPass->GetSpecification().TargetFramebuffer.As<VulkanFramebuffer>();
            PR_CORE_ASSERT(framebuffer);

            uint32_t width = framebuffer->GetWidth();
            uint32_t height = framebuffer->GetHeight();

            VkRenderPassBeginInfo renderPassBeginInfo = {};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.renderPass = framebuffer->GetRenderPass();
            renderPassBeginInfo.framebuffer = framebuffer->GetVulkanFramebuffer();
            renderPassBeginInfo.renderArea.offset = { 0, 0 };
            renderPassBeginInfo.renderArea.extent = { width, height };

            const auto& clearValues = framebuffer->GetVulkanClearValues();
            renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassBeginInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(s_Data->ActiveCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport = {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float)width;
            viewport.height = (float)height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(s_Data->ActiveCommandBuffer, 0, 1, &viewport);

            VkRect2D scissor = {};
            scissor.offset = { 0, 0 };
            scissor.extent = { width, height };
            vkCmdSetScissor(s_Data->ActiveCommandBuffer, 0, 1, &scissor);
        });
    }

    void VulkanRenderer::EndRenderPass()
    {
        Renderer::Submit([]()
        {
            vkCmdEndRenderPass(s_Data->ActiveCommandBuffer);
        });
    }

    void VulkanRenderer::SubmitFullscreenQuad(Ref<Material> material, uint32_t passIndex, uint32_t drawIndex)
    {
    }

    void VulkanRenderer::SetSceneEnvironment(const Ref<SceneEnvironment>& environment)
    {
    }

    std::pair<Ref<TextureCube>, Ref<TextureCube>> VulkanRenderer::CreateEnvironmentMap(const std::string& filepath)
    {
        return {};
    }

    void VulkanRenderer::RenderMesh(Ref<Mesh> mesh, uint32_t submeshIndex, Ref<Material> material,
        uint32_t passIndex, uint32_t drawIndex)
    {
    }

    void VulkanRenderer::RenderQuad(Ref<Material> material, uint32_t passIndex, uint32_t drawIndex)
    {
    }

    void VulkanRenderer::DispatchCompute(Ref<Shader> kernelShader,
        const std::vector<ComputeResourceBinding>& bindings,
        uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
    {
    }

}
