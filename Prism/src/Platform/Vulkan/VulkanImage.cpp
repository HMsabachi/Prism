#include "prpch.h"
#include "Platform/Vulkan/VulkanImage.h"

#include "Platform/Vulkan/VulkanAllocator.h"
#include "Platform/Vulkan/VulkanBarrier.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Prism/Renderer/Renderer.h"

namespace Prism
{
    namespace Utils
    {
        static bool IsDepthFormat(ImageFormat format)
        {
            return format == ImageFormat::DEPTH16 || format == ImageFormat::DEPTH32F ||
                   format == ImageFormat::DEPTH24STENCIL8 || format == ImageFormat::DEPTH32FSTENCIL8;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////
    // Image2D
    //////////////////////////////////////////////////////////////////////////////////

    VulkanImage2D::VulkanImage2D(const ImageSpecification& specification, Buffer buffer)
        : m_Specification(specification), m_ImageData(std::move(buffer))
    {
    }

    VulkanImage2D::VulkanImage2D(const ImageSpecification& specification, std::vector<Buffer>&& mips)
        : m_Specification(specification), m_Mips(std::move(mips))
    {
    }

    VulkanImage2D::~VulkanImage2D()
    {
        Release();
    }

    void VulkanImage2D::Resize(const uint32_t width, const uint32_t height)
    {
        Ref<VulkanImage2D> instance = this;
        Renderer::Submit([instance, width, height]() mutable
        {
            instance->RT_Resize(width, height);
        });
    }

    void VulkanImage2D::RT_Resize(const uint32_t width, const uint32_t height)
    {
        m_Specification.Width = width;
        m_Specification.Height = height;
        Invalidate();
    }

    void VulkanImage2D::Invalidate()
    {
        RT_Invalidate();
    }

    void VulkanImage2D::RT_Invalidate()
    {
        auto device = VulkanContext::GetCurrentDevice();
        auto vulkanDevice = device->GetVulkanDevice();

        if (m_Info.Image || m_Info.ImageView || m_Info.Sampler)
            Release();

        VkFormat format = Utils::VulkanImageFormat(m_Specification.Format);

        VkImageAspectFlags aspectMask = Utils::IsDepthFormat(m_Specification.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        if (m_Specification.Format == ImageFormat::DEPTH24STENCIL8 || m_Specification.Format == ImageFormat::DEPTH32FSTENCIL8)
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

        uint32_t mipCount = !m_Mips.empty() ? (uint32_t)m_Mips.size()
            : m_ImageData ? Utils::CalculateMipCount(m_Specification.Width, m_Specification.Height) : 1;

        VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (Utils::IsDepthFormat(m_Specification.Format))
            usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        else if (m_Specification.Usage == ImageUsage::Attachment)
            usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (m_Specification.Usage == ImageUsage::Storage)
            usage |= VK_IMAGE_USAGE_STORAGE_BIT;
        if (m_Specification.Samples > 1)
        {
            mipCount = 1;
            usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }

        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = format;
        imageCreateInfo.mipLevels = mipCount;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = (VkSampleCountFlagBits)m_Specification.Samples;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.extent = { m_Specification.Width, m_Specification.Height, 1 };
        imageCreateInfo.usage = usage;
        m_Info.MemoryAlloc = VulkanAllocator::AllocateImage(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_Info.Image);

        VkImageViewCreateInfo view{};
        view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view.format = format;
        view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        view.subresourceRange.aspectMask = aspectMask;
        view.subresourceRange.baseMipLevel = 0;
        view.subresourceRange.baseArrayLayer = 0;
        view.subresourceRange.layerCount = 1;
        view.subresourceRange.levelCount = mipCount;
        view.image = m_Info.Image;
        VK_CHECK_RESULT(vkCreateImageView(vulkanDevice, &view, nullptr, &m_Info.ImageView));

        if (m_Specification.Samples > 1)
        {
            UpdateDescriptor();
            return;
        }

        VkSamplerCreateInfo sampler{};
        sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler.magFilter = VK_FILTER_LINEAR;
        sampler.minFilter = VK_FILTER_LINEAR;
        sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        VkSamplerAddressMode wrapMode = m_Wrap == TextureWrap::Clamp ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler.addressModeU = wrapMode;
        sampler.addressModeV = wrapMode;
        sampler.addressModeW = wrapMode;
        sampler.mipLodBias = 0.0f;
        sampler.compareOp = VK_COMPARE_OP_NEVER;
        sampler.minLod = 0.0f;
        sampler.maxLod = (float)mipCount;
        sampler.maxAnisotropy = 1.0f;
        sampler.anisotropyEnable = VK_FALSE;
        sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VK_CHECK_RESULT(vkCreateSampler(vulkanDevice, &sampler, nullptr, &m_Info.Sampler));

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = aspectMask;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.layerCount = 1;

        if (!m_Mips.empty())
        {
            uint64_t totalSize = 0;
            for (auto& mip : m_Mips)
                totalSize += mip.Size;

            VkBuffer stagingBuffer;
            VkBufferCreateInfo stagingCreateInfo{};
            stagingCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            stagingCreateInfo.size = totalSize;
            stagingCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            stagingCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            VmaAllocation stagingAllocation = VulkanAllocator::AllocateBuffer(stagingCreateInfo, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);

            uint8_t* stagingData = VulkanAllocator::MapMemory<uint8_t>(stagingAllocation);
            uint64_t dstOffset = 0;
            for (auto& mip : m_Mips)
            {
                memcpy(stagingData + dstOffset, mip.Data, mip.Size);
                dstOffset += mip.Size;
            }
            VulkanAllocator::UnmapMemory(stagingAllocation);

            VkCommandBuffer copyCmd = device->GetCommandBuffer(true);

            VkImageSubresourceRange allMips = {};
            allMips.aspectMask = aspectMask;
            allMips.baseMipLevel = 0;
            allMips.levelCount = mipCount;
            allMips.layerCount = 1;

            Utils::InsertImageMemoryBarrier(copyCmd, m_Info.Image,
                0, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                allMips);

            bool compressed = Utils::IsCompressedFormat(m_Specification.Format);
            std::vector<VkBufferImageCopy> regions(mipCount);
            uint32_t bufferOffset = 0;
            for (uint32_t i = 0; i < mipCount; i++)
            {
                uint32_t w = std::max(1u, m_Specification.Width >> i);
                uint32_t h = std::max(1u, m_Specification.Height >> i);
                VkBufferImageCopy& region = regions[i];
                region.imageSubresource.aspectMask = aspectMask;
                region.imageSubresource.mipLevel = i;
                region.imageSubresource.baseArrayLayer = 0;
                region.imageSubresource.layerCount = 1;
                region.imageExtent.width = compressed ? ((w + 3) & ~3u) : w;
                region.imageExtent.height = compressed ? ((h + 3) & ~3u) : h;
                region.imageExtent.depth = 1;
                region.bufferOffset = bufferOffset;
                bufferOffset += (uint32_t)m_Mips[i].Size;
            }

            vkCmdCopyBufferToImage(
                copyCmd,
                stagingBuffer,
                m_Info.Image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                mipCount,
                regions.data());

            InsertFinalLayoutBarrier(copyCmd, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, allMips);

            device->FlushCommandBuffer(copyCmd);

            VulkanAllocator::DestroyBuffer(stagingBuffer, stagingAllocation);
        }
        else if (m_ImageData)
        {
            VkBuffer stagingBuffer;
            VkBufferCreateInfo stagingCreateInfo{};
            stagingCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            stagingCreateInfo.size = m_ImageData.Size;
            stagingCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            stagingCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            VmaAllocation stagingAllocation = VulkanAllocator::AllocateBuffer(stagingCreateInfo, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);

            uint8_t* stagingData = VulkanAllocator::MapMemory<uint8_t>(stagingAllocation);
            memcpy(stagingData, m_ImageData.Data, m_ImageData.Size);
            VulkanAllocator::UnmapMemory(stagingAllocation);

            VkCommandBuffer copyCmd = device->GetCommandBuffer(true);

            Utils::InsertImageMemoryBarrier(copyCmd, m_Info.Image,
                0, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                subresourceRange);

            VkBufferImageCopy bufferCopyRegion = {};
            bufferCopyRegion.imageSubresource.aspectMask = aspectMask;
            bufferCopyRegion.imageSubresource.mipLevel = 0;
            bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
            bufferCopyRegion.imageSubresource.layerCount = 1;
            bufferCopyRegion.imageExtent.width = m_Specification.Width;
            bufferCopyRegion.imageExtent.height = m_Specification.Height;
            bufferCopyRegion.imageExtent.depth = 1;
            bufferCopyRegion.bufferOffset = 0;

            vkCmdCopyBufferToImage(
                copyCmd,
                stagingBuffer,
                m_Info.Image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &bufferCopyRegion);

            Utils::InsertImageMemoryBarrier(copyCmd, m_Info.Image,
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                subresourceRange);

            device->FlushCommandBuffer(copyCmd);

            VulkanAllocator::DestroyBuffer(stagingBuffer, stagingAllocation);

            RT_GenerateMips();
        }
        else
        {
            VkCommandBuffer layoutCmd = device->GetCommandBuffer(true);

            InsertFinalLayoutBarrier(layoutCmd, 0, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, subresourceRange);

            device->FlushCommandBuffer(layoutCmd);
        }

        UpdateDescriptor();
    }

    void VulkanImage2D::InsertFinalLayoutBarrier(VkCommandBuffer cmdBuf, VkAccessFlags srcAccessMask,
        VkImageLayout oldImageLayout, VkPipelineStageFlags srcStageMask, const VkImageSubresourceRange& subresourceRange) const
    {
        VkImageLayout newImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkAccessFlags dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        if (m_Specification.Usage == ImageUsage::Storage)
        {
            newImageLayout = VK_IMAGE_LAYOUT_GENERAL;
            dstAccessMask |= VK_ACCESS_SHADER_WRITE_BIT;
            dstStageMask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        }
        else if (Utils::IsDepthFormat(m_Specification.Format))
        {
            newImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        }

        Utils::InsertImageMemoryBarrier(cmdBuf, m_Info.Image, srcAccessMask, dstAccessMask,
            oldImageLayout, newImageLayout, srcStageMask, dstStageMask, subresourceRange);
    }

    void VulkanImage2D::Release()
    {
        if (!m_Info.Image && !m_Info.ImageView && !m_Info.Sampler)
            return;

        VkImage image = m_Info.Image;
        VkImageView imageView = m_Info.ImageView;
        VkSampler sampler = m_Info.Sampler;
        VmaAllocation allocation = m_Info.MemoryAlloc;
        auto storageViews = std::move(m_StorageViews);
        m_StorageViews.clear();
        m_Info = {};

        Renderer::SubmitResourceFree([image, imageView, sampler, allocation, storageViews]()
        {
            VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
            if (sampler)
                vkDestroySampler(device, sampler, nullptr);
            if (imageView)
                vkDestroyImageView(device, imageView, nullptr);
            for (auto& [mip, view] : storageViews)
                vkDestroyImageView(device, view, nullptr);
            if (image)
                VulkanAllocator::DestroyImage(image, allocation);
        });

        m_DescriptorImageInfo = {};
    }

    void VulkanImage2D::RT_GenerateMips()
    {
        auto device = VulkanContext::GetCurrentDevice();

        VkCommandBuffer blitCmd = device->GetCommandBuffer(true);

        uint32_t mipLevels = Utils::CalculateMipCount(m_Specification.Width, m_Specification.Height);
        for (uint32_t i = 1; i < mipLevels; i++)
        {
            VkImageBlit imageBlit{};

            imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.srcSubresource.layerCount = 1;
            imageBlit.srcSubresource.mipLevel = i - 1;
            imageBlit.srcOffsets[1].x = int32_t(m_Specification.Width >> (i - 1));
            imageBlit.srcOffsets[1].y = int32_t(m_Specification.Height >> (i - 1));
            imageBlit.srcOffsets[1].z = 1;

            imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.dstSubresource.layerCount = 1;
            imageBlit.dstSubresource.mipLevel = i;
            imageBlit.dstOffsets[1].x = int32_t(m_Specification.Width >> i);
            imageBlit.dstOffsets[1].y = int32_t(m_Specification.Height >> i);
            imageBlit.dstOffsets[1].z = 1;

            VkImageSubresourceRange mipSubRange = {};
            mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            mipSubRange.baseMipLevel = i;
            mipSubRange.levelCount = 1;
            mipSubRange.layerCount = 1;

            Utils::InsertImageMemoryBarrier(blitCmd, m_Info.Image,
                0, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                mipSubRange);

            vkCmdBlitImage(
                blitCmd,
                m_Info.Image,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                m_Info.Image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &imageBlit,
                VK_FILTER_LINEAR);

            Utils::InsertImageMemoryBarrier(blitCmd, m_Info.Image,
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                mipSubRange);
        }

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.layerCount = 1;
        subresourceRange.levelCount = mipLevels;

        InsertFinalLayoutBarrier(blitCmd, VK_ACCESS_TRANSFER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, subresourceRange);

        device->FlushCommandBuffer(blitCmd);
    }

    void VulkanImage2D::UpdateDescriptor()
    {
        if (m_Specification.Usage == ImageUsage::Storage)
            m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        else if (Utils::IsDepthFormat(m_Specification.Format))
            m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        else
            m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        m_DescriptorImageInfo.imageView = m_Info.ImageView;
        m_DescriptorImageInfo.sampler = m_Info.Sampler;
    }

    VkImageView VulkanImage2D::GetOrCreateStorageImageView(uint32_t mip)
    {
        auto it = m_StorageViews.find(mip);
        if (it != m_StorageViews.end())
            return it->second;

        VkImageViewCreateInfo view{};
        view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view.format = Utils::VulkanImageFormat(m_Specification.Format);
        view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        view.subresourceRange.aspectMask = Utils::IsDepthFormat(m_Specification.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        view.subresourceRange.baseMipLevel = mip;
        view.subresourceRange.levelCount = 1;
        view.subresourceRange.baseArrayLayer = 0;
        view.subresourceRange.layerCount = 1;
        view.image = m_Info.Image;
        VK_CHECK_RESULT(vkCreateImageView(VulkanContext::GetCurrentDevice()->GetVulkanDevice(), &view, nullptr, &m_StorageViews[mip]));
        return m_StorageViews[mip];
    }

    //////////////////////////////////////////////////////////////////////////////////
    // ImageCube
    //////////////////////////////////////////////////////////////////////////////////

    VulkanImageCube::VulkanImageCube(const ImageSpecification& specification, Buffer buffer)
        : m_Specification(specification), m_ImageData(std::move(buffer))
    {
    }

    VulkanImageCube::~VulkanImageCube()
    {
        Release();
    }

    void VulkanImageCube::Invalidate()
    {
        auto device = VulkanContext::GetCurrentDevice();
        auto vulkanDevice = device->GetVulkanDevice();

        if (m_Info.Image || m_Info.ImageView || m_Info.Sampler)
            Release();

        VkFormat format = Utils::VulkanImageFormat(m_Specification.Format);
        uint32_t mipCount = Utils::CalculateMipCount(m_Specification.Width, m_Specification.Height);

        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = format;
        imageCreateInfo.mipLevels = mipCount;
        imageCreateInfo.arrayLayers = 6;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.extent = { m_Specification.Width, m_Specification.Height, 1 };
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        m_Info.MemoryAlloc = VulkanAllocator::AllocateImage(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_Info.Image);

        m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.layerCount = 6;

        if (m_ImageData)
        {
            VkBuffer stagingBuffer;
            VkBufferCreateInfo stagingCreateInfo{};
            stagingCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            stagingCreateInfo.size = m_ImageData.Size;
            stagingCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            stagingCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            VmaAllocation stagingAllocation = VulkanAllocator::AllocateBuffer(stagingCreateInfo, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);

            uint8_t* stagingData = VulkanAllocator::MapMemory<uint8_t>(stagingAllocation);
            memcpy(stagingData, m_ImageData.Data, m_ImageData.Size);
            VulkanAllocator::UnmapMemory(stagingAllocation);

            VkCommandBuffer copyCmd = device->GetCommandBuffer(true);

            Utils::InsertImageMemoryBarrier(copyCmd, m_Info.Image,
                0, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                subresourceRange);

            VkBufferImageCopy bufferCopyRegion = {};
            bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferCopyRegion.imageSubresource.mipLevel = 0;
            bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
            bufferCopyRegion.imageSubresource.layerCount = 6;
            bufferCopyRegion.imageExtent.width = m_Specification.Width;
            bufferCopyRegion.imageExtent.height = m_Specification.Height;
            bufferCopyRegion.imageExtent.depth = 1;
            bufferCopyRegion.bufferOffset = 0;

            vkCmdCopyBufferToImage(
                copyCmd,
                stagingBuffer,
                m_Info.Image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &bufferCopyRegion);

            Utils::InsertImageMemoryBarrier(copyCmd, m_Info.Image,
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                subresourceRange);

            device->FlushCommandBuffer(copyCmd);

            VulkanAllocator::DestroyBuffer(stagingBuffer, stagingAllocation);
        }

        VkCommandBuffer layoutCmd = device->GetCommandBuffer(true);

        VkImageSubresourceRange fullSubresourceRange = {};
        fullSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        fullSubresourceRange.baseMipLevel = 0;
        fullSubresourceRange.levelCount = mipCount;
        fullSubresourceRange.layerCount = 6;

        Utils::InsertImageMemoryBarrier(layoutCmd, m_Info.Image,
            0, 0,
            VK_IMAGE_LAYOUT_UNDEFINED, m_DescriptorImageInfo.imageLayout,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            fullSubresourceRange);

        device->FlushCommandBuffer(layoutCmd);

        VkSamplerCreateInfo sampler{};
        sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler.magFilter = VK_FILTER_LINEAR;
        sampler.minFilter = VK_FILTER_LINEAR;
        sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler.mipLodBias = 0.0f;
        sampler.compareOp = VK_COMPARE_OP_NEVER;
        sampler.minLod = 0.0f;
        sampler.maxLod = (float)mipCount;
        sampler.maxAnisotropy = 1.0f;
        sampler.anisotropyEnable = VK_FALSE;
        sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VK_CHECK_RESULT(vkCreateSampler(vulkanDevice, &sampler, nullptr, &m_Info.Sampler));

        VkImageViewCreateInfo view{};
        view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        view.format = format;
        view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view.subresourceRange.baseMipLevel = 0;
        view.subresourceRange.baseArrayLayer = 0;
        view.subresourceRange.layerCount = 6;
        view.subresourceRange.levelCount = mipCount;
        view.image = m_Info.Image;
        VK_CHECK_RESULT(vkCreateImageView(vulkanDevice, &view, nullptr, &m_Info.ImageView));

        UpdateDescriptor();
    }

    void VulkanImageCube::Release()
    {
        if (!m_Info.Image && !m_Info.ImageView && !m_Info.Sampler)
            return;

        VkImage image = m_Info.Image;
        VkImageView imageView = m_Info.ImageView;
        VkSampler sampler = m_Info.Sampler;
        VmaAllocation allocation = m_Info.MemoryAlloc;
        auto storageViews = std::move(m_StorageViews);
        m_StorageViews.clear();
        m_Info = {};

        Renderer::SubmitResourceFree([image, imageView, sampler, allocation, storageViews]()
        {
            VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
            if (sampler)
                vkDestroySampler(device, sampler, nullptr);
            if (imageView)
                vkDestroyImageView(device, imageView, nullptr);
            for (auto& [mip, view] : storageViews)
                vkDestroyImageView(device, view, nullptr);
            if (image)
                VulkanAllocator::DestroyImage(image, allocation);
        });

        m_DescriptorImageInfo = {};
    }

    void VulkanImageCube::GenerateMipMap()
    {
        Ref<VulkanImageCube> instance = this;
        Renderer::Submit([instance]() mutable
        {
            instance->RT_GenerateMips();
        });
    }

    void VulkanImageCube::CopyTo(Ref<ImageCube> destination) const
    {
        VkImage srcImage = m_Info.Image;
        VkImage dstImage = destination.As<VulkanImageCube>()->GetImageInfo().Image;
        uint32_t width = m_Specification.Width, height = m_Specification.Height;

        Renderer::Submit([srcImage, dstImage, width, height]()
        {
            auto device = VulkanContext::GetCurrentDevice();
            VkCommandBuffer copyCmd = device->GetCommandBuffer(true, true);

            VkImageCopy region{};
            region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.srcSubresource.mipLevel = 0;
            region.srcSubresource.baseArrayLayer = 0;
            region.srcSubresource.layerCount = 6;
            region.dstSubresource = region.srcSubresource;
            region.extent = { width, height, 1 };

            vkCmdCopyImage(copyCmd,
                srcImage, VK_IMAGE_LAYOUT_GENERAL,
                dstImage, VK_IMAGE_LAYOUT_GENERAL,
                1, &region);

            device->FlushCommandBuffer(copyCmd, true);
        });
    }

    void VulkanImageCube::RT_GenerateMips(bool readonly)
    {
        auto device = VulkanContext::GetCurrentDevice();

        VkImage image = m_Info.Image;

        VkCommandBuffer blitCmd = device->GetCommandBuffer(true);

        // Invalidate 后整图处于 GENERAL，先把 mip 0 转为 blit 源布局
        for (uint32_t face = 0; face < 6; face++)
        {
            VkImageSubresourceRange mipSubRange = {};
            mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            mipSubRange.baseMipLevel = 0;
            mipSubRange.baseArrayLayer = face;
            mipSubRange.levelCount = 1;
            mipSubRange.layerCount = 1;

            Utils::InsertImageMemoryBarrier(blitCmd, image,
                0, VK_ACCESS_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                mipSubRange);
        }

        uint32_t mipLevels = Utils::CalculateMipCount(m_Specification.Width, m_Specification.Height);
        for (uint32_t i = 1; i < mipLevels; i++)
        {
            for (uint32_t face = 0; face < 6; face++)
            {
                VkImageBlit imageBlit{};

                imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                imageBlit.srcSubresource.layerCount = 1;
                imageBlit.srcSubresource.mipLevel = i - 1;
                imageBlit.srcSubresource.baseArrayLayer = face;
                imageBlit.srcOffsets[1].x = int32_t(m_Specification.Width >> (i - 1));
                imageBlit.srcOffsets[1].y = int32_t(m_Specification.Height >> (i - 1));
                imageBlit.srcOffsets[1].z = 1;

                imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                imageBlit.dstSubresource.layerCount = 1;
                imageBlit.dstSubresource.mipLevel = i;
                imageBlit.dstSubresource.baseArrayLayer = face;
                imageBlit.dstOffsets[1].x = int32_t(m_Specification.Width >> i);
                imageBlit.dstOffsets[1].y = int32_t(m_Specification.Height >> i);
                imageBlit.dstOffsets[1].z = 1;

                VkImageSubresourceRange mipSubRange = {};
                mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                mipSubRange.baseMipLevel = i;
                mipSubRange.baseArrayLayer = face;
                mipSubRange.levelCount = 1;
                mipSubRange.layerCount = 1;

                Utils::InsertImageMemoryBarrier(blitCmd, image,
                    0, VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                    mipSubRange);

                vkCmdBlitImage(
                    blitCmd,
                    image,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1,
                    &imageBlit,
                    VK_FILTER_LINEAR);

                Utils::InsertImageMemoryBarrier(blitCmd, image,
                    VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                    mipSubRange);
            }
        }

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.layerCount = 6;
        subresourceRange.levelCount = mipLevels;

        Utils::InsertImageMemoryBarrier(blitCmd, image,
            VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readonly ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            subresourceRange);

        device->FlushCommandBuffer(blitCmd);

        m_DescriptorImageInfo.imageLayout = readonly ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
    }

    void VulkanImageCube::UpdateDescriptor()
    {
        m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        m_DescriptorImageInfo.imageView = m_Info.ImageView;
        m_DescriptorImageInfo.sampler = m_Info.Sampler;
    }

    VkImageView VulkanImageCube::GetOrCreateStorageImageView(uint32_t mip)
    {
        auto it = m_StorageViews.find(mip);
        if (it != m_StorageViews.end())
            return it->second;

        VkImageViewCreateInfo view{};
        view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        view.format = Utils::VulkanImageFormat(m_Specification.Format);
        view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view.subresourceRange.baseMipLevel = mip;
        view.subresourceRange.levelCount = 1;
        view.subresourceRange.baseArrayLayer = 0;
        view.subresourceRange.layerCount = 6;
        view.image = m_Info.Image;
        VK_CHECK_RESULT(vkCreateImageView(VulkanContext::GetCurrentDevice()->GetVulkanDevice(), &view, nullptr, &m_StorageViews[mip]));
        return m_StorageViews[mip];
    }

    namespace Utils
    {
        VkFormat VulkanImageFormat(ImageFormat format)
        {
            switch (format)
            {
                case ImageFormat::R8:         return VK_FORMAT_R8_UNORM;
                case ImageFormat::RG8:        return VK_FORMAT_R8G8_UNORM;
                case ImageFormat::RGB8:       return VK_FORMAT_R8G8B8_UNORM;
                case ImageFormat::RGBA8:      return VK_FORMAT_R8G8B8A8_UNORM;

                case ImageFormat::R8_SRGB:    return VK_FORMAT_R8_SRGB;
                case ImageFormat::RG8_SRGB:   return VK_FORMAT_R8G8_SRGB;
                case ImageFormat::RGB8_SRGB:  return VK_FORMAT_R8G8B8_SRGB;
                case ImageFormat::RGBA8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;

                case ImageFormat::R8_SNORM:    return VK_FORMAT_R8_SNORM;
                case ImageFormat::RG8_SNORM:   return VK_FORMAT_R8G8_SNORM;
                case ImageFormat::RGB8_SNORM:  return VK_FORMAT_R8G8B8_SNORM;
                case ImageFormat::RGBA8_SNORM: return VK_FORMAT_R8G8B8A8_SNORM;

                case ImageFormat::R16F:    return VK_FORMAT_R16_SFLOAT;
                case ImageFormat::RG16F:   return VK_FORMAT_R16G16_SFLOAT;
                case ImageFormat::RGB16F:  return VK_FORMAT_R16G16B16_SFLOAT;
                case ImageFormat::RGBA16F: return VK_FORMAT_R16G16B16A16_SFLOAT;

                case ImageFormat::R32F:    return VK_FORMAT_R32_SFLOAT;
                case ImageFormat::RG32F:   return VK_FORMAT_R32G32_SFLOAT;
                case ImageFormat::RGB32F:  return VK_FORMAT_R32G32B32_SFLOAT;
                case ImageFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;

                case ImageFormat::R16_UINT:    return VK_FORMAT_R16_UINT;
                case ImageFormat::RG16_UINT:   return VK_FORMAT_R16G16_UINT;
                case ImageFormat::RGBA16_UINT: return VK_FORMAT_R16G16B16A16_UINT;
                case ImageFormat::R32_UINT:    return VK_FORMAT_R32_UINT;
                case ImageFormat::RG32_UINT:   return VK_FORMAT_R32G32_UINT;
                case ImageFormat::RGBA32_UINT: return VK_FORMAT_R32G32B32A32_UINT;

                case ImageFormat::R16_SINT:    return VK_FORMAT_R16_SINT;
                case ImageFormat::RG16_SINT:   return VK_FORMAT_R16G16_SINT;
                case ImageFormat::RGBA16_SINT: return VK_FORMAT_R16G16B16A16_SINT;
                case ImageFormat::R32_SINT:    return VK_FORMAT_R32_SINT;
                case ImageFormat::RG32_SINT:   return VK_FORMAT_R32G32_SINT;
                case ImageFormat::RGBA32_SINT: return VK_FORMAT_R32G32B32A32_SINT;

                case ImageFormat::RGB565:   return VK_FORMAT_R5G6B5_UNORM_PACK16;
                case ImageFormat::RGBA4:    return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
                case ImageFormat::RGB5A1:   return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
                case ImageFormat::RGB10A2:  return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
                case ImageFormat::RG11B10F: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
                case ImageFormat::RGB9E5:   return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

                case ImageFormat::DEPTH16:          return VK_FORMAT_D16_UNORM;
                case ImageFormat::DEPTH32F:         return VK_FORMAT_D32_SFLOAT;
                case ImageFormat::DEPTH32FSTENCIL8: return VK_FORMAT_D32_SFLOAT_S8_UINT;
                case ImageFormat::DEPTH24STENCIL8:  return VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetDepthFormat();

                case ImageFormat::BC1:       return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
                case ImageFormat::BC1_SRGB:  return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
                case ImageFormat::BC2:       return VK_FORMAT_BC2_UNORM_BLOCK;
                case ImageFormat::BC2_SRGB:  return VK_FORMAT_BC2_SRGB_BLOCK;
                case ImageFormat::BC3:       return VK_FORMAT_BC3_UNORM_BLOCK;
                case ImageFormat::BC3_SRGB:  return VK_FORMAT_BC3_SRGB_BLOCK;
                case ImageFormat::BC4:       return VK_FORMAT_BC4_UNORM_BLOCK;
                case ImageFormat::BC5:       return VK_FORMAT_BC5_UNORM_BLOCK;
                case ImageFormat::BC6H_UF16: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
                case ImageFormat::BC6H_SF16: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
                case ImageFormat::BC7:       return VK_FORMAT_BC7_UNORM_BLOCK;
                case ImageFormat::BC7_SRGB:  return VK_FORMAT_BC7_SRGB_BLOCK;

                case ImageFormat::ETC2_RGB8:       return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
                case ImageFormat::ETC2_RGB8_SRGB:  return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
                case ImageFormat::ETC2_RGBA8:      return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
                case ImageFormat::ETC2_RGBA8_SRGB: return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;

                case ImageFormat::ASTC_4x4: return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
                case ImageFormat::ASTC_5x5: return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
                case ImageFormat::ASTC_6x6: return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
                case ImageFormat::ASTC_8x8: return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
            }
            PR_CORE_ASSERT(false, "Unknown image format");
            return VK_FORMAT_UNDEFINED;
        }
    }
}
