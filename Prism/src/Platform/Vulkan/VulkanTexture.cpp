#include "prpch.h"
#include "Platform/Vulkan/VulkanTexture.h"

#include "Platform/Vulkan/VulkanAllocator.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Prism/Renderer/Renderer.h"

#include "stb_image.h"

namespace Prism
{
    namespace Utils
    {
        static void InsertImageMemoryBarrier(
            VkCommandBuffer cmdbuffer,
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
                cmdbuffer,
                srcStageMask,
                dstStageMask,
                0,
                0, nullptr,
                0, nullptr,
                1, &imageMemoryBarrier);
        }
    }

    //////////////////////////////////////////////////////////////////////////////////
    // Texture2D
    //////////////////////////////////////////////////////////////////////////////////

    VulkanTexture2D::VulkanTexture2D(const std::string& path, bool srgb)
        : m_Path(path)
    {
        int width, height, channels;
        if (stbi_is_hdr(path.c_str()))
        {
            PR_CORE_INFO("Loading HDR texture {0}, srgb={1}", path, srgb);
            float* data = stbi_loadf(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            PR_CORE_ASSERT(data, "Could not read HDR image!");
            m_Format = ImageFormat::RGBA16F;
            m_ImageData = Buffer::Copy((byte*)data, width * height * 4 * sizeof(float));
            stbi_image_free(data);
        }
        else
        {
            PR_CORE_INFO("Loading texture {0}, srgb={1}", path, srgb);
            stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            PR_CORE_ASSERT(data, "Could not read image!");
            m_Format = srgb ? ImageFormat::SRGB : ImageFormat::RGBA;
            m_ImageData = Buffer::Copy((byte*)data, width * height * 4);
            stbi_image_free(data);
        }

        m_Width = width;
        m_Height = height;
        m_Loaded = true;

        Ref<VulkanTexture2D> instance = this;
        Renderer::Submit([instance]() mutable
        {
            instance->Invalidate();
        });
    }

    VulkanTexture2D::VulkanTexture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data, TextureWrap wrap)
        : m_Format(format)
    {
        m_Width = width;
        m_Height = height;
        m_Loaded = true;

        if (data)
            m_ImageData = Buffer::Copy((byte*)data, Utils::GetImageMemorySize(format, width, height));

        Ref<VulkanTexture2D> instance = this;
        Renderer::Submit([instance]() mutable
        {
            instance->Invalidate();
        });
    }

    VulkanTexture2D::~VulkanTexture2D()
    {
        if (m_Image)
            m_Image->Release();
    }

    void VulkanTexture2D::Invalidate()
    {
        auto device = VulkanContext::GetCurrentDevice();
        auto vulkanDevice = device->GetVulkanDevice();

        if (m_Image)
            m_Image->Release();

        m_Image = Image2D::Create(m_Format, m_Width, m_Height);
        Ref<VulkanImage2D> image = m_Image.As<VulkanImage2D>();
        auto& info = image->GetImageInfo();

        VkFormat format = Utils::VulkanImageFormat(m_Format);
        uint32_t mipCount = m_ImageData ? GetMipLevelCount() : 1;

        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = format;
        imageCreateInfo.mipLevels = mipCount;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.extent = { m_Width, m_Height, 1 };
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        info.MemoryAlloc = VulkanAllocator::AllocateImage(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, info.Image);

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.layerCount = 1;

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

            Utils::InsertImageMemoryBarrier(copyCmd, info.Image,
                0, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                subresourceRange);

            VkBufferImageCopy bufferCopyRegion = {};
            bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferCopyRegion.imageSubresource.mipLevel = 0;
            bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
            bufferCopyRegion.imageSubresource.layerCount = 1;
            bufferCopyRegion.imageExtent.width = m_Width;
            bufferCopyRegion.imageExtent.height = m_Height;
            bufferCopyRegion.imageExtent.depth = 1;
            bufferCopyRegion.bufferOffset = 0;

            vkCmdCopyBufferToImage(
                copyCmd,
                stagingBuffer,
                info.Image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &bufferCopyRegion);

            Utils::InsertImageMemoryBarrier(copyCmd, info.Image,
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                subresourceRange);

            device->FlushCommandBuffer(copyCmd);

            VulkanAllocator::DestroyBuffer(stagingBuffer, stagingAllocation);
        }
        else
        {
            VkCommandBuffer layoutCmd = device->GetCommandBuffer(true);

            Utils::InsertImageMemoryBarrier(layoutCmd, info.Image,
                0, VK_ACCESS_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                subresourceRange);

            device->FlushCommandBuffer(layoutCmd);
        }

        VkSamplerCreateInfo sampler{};
        sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler.magFilter = VK_FILTER_LINEAR;
        sampler.minFilter = VK_FILTER_LINEAR;
        sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler.mipLodBias = 0.0f;
        sampler.compareOp = VK_COMPARE_OP_NEVER;
        sampler.minLod = 0.0f;
        sampler.maxLod = (float)mipCount;
        sampler.maxAnisotropy = 1.0f;
        sampler.anisotropyEnable = VK_FALSE;
        sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VK_CHECK_RESULT(vkCreateSampler(vulkanDevice, &sampler, nullptr, &info.Sampler));

        VkImageViewCreateInfo view{};
        view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view.format = format;
        view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view.subresourceRange.baseMipLevel = 0;
        view.subresourceRange.baseArrayLayer = 0;
        view.subresourceRange.layerCount = 1;
        view.subresourceRange.levelCount = mipCount;
        view.image = info.Image;
        VK_CHECK_RESULT(vkCreateImageView(vulkanDevice, &view, nullptr, &info.ImageView));

        if (m_ImageData)
            GenerateMips();
        image->UpdateDescriptor();
    }

    void VulkanTexture2D::Lock()
    {
    }

    void VulkanTexture2D::Unlock()
    {
    }

    Buffer VulkanTexture2D::GetWriteableBuffer()
    {
        return m_ImageData;
    }

    uint32_t VulkanTexture2D::GetMipLevelCount() const
    {
        return Utils::CalculateMipCount(m_Width, m_Height);
    }

    void VulkanTexture2D::GenerateMips()
    {
        auto device = VulkanContext::GetCurrentDevice();

        Ref<VulkanImage2D> image = m_Image.As<VulkanImage2D>();
        auto& info = image->GetImageInfo();

        VkCommandBuffer blitCmd = device->GetCommandBuffer(true);

        uint32_t mipLevels = GetMipLevelCount();
        for (uint32_t i = 1; i < mipLevels; i++)
        {
            VkImageBlit imageBlit{};

            imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.srcSubresource.layerCount = 1;
            imageBlit.srcSubresource.mipLevel = i - 1;
            imageBlit.srcOffsets[1].x = int32_t(m_Width >> (i - 1));
            imageBlit.srcOffsets[1].y = int32_t(m_Height >> (i - 1));
            imageBlit.srcOffsets[1].z = 1;

            imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.dstSubresource.layerCount = 1;
            imageBlit.dstSubresource.mipLevel = i;
            imageBlit.dstOffsets[1].x = int32_t(m_Width >> i);
            imageBlit.dstOffsets[1].y = int32_t(m_Height >> i);
            imageBlit.dstOffsets[1].z = 1;

            VkImageSubresourceRange mipSubRange = {};
            mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            mipSubRange.baseMipLevel = i;
            mipSubRange.levelCount = 1;
            mipSubRange.layerCount = 1;

            Utils::InsertImageMemoryBarrier(blitCmd, info.Image,
                0, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                mipSubRange);

            vkCmdBlitImage(
                blitCmd,
                info.Image,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                info.Image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &imageBlit,
                VK_FILTER_LINEAR);

            Utils::InsertImageMemoryBarrier(blitCmd, info.Image,
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                mipSubRange);
        }

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.layerCount = 1;
        subresourceRange.levelCount = mipLevels;

        Utils::InsertImageMemoryBarrier(blitCmd, info.Image,
            VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            subresourceRange);

        device->FlushCommandBuffer(blitCmd);
    }

    //////////////////////////////////////////////////////////////////////////////////
    // TextureCube
    //////////////////////////////////////////////////////////////////////////////////

    VulkanTextureCube::VulkanTextureCube(ImageFormat format, uint32_t width, uint32_t height, const void* data)
        : m_Format(format), m_Width(width), m_Height(height)
    {
        if (data)
            m_LocalStorage = Buffer::Copy((byte*)data, Utils::GetImageMemorySize(format, width, height) * 6);

        Ref<VulkanTextureCube> instance = this;
        Renderer::Submit([instance]() mutable
        {
            instance->Invalidate();
        });
    }

    VulkanTextureCube::~VulkanTextureCube()
    {
        if (m_Image)
            m_Image->Release();
    }

    void VulkanTextureCube::Invalidate()
    {
        auto device = VulkanContext::GetCurrentDevice();
        auto vulkanDevice = device->GetVulkanDevice();

        if (m_Image)
            m_Image->Release();

        m_Image = ImageCube::Create(m_Format, m_Width, m_Height);
        Ref<VulkanImageCube> image = m_Image.As<VulkanImageCube>();
        auto& info = image->GetImageInfo();

        VkFormat format = Utils::VulkanImageFormat(m_Format);
        uint32_t mipCount = GetMipLevelCount();

        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = format;
        imageCreateInfo.mipLevels = mipCount;
        imageCreateInfo.arrayLayers = 6;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.extent = { m_Width, m_Height, 1 };
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        info.MemoryAlloc = VulkanAllocator::AllocateImage(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, info.Image);

        m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.layerCount = 6;

        if (m_LocalStorage)
        {
            VkBuffer stagingBuffer;
            VkBufferCreateInfo stagingCreateInfo{};
            stagingCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            stagingCreateInfo.size = m_LocalStorage.Size;
            stagingCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            stagingCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            VmaAllocation stagingAllocation = VulkanAllocator::AllocateBuffer(stagingCreateInfo, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);

            uint8_t* stagingData = VulkanAllocator::MapMemory<uint8_t>(stagingAllocation);
            memcpy(stagingData, m_LocalStorage.Data, m_LocalStorage.Size);
            VulkanAllocator::UnmapMemory(stagingAllocation);

            VkCommandBuffer copyCmd = device->GetCommandBuffer(true);

            Utils::InsertImageMemoryBarrier(copyCmd, info.Image,
                0, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                subresourceRange);

            VkBufferImageCopy bufferCopyRegion = {};
            bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferCopyRegion.imageSubresource.mipLevel = 0;
            bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
            bufferCopyRegion.imageSubresource.layerCount = 6;
            bufferCopyRegion.imageExtent.width = m_Width;
            bufferCopyRegion.imageExtent.height = m_Height;
            bufferCopyRegion.imageExtent.depth = 1;
            bufferCopyRegion.bufferOffset = 0;

            vkCmdCopyBufferToImage(
                copyCmd,
                stagingBuffer,
                info.Image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &bufferCopyRegion);

            Utils::InsertImageMemoryBarrier(copyCmd, info.Image,
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

        Utils::InsertImageMemoryBarrier(layoutCmd, info.Image,
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
        VK_CHECK_RESULT(vkCreateSampler(vulkanDevice, &sampler, nullptr, &info.Sampler));

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
        view.image = info.Image;
        VK_CHECK_RESULT(vkCreateImageView(vulkanDevice, &view, nullptr, &info.ImageView));

        m_DescriptorImageInfo.sampler = info.Sampler;
        m_DescriptorImageInfo.imageView = info.ImageView;
        image->UpdateDescriptor();
    }

    uint32_t VulkanTextureCube::GetMipLevelCount() const
    {
        return Utils::CalculateMipCount(m_Width, m_Height);
    }

    VkImageView VulkanTextureCube::CreateImageViewSingleMip(uint32_t mip)
    {
        auto device = VulkanContext::GetCurrentDevice();

        VkFormat format = Utils::VulkanImageFormat(m_Format);

        VkImageViewCreateInfo view{};
        view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        view.format = format;
        view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view.subresourceRange.baseMipLevel = mip;
        view.subresourceRange.baseArrayLayer = 0;
        view.subresourceRange.layerCount = 6;
        view.subresourceRange.levelCount = 1;
        view.image = m_Image.As<VulkanImageCube>()->GetImageInfo().Image;

        VkImageView result;
        VK_CHECK_RESULT(vkCreateImageView(device->GetVulkanDevice(), &view, nullptr, &result));
        return result;
    }

    void VulkanTextureCube::GenerateMips(bool readonly)
    {
        auto device = VulkanContext::GetCurrentDevice();

        VkImage image = m_Image.As<VulkanImageCube>()->GetImageInfo().Image;

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

        uint32_t mipLevels = GetMipLevelCount();
        for (uint32_t i = 1; i < mipLevels; i++)
        {
            for (uint32_t face = 0; face < 6; face++)
            {
                VkImageBlit imageBlit{};

                imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                imageBlit.srcSubresource.layerCount = 1;
                imageBlit.srcSubresource.mipLevel = i - 1;
                imageBlit.srcSubresource.baseArrayLayer = face;
                imageBlit.srcOffsets[1].x = int32_t(m_Width >> (i - 1));
                imageBlit.srcOffsets[1].y = int32_t(m_Height >> (i - 1));
                imageBlit.srcOffsets[1].z = 1;

                imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                imageBlit.dstSubresource.layerCount = 1;
                imageBlit.dstSubresource.mipLevel = i;
                imageBlit.dstSubresource.baseArrayLayer = face;
                imageBlit.dstOffsets[1].x = int32_t(m_Width >> i);
                imageBlit.dstOffsets[1].y = int32_t(m_Height >> i);
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
}
