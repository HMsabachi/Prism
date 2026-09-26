#pragma once

#include "Platform/Vulkan/Vulkan.h"

namespace Prism
{
    namespace Utils
    {
        void InsertImageMemoryBarrier(
            VkCommandBuffer cmdBuf,
            VkImage image,
            VkAccessFlags srcAccessMask,
            VkAccessFlags dstAccessMask,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask,
            VkImageSubresourceRange subresourceRange);

        void InsertBufferMemoryBarrier(
            VkCommandBuffer cmdBuf,
            VkBuffer buffer,
            VkDeviceSize offset,
            VkDeviceSize size,
            VkAccessFlags srcAccessMask,
            VkAccessFlags dstAccessMask,
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask);
    }
}
