#pragma once

#include "Prism/Renderer/RenderPass.h"
#include "Platform/Vulkan/VulkanDescriptorSet.h"

namespace Prism
{
    class VulkanRenderPass : public RenderPass
    {
    public:
        VulkanRenderPass(const RenderPassSpecification& spec);
        virtual ~VulkanRenderPass();

        virtual RenderPassSpecification& GetSpecification() override { return m_Specification; }
        virtual const RenderPassSpecification& GetSpecification() const override { return m_Specification; }

        virtual void SetInput(uint32_t binding, const Ref<Image2D>& image) override;
        virtual void SetInput(uint32_t binding, const Ref<ImageCube>& texture) override;
        virtual void SetInput(uint32_t binding, const Ref<UniformBuffer>& ubo) override;
        virtual void SetInput(uint32_t binding, const Ref<ShaderStorageBuffer>& ssbo) override;

        virtual Ref<Image2D> GetOutput(uint32_t index = 0) const override;
        virtual Ref<Image2D> GetDepthOutput() const override;
        virtual Ref<Framebuffer> GetTargetFramebuffer() const override;

        virtual void Bake() override;

        void RT_Prepare() { m_DescriptorSet.RT_Prepare(); }
        bool IsBaked() const { return m_DescriptorSet.IsBaked(); }
        VkDescriptorSet RT_GetDescriptorSet() const { return m_DescriptorSet.RT_GetDescriptorSet(); }
    private:
        RenderPassSpecification m_Specification;
        VulkanDescriptorSet m_DescriptorSet;
    };
}
