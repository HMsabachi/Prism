#pragma once
#include "Prism/Renderer/Material.h"
#include "Platform/Vulkan/Vulkan.h"
#include "Platform/Vulkan/VulkanDescriptorSet.h"

namespace Prism
{
    class VulkanUniformBuffer;

    class VulkanMaterialBackend : public MaterialBackend
    {
    public:
        VulkanMaterialBackend(const WeakRef<Material> material);
        virtual ~VulkanMaterialBackend();
        virtual void OnAllocate() override;

        VkDescriptorSet RT_GetDescriptorSet() const;
    private:
        void SetTextureInputs() const;
    private:
        mutable VulkanDescriptorSet m_DescriptorSet;
        mutable Ref<VulkanUniformBuffer> m_UniformBuffer;
        mutable bool m_IsSetDirty[VulkanFramesInFlight];
        WeakRef<Material> m_Material;
    };
}
