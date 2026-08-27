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
#include "VulkanTexture.h"

#include "Prism/Renderer/Renderer.h"
#include "Prism/Renderer/Material.h"
#include "Prism/Renderer/Mesh.h"

#include <glm/glm.hpp>
#include <map>

namespace Prism
{

    struct VulkanRendererData
    {
        RenderAPICapabilities RenderCaps;

        VkCommandBuffer ActiveCommandBuffer = nullptr;
        Ref<VulkanRenderPass> ActiveRenderPass;

        Ref<VulkanVertexBuffer> FullscreenQuadVB;
        Ref<VulkanIndexBuffer> FullscreenQuadIB;

        Ref<VulkanImage2D> BlackImage2D;
        Ref<VulkanImageCube> BlackImageCube;
        Ref<VulkanTexture2D> BlackTexture2D;
        Ref<VulkanTextureCube> BlackTextureCube;

        VulkanDescriptorSet GlobalDescriptorSet;
        bool IsGlobalDescriptorSetPrepared = false;

        VulkanPipelineCache PipelineCache;
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

        s_Data->PipelineCache.Init();

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

        float blackPixel[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        s_Data->BlackImage2D = Image2D::Create(ImageFormat::RGBA, 1, 1, blackPixel).As<VulkanImage2D>();
        s_Data->BlackImage2D->Invalidate();
        s_Data->BlackImageCube = ImageCube::Create(ImageFormat::RGBA, 1, 1, blackPixel).As<VulkanImageCube>();
        s_Data->BlackImageCube->Invalidate();
        s_Data->BlackTexture2D = Texture2D::Create(ImageFormat::RGBA, 1, 1, blackPixel).As<VulkanTexture2D>();
        s_Data->BlackTextureCube = TextureCube::Create(ImageFormat::RGBA, 1, 1, blackPixel).As<VulkanTextureCube>();
    }

    void VulkanRenderer::Shutdown()
    {
        s_Data->PipelineCache.Shutdown();
        s_Data->FullscreenQuadVB = nullptr;
        s_Data->FullscreenQuadIB = nullptr;
        s_Data->BlackImage2D = nullptr;
        s_Data->BlackImageCube = nullptr;
        s_Data->BlackTexture2D = nullptr;
        s_Data->BlackTextureCube = nullptr;
        delete s_Data;
        s_Data = nullptr;
    }

    VulkanPipelineCache& VulkanRenderer::GetPipelineCache()
    {
        PR_CORE_ASSERT(s_Data, "VulkanRenderer::GetPipelineCache: VulkanRenderer 未初始化!");
        return s_Data->PipelineCache;
    }

    RenderAPICapabilities& VulkanRenderer::GetCapabilities()
    {
        return s_Data->RenderCaps;
    }

    VkCommandBuffer VulkanRenderer::RT_GetCurrentCommandBuffer()
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
            s_Data->IsGlobalDescriptorSetPrepared = false;
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


    void VulkanRenderer::BakeGlobalInputs()
    {
        Renderer::Submit([]()
        {
            s_Data->GlobalDescriptorSet.Bake();
        });
    }

    //////////////////////////////////////////////////////////////////////////////////
    // S4/S5（材质/mesh/compute）落地时补齐
    //////////////////////////////////////////////////////////////////////////////////

