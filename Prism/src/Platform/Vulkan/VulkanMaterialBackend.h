#pragma once
#include "Prism/Renderer/Material.h"

namespace Prism
{
    class VulkanMaterialBackend : public MaterialBackend
    {
    public:
        VulkanMaterialBackend(const WeakRef<Material> material);
        virtual ~VulkanMaterialBackend();
        virtual void OnAllocate() override;

        // 这里可以Get Material DescriptorSet 如果Material 是 Dirty 则重新分配 DescriptorSet 并更新 UniformBuffer 和 Texture Bindings
    private:
        WeakRef<Material> m_Material;
    };
}
