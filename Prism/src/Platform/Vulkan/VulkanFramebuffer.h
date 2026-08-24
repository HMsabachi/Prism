#pragma once

#include "Prism/Renderer/Buffer/Framebuffer.h"
#include "Platform/Vulkan/Vulkan.h"

#include "Prism/Utilities/StaticVector.h"

#include <vector>

namespace Prism
{
    class VulkanFramebuffer : public Framebuffer
    {
    static constexpr uint32_t VulkanMaxFramebufferAttachments = 8;
    public:
        VulkanFramebuffer(const FramebufferSpecification& spec);
        virtual ~VulkanFramebuffer();

        virtual void Resize(uint32_t width, uint32_t height, bool forceRecreate = false) override;
        virtual void RT_Resize(uint32_t width, uint32_t height, bool forceRecreate = false) override;
        virtual void AddResizeCallback(const std::function<void(Ref<Framebuffer>)>& func) override {}

        virtual uint32_t GetWidth() const override { return m_Width; }
        virtual uint32_t GetHeight() const override { return m_Height; }

        virtual const Ref<Image2D>& GetImage(uint32_t attachmentIndex = 0) const override;
        virtual const Ref<Image2D>& GetDepthImage() const override { return m_DepthAttachment; }

        virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

        size_t GetColorAttachmentCount() const { return m_Specification.SwapChainTarget ? 1 : m_ColorAttachments.size(); }
        uint32_t GetSamples() const;
        StaticVector<VkFormat, VulkanMaxFramebufferAttachments> GetAttachmentFormats() const;
        VkRenderPass GetRenderPass() const;
        VkFramebuffer GetVulkanFramebuffer() const;
        const StaticVector<VkClearValue, VulkanMaxFramebufferAttachments>& GetVulkanClearValues() const { return m_ClearValues; }
    private:
        void RT_Invalidate();
        void RT_Release();
    private:
        FramebufferSpecification m_Specification;
        uint32_t m_Width = 0, m_Height = 0;

        StaticVector<Ref<Image2D>, VulkanMaxFramebufferAttachments> m_ColorAttachments;
        Ref<Image2D> m_DepthAttachment;

        StaticVector<VkClearValue, VulkanMaxFramebufferAttachments> m_ClearValues;

        VkRenderPass m_RenderPass = VK_NULL_HANDLE;
        VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
    };
}
