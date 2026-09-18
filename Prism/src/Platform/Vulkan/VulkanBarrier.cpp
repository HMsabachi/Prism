#include "prpch.h"
#include "VulkanBarrier.h"

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
            VkImageSubresourceRange subresourceRange)
        {
            VkImageMemoryBarrier imageMemoryBarrier{};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.srcAccessMask = srcAccessMask;
            imageMemoryBarrier.dstAccessMask = dstAccessMask;
            imageMemoryBarrier.oldLayout = oldImageLayout;
            imageMemoryBarrier.newLayout = newImageLayout;
            imageMemoryBarrier.image = image;
            imageMemoryBarrier.subresourceRange = subresourceRange;

            vkCmdPipelineBarrier(
                cmdBuf,
                srcStageMask,
                dstStageMask,
                0,
                0, nullptr,
                0, nullptr,
                1, &imageMemoryBarrier);
        }

        void InsertBufferMemoryBarrier(
            VkCommandBuffer cmdBuf,
            VkBuffer buffer,
            VkDeviceSize offset,
            VkDeviceSize size,
            VkAccessFlags srcAccessMask,
            VkAccessFlags dstAccessMask,
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask)
        {
            VkBufferMemoryBarrier bufferMemoryBarrier{};
            bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferMemoryBarrier.srcAccessMask = srcAccessMask;
            bufferMemoryBarrier.dstAccessMask = dstAccessMask;
            bufferMemoryBarrier.buffer = buffer;
            bufferMemoryBarrier.offset = offset;
            bufferMemoryBarrier.size = size;

            vkCmdPipelineBarrier(
                cmdBuf,
                srcStageMask,
                dstStageMask,
                0,
                0, nullptr,
                1, &bufferMemoryBarrier,
                0, nullptr);
        }
    }
}
