#include "prpch.h"
#include "VulkanFramebuffer.h"

#include "VulkanContext.h"
#include "VulkanImage.h"

#include "Prism/Core/RenderThread.h"
#include "Prism/Renderer/Renderer.h"

#include <vector>

namespace Prism
{
    namespace
    {
        bool IsDepthFormat(ImageFormat format)
        {
            return format == ImageFormat::DEPTH32F || format == ImageFormat::DEPTH24STENCIL8;
        }
    }

    VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& spec)
        : m_Specification(spec), m_Width(spec.Width), m_Height(spec.Height)
    {
        if (!m_Specification.SwapChainTarget)
        {
            for (auto& format : m_Specification.Attachments.Attachments)
            {
                Ref<Image2D> image = Image2D::Create(format.Format, m_Width, m_Height, nullptr, m_Specification.Samples);
                if (!IsDepthFormat(format.Format))
                {
                    // GL 语义下任意纹理可挂 FBO，Vulkan 需显式声明渲染目标用途
                    image.As<VulkanImage2D>()->SetExtraUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
                    m_ColorAttachments.emplace_back(image);
                }
                else
                {
                    m_DepthAttachment = image;
                }
            }
        }
        else
        {
            m_ClearValues.resize(1);
            const auto& clearColor = m_Specification.ClearColor;
            m_ClearValues[0].color = { { clearColor.r, clearColor.g, clearColor.b, clearColor.a } };
        }

        Resize(m_Specification.Width, m_Specification.Height, true);
        CalculateHash();
    }

    VulkanFramebuffer::~VulkanFramebuffer()
    {
        RT_Release();
    }

    void VulkanFramebuffer::Resize(uint32_t width, uint32_t height, bool forceRecreate)
    {
        if (!forceRecreate && (m_Width == width && m_Height == height))
            return;

        if (RenderThread::IsCurrentThreadRT())
        {
            RT_Resize(width, height, forceRecreate);
        }
        else
        {
            Ref<VulkanFramebuffer> instance = this;
            Renderer::Submit([instance, width, height, forceRecreate]() mutable
            {
                instance->RT_Resize(width, height, forceRecreate);
            });
        }
    }

    void VulkanFramebuffer::RT_Resize(uint32_t width, uint32_t height, bool forceRecreate)
    {
        if (!forceRecreate && (m_Width == width && m_Height == height))
            return;

        m_Width = width;
        m_Height = height;
        if (m_Specification.SwapChainTarget)
        {
            // renderpass/framebuffer 由交换链持有，GetRenderPass/GetVulkanFramebuffer 动态取用
            CalculateHash();
            return;
        }
        RT_Invalidate();
    }

    const Ref<Image2D>& VulkanFramebuffer::GetImage(uint32_t attachmentIndex) const
    {
        PR_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());
        return m_ColorAttachments[attachmentIndex];
    }

    VkRenderPass VulkanFramebuffer::GetRenderPass() const
    {
        if (m_Specification.SwapChainTarget)
            return VulkanContext::Get()->GetSwapChain().GetRenderPass();
        return m_RenderPass;
    }

    VkFramebuffer VulkanFramebuffer::GetVulkanFramebuffer() const
    {
        if (m_Specification.SwapChainTarget)
            return VulkanContext::Get()->GetSwapChain().GetCurrentFramebuffer();
        return m_Framebuffer;
    }

    uint32_t VulkanFramebuffer::GetSamples() const
    {
        if (m_Specification.SwapChainTarget || m_ColorAttachments.empty())
            return m_DepthAttachment ? m_DepthAttachment->GetSamples() : 1;
        return m_ColorAttachments[0]->GetSamples();
    }

    StaticVector<VkFormat, VulkanFramebuffer::VulkanMaxFramebufferAttachments> VulkanFramebuffer::GetAttachmentFormats() const
    {
        StaticVector<VkFormat, VulkanMaxFramebufferAttachments> formats;
        if (m_Specification.SwapChainTarget)
        {
            formats.push_back(VulkanContext::Get()->GetSwapChain().GetColorFormat());
            return formats;
        }

        for (auto& image : m_ColorAttachments)
            formats.push_back(Utils::VulkanImageFormat(image->GetFormat()));
        if (m_DepthAttachment)
            formats.push_back(Utils::VulkanImageFormat(m_DepthAttachment->GetFormat()));
        return formats;
    }

    void VulkanFramebuffer::CalculateHash()
    {
        constexpr uint64_t FNV_PRIME = 1099511628211ULL;
        constexpr uint64_t OFFSET_BASIS = 14695981039346656037ULL;
        m_Hash = OFFSET_BASIS;
        auto mix = [this](uint64_t value)
        {
            m_Hash ^= value;
            m_Hash *= FNV_PRIME;
        };
        mix(GetColorAttachmentCount());
        mix(GetSamples());
        for (VkFormat format : GetAttachmentFormats())
            mix((uint64_t)format);
    }

    void VulkanFramebuffer::RT_Invalidate()
    {
        VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

        RT_Release();

        StaticVector<VkAttachmentDescription, 10> attachmentDescriptions;
        StaticVector<VkAttachmentReference, 10> colorAttachmentReferences;
        VkAttachmentReference depthAttachmentReference{};

        m_ClearValues.resize(m_ColorAttachments.size() + (m_DepthAttachment ? 1 : 0));

        for (size_t i = 0; i < m_ColorAttachments.size(); i++)
        {
            Ref<VulkanImage2D> image = m_ColorAttachments[i].As<VulkanImage2D>();
            image->RT_Resize(m_Width, m_Height);

            VkAttachmentDescription& attachmentDescription = attachmentDescriptions.emplace_back();
            attachmentDescription.format = Utils::VulkanImageFormat(image->GetFormat());
            attachmentDescription.samples = (VkSampleCountFlagBits)image->GetSamples();
            attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            const auto& clearColor = m_Specification.ClearColor;
            m_ClearValues[i].color = { { clearColor.r, clearColor.g, clearColor.b, clearColor.a } };
            colorAttachmentReferences.emplace_back(VkAttachmentReference{ (uint32_t)i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
        }

        if (m_DepthAttachment)
        {
            uint32_t depthAttachmentIndex = (uint32_t)m_ColorAttachments.size();
            Ref<VulkanImage2D> image = m_DepthAttachment.As<VulkanImage2D>();
            image->RT_Resize(m_Width, m_Height);

            VkAttachmentDescription& attachmentDescription = attachmentDescriptions.emplace_back();
            attachmentDescription.format = Utils::VulkanImageFormat(image->GetFormat());
            attachmentDescription.samples = (VkSampleCountFlagBits)image->GetSamples();
            attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

            depthAttachmentReference = { depthAttachmentIndex, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
            m_ClearValues[depthAttachmentIndex].depthStencil = { 1.0f, 0 };
        }

        VkSubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentReferences.size());
        subpassDescription.pColorAttachments = colorAttachmentReferences.data();
        if (m_DepthAttachment)
            subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;

        // 子渲染通道依赖：颜色/深度 attachment 与下一 pass 的采样读之间的布局和内存依赖
        StaticVector<VkSubpassDependency, 10> dependencies;
        if (!m_ColorAttachments.empty())
        {
            VkSubpassDependency& inputDependency = dependencies.emplace_back();
            inputDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            inputDependency.dstSubpass = 0;
            inputDependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            inputDependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            inputDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            inputDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            inputDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            VkSubpassDependency& outputDependency = dependencies.emplace_back();
            outputDependency.srcSubpass = 0;
            outputDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
            outputDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            outputDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            outputDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            outputDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            outputDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }
        if (m_DepthAttachment)
        {
            VkSubpassDependency& inputDependency = dependencies.emplace_back();
            inputDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            inputDependency.dstSubpass = 0;
            inputDependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            inputDependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            inputDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            inputDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            inputDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            VkSubpassDependency& outputDependency = dependencies.emplace_back();
            outputDependency.srcSubpass = 0;
            outputDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
            outputDependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            outputDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            outputDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            outputDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            outputDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
        renderPassInfo.pAttachments = attachmentDescriptions.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescription;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();
        VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass));

        std::vector<VkImageView> attachments;
        attachments.reserve(m_ColorAttachments.size() + (m_DepthAttachment ? 1 : 0));
        for (auto& image : m_ColorAttachments)
            attachments.push_back(image.As<VulkanImage2D>()->GetImageInfo().ImageView);
        if (m_DepthAttachment)
            attachments.push_back(m_DepthAttachment.As<VulkanImage2D>()->GetImageInfo().ImageView);

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_RenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_Width;
        framebufferInfo.height = m_Height;
        framebufferInfo.layers = 1;
        VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_Framebuffer));
    }

    void VulkanFramebuffer::RT_Release()
    {
        if (!m_Framebuffer)
            return;

        VkRenderPass renderPass = m_RenderPass;
        VkFramebuffer framebuffer = m_Framebuffer;
        Renderer::SubmitResourceFree([renderPass, framebuffer]()
        {
            auto device = VulkanContext::GetCurrentDevice();
            PR_CORE_ASSERT(device, "VulkanFramebuffer::RT_Release: VulkanContext::GetCurrentDevice() returned nullptr");
            vkDestroyRenderPass(device->GetVulkanDevice(), renderPass, nullptr);
            vkDestroyFramebuffer(device->GetVulkanDevice(), framebuffer, nullptr);
        });

        m_RenderPass = VK_NULL_HANDLE;
        m_Framebuffer = VK_NULL_HANDLE;
    }
}
