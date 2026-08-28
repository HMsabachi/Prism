#pragma once

#include "Platform/Vulkan/Vulkan.h"


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
        ImageCube,
        StorageImage2D,
        StorageImageCube
    };

    class VulkanGlobalDescriptorPool
    {
    public:
        VulkanGlobalDescriptorPool() = delete;

        static VkDescriptorPool Allocate(VkDescriptorSetLayout layout, uint32_t setCount, VkDescriptorSet* outSets);
        static void Free(VkDescriptorPool pool, uint32_t setCount, const VkDescriptorSet* sets);
        static void Shutdown();
    };

    class VulkanDescriptorSet
    {
    public:
        struct Binding
        {
            RenderResourceType Type = RenderResourceType::None;
            Ref<RefCounted> Resource;
            std::array<void*, VulkanFramesInFlight> NativeHandle{ nullptr };
            uint32_t Level = 0;
        };

    public:
        VulkanDescriptorSet() = default;
        ~VulkanDescriptorSet();
        VulkanDescriptorSet(const VulkanDescriptorSet&) = delete;
        VulkanDescriptorSet& operator=(const VulkanDescriptorSet&) = delete;

        void SetInput(uint32_t binding, Ref<VulkanUniformBuffer> buffer);
        void SetInput(uint32_t binding, Ref<VulkanShaderStorageBuffer> buffer);
        void SetInput(uint32_t binding, Ref<VulkanImage2D> image);
        void SetInput(uint32_t binding, Ref<VulkanImageCube> image);
        void SetInput(uint32_t binding, Ref<VulkanImage2D> image, uint32_t level);
        void SetInput(uint32_t binding, Ref<VulkanImageCube> image, uint32_t level);

        void Reset();
        void Bake();
        void RT_Prepare();

        bool IsBaked() const { return m_IsBaked; }
        VkDescriptorSet RT_GetDescriptorSet() const { PR_CORE_ASSERT(m_IsBaked); return m_DescriptorSets[CurrentSlotIndex()]; };
    private:
        uint32_t CurrentSlotIndex() const;
    private:
        std::map<uint32_t, Binding> m_Bindings;
        std::array<VkDescriptorSet, VulkanFramesInFlight> m_DescriptorSets{ VK_NULL_HANDLE };
        VkDescriptorPool m_SourcePool = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
        bool m_IsBaked = false;
    };
}
