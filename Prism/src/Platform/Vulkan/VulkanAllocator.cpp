#include "prpch.h"
#include "Platform/Vulkan/VulkanAllocator.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"

namespace Prism
{
    void VulkanAllocator::Init(const Ref<VulkanDevice>& device)
    {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
        allocatorInfo.physicalDevice = device->GetPhysicalDevice()->GetVulkanPhysicalDevice();
        allocatorInfo.device = device->GetVulkanDevice();
        allocatorInfo.instance = VulkanContext::GetInstance();

        VK_CHECK_RESULT(vmaCreateAllocator(&allocatorInfo, &s_Allocator));
    }

    void VulkanAllocator::Shutdown()
    {
        if (s_Allocator != VK_NULL_HANDLE)
        {
            vmaDestroyAllocator(s_Allocator);
            s_Allocator = VK_NULL_HANDLE;
        }
    }

    VmaAllocation VulkanAllocator::AllocateBuffer(const VkBufferCreateInfo& bufferCreateInfo, VmaMemoryUsage usage, VkBuffer& outBuffer)
    {
        PR_CORE_ASSERT(bufferCreateInfo.size > 0);

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = usage;

        VmaAllocation allocation;
        VK_CHECK_RESULT(vmaCreateBuffer(s_Allocator, &bufferCreateInfo, &allocCreateInfo, &outBuffer, &allocation, nullptr));
        if (allocation == nullptr)
            PR_CORE_ERROR("GPU buffer 分配失败 VulkanAllocator::AllocateBuffer size = {0}", bufferCreateInfo.size);

        return allocation;
    }

    VmaAllocation VulkanAllocator::AllocateImage(const VkImageCreateInfo& imageCreateInfo, VmaMemoryUsage usage, VkImage& outImage)
    {
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = usage;

        VmaAllocation allocation;
        VK_CHECK_RESULT(vmaCreateImage(s_Allocator, &imageCreateInfo, &allocCreateInfo, &outImage, &allocation, nullptr));
        if (allocation == nullptr)
            PR_CORE_ERROR("GPU image 分配失败 VulkanAllocator::AllocateImage extent = {0}x{1}x{2}", imageCreateInfo.extent.width, imageCreateInfo.extent.height, imageCreateInfo.extent.depth);

        return allocation;
    }

    void VulkanAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
    {
        PR_CORE_ASSERT(buffer);
        PR_CORE_ASSERT(allocation);
        vmaDestroyBuffer(s_Allocator, buffer, allocation);
    }

    void VulkanAllocator::DestroyImage(VkImage image, VmaAllocation allocation)
    {
        PR_CORE_ASSERT(image);
        PR_CORE_ASSERT(allocation);
        vmaDestroyImage(s_Allocator, image, allocation);
    }

    void VulkanAllocator::UnmapMemory(VmaAllocation allocation)
    {
        vmaUnmapMemory(s_Allocator, allocation);
    }
}
