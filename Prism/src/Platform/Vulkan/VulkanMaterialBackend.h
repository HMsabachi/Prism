#pragma once
#include "Prism/Renderer/Material.h"
#include "Platform/Vulkan/Vulkan.h"

namespace Prism
{
    class VulkanUniformBuffer;
    class Texture;

    class VulkanMaterialBackend : public MaterialBackend
    {
    public:
        VulkanMaterialBackend(const WeakRef<Material> material);
        virtual ~VulkanMaterialBackend();
        virtual void OnAllocate() override;

        const Ref<VulkanUniformBuffer>& RT_GetUniformBuffer() const { return m_UniformBuffer; }
        VkDescriptorSet RT_GetDescriptorSet() const;
    private:
        mutable Ref<VulkanUniformBuffer> m_UniformBuffer;
        mutable VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
        mutable VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
        mutable VkDescriptorSet m_DescriptorSets[VulkanFramesInFlight] = {};
        WeakRef<Material> m_Material;
    };
}