    void VulkanRenderer::BeginRenderPass(Ref<RenderPass> renderPass, bool clear)
    {
        // TODO: clear 参数当前忽略（loadOp=CLEAR 固化在 framebuffer 的 renderpass 里），clear=false 需求出现时改 vkCmdClearAttachments
        Ref<VulkanRenderPass> vkRenderPass = renderPass.As<VulkanRenderPass>();
        Renderer::Submit([vkRenderPass]() mutable
        {
            s_Data->ActiveRenderPass = vkRenderPass;
            if (!s_Data->IsGlobalDescriptorSetPrepared)
            {
                s_Data->GlobalDescriptorSet.RT_Prepare();
                s_Data->IsGlobalDescriptorSetPrepared = true;
            }
            vkRenderPass->RT_Prepare(); // 检查 renderpass 资源
            Ref<VulkanFramebuffer> framebuffer = vkRenderPass->GetSpecification().TargetFramebuffer.As<VulkanFramebuffer>();
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
        Ref<VulkanMaterialBackend> backend = material->RT_GetBackend().As<VulkanMaterialBackend>();
        Ref<VulkanShader> shader = material->GetProgram(passIndex).As<VulkanShader>();
        Renderer::Submit([=]()
        {
            auto& pCache = s_Data->PipelineCache;
            VkCommandBuffer cmdBuf = s_Data->ActiveCommandBuffer;
            StaticVector<VertexBufferLayout, 4> vertexLayouts;
            vertexLayouts.push_back(s_Data->FullscreenQuadVB->GetLayout());
            VulkanPipelineSpecification spec{};
            spec.Framebuffer = s_Data->ActiveRenderPass->GetSpecification().TargetFramebuffer.As<VulkanFramebuffer>();
            spec.Shader = shader;
            spec.State = material->GetRenderState(passIndex);
            spec.Topology = PrimitiveType::Triangles;
            spec.VertexLayouts = vertexLayouts;
            // 绑定 Pipeline
            WeakRef<VulkanPipeline> pipeline = pCache.Get(spec);
            pipeline->RT_Bind(cmdBuf);
            pipeline->RT_BindGlobalSet(cmdBuf, s_Data->GlobalDescriptorSet.RT_GetDescriptorSet());
            pipeline->RT_BindRenderPassSet(cmdBuf, s_Data->ActiveRenderPass->RT_GetDescriptorSet());
            pipeline->RT_BindMaterialSet(cmdBuf, backend->RT_GetDescriptorSet());
            // Push Constant
            int32_t drawIndexPC = (int32_t)drawIndex;
            vkCmdPushConstants(cmdBuf, pipeline->GetPipelineLayout(),
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int32_t), &drawIndexPC);
            // 绑定 Vertex Buffer
            VkBuffer vertBufs[] = { s_Data->FullscreenQuadVB->GetVulkanBuffer() };
            VkDeviceSize vertOffs[] = { 0 };
            vkCmdBindVertexBuffers(cmdBuf, 0, 1, vertBufs, vertOffs);
            // 绑定 Index Buffer
            vkCmdBindIndexBuffer(cmdBuf, s_Data->FullscreenQuadIB->GetVulkanBuffer(), 0, VK_INDEX_TYPE_UINT32);
            // 绘制
            vkCmdDrawIndexed(cmdBuf, s_Data->FullscreenQuadIB->GetCount(), 1, 0, 0, 0);
        });
    }

    void VulkanRenderer::SetSceneEnvironment(const Ref<SceneEnvironment>& environment)
    {
    }

    std::pair<Ref<TextureCube>, Ref<TextureCube>> VulkanRenderer::CreateEnvironmentMap(const std::string& filepath)
    {
        return {s_Data->BlackTextureCube, s_Data->BlackTextureCube };
    }

    void VulkanRenderer::RenderMesh(Ref<Mesh> mesh, uint32_t submeshIndex, Ref<Material> material,
        uint32_t passIndex, uint32_t drawIndex)
    {
        Ref<VulkanVertexBuffer> vertexBuffer = mesh->m_VertexBuffer.As<VulkanVertexBuffer>();
        Ref<VulkanIndexBuffer> indexBuffer = mesh->m_IndexBuffer.As<VulkanIndexBuffer>();
        Ref<VulkanMaterialBackend> backend = material->RT_GetBackend().As<VulkanMaterialBackend>();
        Ref<VulkanShader> shader = material->GetProgram(passIndex).As<VulkanShader>();
        Renderer::Submit([=]()
        {
            auto& pCache = s_Data->PipelineCache;
            VkCommandBuffer cmdBuf = s_Data->ActiveCommandBuffer;
            auto& submesh = mesh->m_Submeshes[submeshIndex];
            StaticVector<VertexBufferLayout, 4> vertexLayouts;
            vertexLayouts.push_back(vertexBuffer->GetLayout());
            VulkanPipelineSpecification spec{};
            spec.Framebuffer = s_Data->ActiveRenderPass->GetSpecification().TargetFramebuffer.As<VulkanFramebuffer>();
            spec.Shader = shader;
            spec.State = material->GetRenderState(passIndex);
            spec.Topology = PrimitiveType::Triangles;
            spec.VertexLayouts = vertexLayouts;
            // 绑定 Pipeline
            WeakRef<VulkanPipeline> pipeline = pCache.Get(spec);
            pipeline->RT_Bind(cmdBuf);
            pipeline->RT_BindGlobalSet(cmdBuf, s_Data->GlobalDescriptorSet.RT_GetDescriptorSet());
            pipeline->RT_BindRenderPassSet(cmdBuf, s_Data->ActiveRenderPass->RT_GetDescriptorSet());
            pipeline->RT_BindMaterialSet(cmdBuf, backend->RT_GetDescriptorSet());
            // Push Constant
            int32_t drawIndexPC = (int32_t)drawIndex;
            vkCmdPushConstants(cmdBuf, pipeline->GetPipelineLayout(),
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int32_t), &drawIndexPC);
            // 绑定 Vertex Buffer
            VkBuffer vertBufs[] = { vertexBuffer->GetVulkanBuffer() };
            VkDeviceSize vertOffs[] = { 0 };
            vkCmdBindVertexBuffers(cmdBuf, 0, 1, vertBufs, vertOffs);
            // 绑定 Index Buffer
            VkBuffer indBuf = indexBuffer->GetVulkanBuffer();
            vkCmdBindIndexBuffer(cmdBuf, indBuf, 0, VK_INDEX_TYPE_UINT32);
            // 绘制
            uint32_t count = submesh.IndexCount, baseIndex = submesh.BaseIndex, baseVertex = submesh.BaseVertex;
            vkCmdDrawIndexed(cmdBuf, count, 1, baseIndex, (int32_t)baseVertex, 0);
        });
    }

    void VulkanRenderer::RenderQuad(Ref<Material> material, uint32_t passIndex, uint32_t drawIndex)
    {
        SubmitFullscreenQuad(material, passIndex, drawIndex);
    }

    void VulkanRenderer::DispatchCompute(Ref<Shader> kernelShader,
        const std::vector<ComputeResourceBinding>& bindings,
        uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
    {
    }


    WeakRef<VulkanImage2D> VulkanRenderer::RT_GetBlackImage2D()
    {
        return s_Data->BlackImage2D;
    }

    WeakRef<VulkanImageCube> VulkanRenderer::RT_GetBlackImageCube()
    {
        return s_Data->BlackImageCube;
    }
}
