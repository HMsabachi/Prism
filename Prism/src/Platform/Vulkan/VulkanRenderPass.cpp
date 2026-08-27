#include "prpch.h"
#include "VulkanRenderPass.h"

#include "VulkanImage.h"
#include "VulkanUniformBuffer.h"
#include "VulkanShaderStorageBuffer.h"
#include "VulkanFramebuffer.h"

#include "Prism/Renderer/Renderer.h"

namespace Prism
{
    VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& spec)
        : m_Specification(spec)
    {
    }

    VulkanRenderPass::~VulkanRenderPass()
    {
        // m_DescriptorSet 成员析构 -> Reset() 延迟归还 sets + 销毁 layout
    }

    void VulkanRenderPass::SetInput(uint32_t binding, const Ref<Image2D>& image)
    {
        Ref<VulkanRenderPass> instance = this;
        Ref<VulkanImage2D> vulkanImage = image.As<VulkanImage2D>();
        Renderer::Submit([instance, binding, vulkanImage]() mutable {
            instance->m_DescriptorSet.SetInput(binding, vulkanImage);
        });
    }

    void VulkanRenderPass::SetInput(uint32_t binding, const Ref<ImageCube>& texture)
    {
        Ref<VulkanRenderPass> instance = this;
        Ref<VulkanImageCube> vulkanImage = texture.As<VulkanImageCube>();
        Renderer::Submit([instance, binding, vulkanImage]() mutable {
            instance->m_DescriptorSet.SetInput(binding, vulkanImage);
        });
    }

    void VulkanRenderPass::SetInput(uint32_t binding, const Ref<UniformBuffer>& ubo)
    {
        Ref<VulkanRenderPass> instance = this;
        Ref<VulkanUniformBuffer> vulkanBuffer = ubo.As<VulkanUniformBuffer>();
        Renderer::Submit([instance, binding, vulkanBuffer]() mutable {
            instance->m_DescriptorSet.SetInput(binding, vulkanBuffer);
        });
    }

    void VulkanRenderPass::SetInput(uint32_t binding, const Ref<ShaderStorageBuffer>& ssbo)
    {
        Ref<VulkanRenderPass> instance = this;
        Ref<VulkanShaderStorageBuffer> vulkanBuffer = ssbo.As<VulkanShaderStorageBuffer>();
        Renderer::Submit([instance, binding, vulkanBuffer]() mutable {
            instance->m_DescriptorSet.SetInput(binding, vulkanBuffer);
        });
    }

    Ref<Image2D> VulkanRenderPass::GetOutput(uint32_t index) const
    {
        return m_Specification.TargetFramebuffer->GetImage(index);
    }

    Ref<Image2D> VulkanRenderPass::GetDepthOutput() const
    {
        return m_Specification.TargetFramebuffer->GetDepthImage();
    }

    Ref<Framebuffer> VulkanRenderPass::GetTargetFramebuffer() const
    {
        return m_Specification.TargetFramebuffer;
    }

    void VulkanRenderPass::Bake()
    {
        Ref<VulkanRenderPass> instance = this;
        Renderer::Submit([instance]() mutable {
            instance->m_DescriptorSet.Bake();
        });
    }
}
