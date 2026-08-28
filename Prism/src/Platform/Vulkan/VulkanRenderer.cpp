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
#include "VulkanShader.h"
#include "VulkanUniformBuffer.h"
#include "VulkanShaderStorageBuffer.h"
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"
#include "VulkanTexture.h"

#include "Prism/Renderer/Renderer.h"
#include "Prism/Renderer/Material.h"
#include "Prism/Renderer/Mesh.h"
#include "Prism/Renderer/ComputeShader/ComputeShader.h"

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
        Ref<VulkanUniformBuffer> EmptyUniformBuffer;
        Ref<VulkanShaderStorageBuffer> EmptyShaderStorageBuffer;
        Ref<VulkanTexture2D> BlackTexture2D;
        Ref<VulkanTextureCube> BlackTextureCube;

        VulkanDescriptorSet GlobalDescriptorSet;
        bool IsGlobalDescriptorSetPrepared = false;

        VulkanPipelineCache PipelineCache;
    };

    static VulkanRendererData* s_Data = nullptr;
    static Ref<ComputeShader> s_EnvironmentShader;

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

        float blackPixel[16] = { 0.0f };
        s_Data->BlackImage2D = Image2D::Create(ImageFormat::RGBA, 1, 1, blackPixel).As<VulkanImage2D>();
        s_Data->BlackImage2D->SetExtraUsage(VK_IMAGE_USAGE_STORAGE_BIT);
        s_Data->BlackImage2D->Invalidate();
        s_Data->BlackImageCube = ImageCube::Create(ImageFormat::RGBA, 1, 1, blackPixel).As<VulkanImageCube>();
        s_Data->BlackImageCube->Invalidate();
        s_Data->BlackTexture2D = Texture2D::Create(ImageFormat::RGBA, 1, 1, blackPixel).As<VulkanTexture2D>();
        s_Data->BlackTextureCube = TextureCube::Create(ImageFormat::RGBA, 1, 1, blackPixel).As<VulkanTextureCube>();
        s_Data->EmptyUniformBuffer = UniformBuffer::Create(1).As<VulkanUniformBuffer>();
        s_Data->EmptyShaderStorageBuffer = ShaderStorageBuffer::Create(1).As <VulkanShaderStorageBuffer>();
    }

    void VulkanRenderer::Shutdown()
    {
        // 先于 s_Data 销毁：static Ref 析构链（VulkanShader -> PipelineCache::Erase 依赖 s_Data）
        s_EnvironmentShader = nullptr;
        s_Data->FullscreenQuadVB = nullptr;
        s_Data->FullscreenQuadIB = nullptr;
        s_Data->BlackImage2D = nullptr;
        s_Data->BlackImageCube = nullptr;
        s_Data->BlackTexture2D = nullptr;
        s_Data->BlackTextureCube = nullptr;
        s_Data->EmptyUniformBuffer = nullptr;
        s_Data->EmptyShaderStorageBuffer = nullptr;
        s_Data->PipelineCache.Shutdown();
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
            uint32_t drawIndexPC = drawIndex;
            pipeline->RT_Bind(cmdBuf);
            pipeline->RT_BindGlobalSet(cmdBuf, s_Data->GlobalDescriptorSet.RT_GetDescriptorSet());
            pipeline->RT_BindRenderPassSet(cmdBuf, s_Data->ActiveRenderPass->RT_GetDescriptorSet());
            pipeline->RT_BindMaterialSet(cmdBuf, backend->RT_GetDescriptorSet());
            pipeline->RT_BindPushConstant(cmdBuf, 0, sizeof(uint32_t), &drawIndexPC);
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
            int32_t drawIndexPC = (int32_t)drawIndex;
            pipeline->RT_Bind(cmdBuf);
            pipeline->RT_BindGlobalSet(cmdBuf, s_Data->GlobalDescriptorSet.RT_GetDescriptorSet());
            pipeline->RT_BindRenderPassSet(cmdBuf, s_Data->ActiveRenderPass->RT_GetDescriptorSet());
            pipeline->RT_BindMaterialSet(cmdBuf, backend->RT_GetDescriptorSet());
            pipeline->RT_BindPushConstant(cmdBuf, 0, sizeof(uint32_t), &drawIndexPC);
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

    std::pair<Ref<TextureCube>, Ref<TextureCube>> VulkanRenderer::CreateEnvironmentMap(const std::string& filepath)
    {
        PR_PROFILE_FUNCTION();
        const uint32_t cubemapSize = 2048;
        const uint32_t irradianceMapSize = 32;

        if (!s_EnvironmentShader)
            s_EnvironmentShader = ComputeShader::Create("Assets/Shaders/Environment.ComputeShader");

        Ref<TextureCube> envUnfiltered = TextureCube::Create(ImageFormat::RGBA32F, cubemapSize, cubemapSize);
        Ref<Texture2D> envEquirect = Texture2D::Create(filepath);
        PR_CORE_ASSERT(envEquirect->GetFormat() == ImageFormat::RGBA32F, "Texture is not HDR!");

        int toCubeKernel = s_EnvironmentShader->FindKernel("CSEquirectToCube");
        s_EnvironmentShader->SetTexture(toCubeKernel, "u_EquirectangularTex", envEquirect);
        s_EnvironmentShader->SetImage(toCubeKernel, "o_OutputCube", envUnfiltered);
        s_EnvironmentShader->Dispatch(toCubeKernel, cubemapSize / 32, cubemapSize / 32, 6);
        Renderer::Submit([envUnfiltered]()
        {
            envUnfiltered.As<VulkanTextureCube>()->RT_GenerateMips();
        });

        Ref<TextureCube> envFiltered = TextureCube::Create(ImageFormat::RGBA32F, cubemapSize, cubemapSize);
        envUnfiltered.As<VulkanTextureCube>()->CopyTo(envFiltered);

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
        Renderer::Submit([irradianceMap]()
        {
            irradianceMap.As<VulkanTextureCube>()->RT_GenerateMips();
        });

        return { envFiltered, irradianceMap };
    }

    void VulkanRenderer::DispatchCompute(Ref<ComputeShader> computeShader, int32_t kernel,
        uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
    {
        Ref<Shader> kernelShader = computeShader->GetKernelShader(kernel);
        if (!kernelShader)
            return;
        std::vector<ComputeResourceBinding> captured = computeShader->GetResources();
        Ref<VulkanShader> vkShader = kernelShader.As<VulkanShader>();

        Renderer::Submit([=]() mutable {
            VulkanDescriptorSet tempSet;
            using DK = PrismShaderCompiler::DescriptorKind;
            for (const auto& binding : captured)
            {
                Ref<RefCounted> res = binding.res;
                uint32_t set = binding.Resource.Set;
                uint32_t bindingIndex = binding.Resource.Binding;
                switch (binding.Resource.Kind)
                {
                case PrismShaderCompiler::CSL::ResourceKind::StorageBuffer:
                {
                    tempSet.SetInput(bindingIndex, res.As<VulkanShaderStorageBuffer>());
                    break;
                }
                case PrismShaderCompiler::CSL::ResourceKind::UniformBuffer:
                {
                    tempSet.SetInput(bindingIndex, res.As<VulkanUniformBuffer>());
                    break;
                }
                case PrismShaderCompiler::CSL::ResourceKind::Sampler2D:
                {
                    Ref<VulkanImage2D> image = res ? res.As<VulkanTexture2D>()->GetImage().As<VulkanImage2D>() : nullptr;
                    tempSet.SetInput(bindingIndex, image);
                    break;
                }
                case PrismShaderCompiler::CSL::ResourceKind::SamplerCube:
                {
                    Ref<VulkanImageCube> image = res ? res.As<VulkanTextureCube>()->GetImage().As<VulkanImageCube>() : nullptr;
                    tempSet.SetInput(bindingIndex, image);
                    break;
                }
                case PrismShaderCompiler::CSL::ResourceKind::Image2D:
                {
                    Ref<VulkanImage2D> image = res ? res.As<VulkanTexture2D>()->GetImage().As<VulkanImage2D>() : nullptr;
                    tempSet.SetInput(bindingIndex, image, binding.Level);
                    break;
                }
                case PrismShaderCompiler::CSL::ResourceKind::ImageCube:
                {
                    Ref<VulkanImageCube> image = res ? res.As<VulkanTextureCube>()->GetImage().As<VulkanImageCube>() : nullptr;
                    tempSet.SetInput(bindingIndex, image, binding.Level);
                    break;
                }
                default:
                    PR_CORE_ASSERT(false, "VulkanRenderer::DispatchCompute: 未知的 ResourceKind!");
                    break;

                }
            }
            tempSet.Bake();
            tempSet.RT_Prepare();
            WeakRef<VulkanComputePipeline> pipeline = s_Data->PipelineCache.GetCompute(vkShader.Raw());
            VkDescriptorSet descriptorSets[] = { tempSet.RT_GetDescriptorSet() };
            pipeline->RT_Execute(descriptorSets, 1, numGroupsX, numGroupsY, numGroupsZ);
        });
    }



    WeakRef<VulkanImage2D> VulkanRenderer::RT_GetBlackImage2D()
    {
        return s_Data->BlackImage2D;
    }

    WeakRef<VulkanImageCube> VulkanRenderer::RT_GetBlackImageCube()
    {
        return s_Data->BlackImageCube;
    }

    WeakRef<VulkanUniformBuffer> VulkanRenderer::RT_GetEmptyUniformBuffer()
    {
        return s_Data->EmptyUniformBuffer;
    }


    WeakRef<VulkanShaderStorageBuffer> VulkanRenderer::RT_GetEmptyShaderStorageBuffer()
    {
        return s_Data->EmptyShaderStorageBuffer;
    }

}
