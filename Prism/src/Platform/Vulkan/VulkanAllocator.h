#pragma once

#include "Platform/Vulkan/Vulkan.h"
#include "Prism/Core/Ref.h"
#include "VulkanMemoryAllocator/vk_mem_alloc.h"

namespace Prism
{
    class VulkanDevice;

    class VulkanAllocator
    {
    public:
        VulkanAllocator() = delete;

        static void Init(const Ref<VulkanDevice>& device);
        static void Shutdown();

        static VmaAllocator Get() { return s_Allocator; }

        static VmaAllocation AllocateBuffer(const VkBufferCreateInfo& bufferCreateInfo, VmaMemoryUsage usage, VkBuffer& outBuffer);
        static VmaAllocation AllocateImage(const VkImageCreateInfo& imageCreateInfo, VmaMemoryUsage usage, VkImage& outImage);

        static void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);
        static void DestroyImage(VkImage image, VmaAllocation allocation);

        template<typename T>
        static T* MapMemory(VmaAllocation allocation)
        {
            T* mappedMemory;
            vmaMapMemory(s_Allocator, allocation, (void**)&mappedMemory);
            return mappedMemory;
        }

        static void UnmapMemory(VmaAllocation allocation);

    private:
        inline static VmaAllocator s_Allocator = VK_NULL_HANDLE;
    };
}
