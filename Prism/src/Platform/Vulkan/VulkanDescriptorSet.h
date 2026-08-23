#pragma once

#include "Platform/Vulkan/Vulkan.h"

#include "Prism/Utilities/StaticVector.h"


namespace Prism
{
    class VulkanUniformBuffer;
    class VulkanShaderStorageBuffer;
    class VulkanImage2D;
    class VulkanImageCube;

    enum class RenderResourceType : uint16_t
    {
        None = 0,
        UniformBuffer,
        StorageBuffer,
        Image2D,
        ImageCube
    };

    class VulkanDescriptorSet
    {
    public:
        struct Binding
        {
            RenderResourceType Type = RenderResourceType::None;
            Ref<RefCounted> Resource;
            void* NativeHandle = nullptr; // Vulkan native handle (VkBuffer, VkImageView, etc.)
        };

    public:
        VulkanDescriptorSet() = default;
        ~VulkanDescriptorSet();

        void SetInput(uint32_t binding, Ref<VulkanUniformBuffer> buffer);
        void SetInput(uint32_t binding, Ref<VulkanShaderStorageBuffer> buffer);
        void SetInput(uint32_t binding, Ref<VulkanImage2D> image);
        void SetInput(uint32_t binding, Ref<VulkanImageCube> image);

        void Bake();
        void RT_Prepare();

        VkDescriptorSet GetDescriptorSet() const;

    private:
        uint32_t CurrentSlotIndex() const;
    private:
        std::map<uint32_t, Binding> m_Bindings;
        std::array<VkDescriptorSet, VulkanFramesInFlight> m_DescriptorSets{ VK_NULL_HANDLE };
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
        bool m_Dirty = true;
    };
}
