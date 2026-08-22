#include "prpch.h"
#include "Platform/Vulkan/VulkanImage.h"

#include "Platform/Vulkan/VulkanAllocator.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Prism/Renderer/Renderer.h"

namespace Prism
{
    VulkanImage2D::VulkanImage2D(ImageFormat format, uint32_t width, uint32_t height, Buffer buffer, uint32_t samples)
        : m_Format(format), m_Width(width), m_Height(height), m_Samples(samples)
    {
        m_ImageData = std::move(buffer);
    }

    VulkanImage2D::VulkanImage2D(ImageFormat format, uint32_t width, uint32_t height, const void* data, uint32_t samples)
        : m_Format(format), m_Width(width), m_Height(height), m_Samples(samples)
    {
        if (data)
            m_ImageData = Buffer::Copy((byte*)data, Utils::GetImageMemorySize(format, width, height));
    }

    VulkanImage2D::~VulkanImage2D()
    {
        Release();
    }

    void VulkanImage2D::Invalidate()
    {
    }

    void VulkanImage2D::Release()
    {
        if (!m_Info.Image && !m_Info.ImageView && !m_Info.Sampler)
            return;

        VkImage image = m_Info.Image;
        VkImageView imageView = m_Info.ImageView;
        VkSampler sampler = m_Info.Sampler;
        VmaAllocation allocation = m_Info.MemoryAlloc;
        m_Info = {};

        Renderer::SubmitResourceFree([image, imageView, sampler, allocation]()
        {
            VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
            if (sampler)
                vkDestroySampler(device, sampler, nullptr);
            if (imageView)
                vkDestroyImageView(device, imageView, nullptr);
            if (image)
                VulkanAllocator::DestroyImage(image, allocation);
        });

        m_DescriptorImageInfo = {};
    }

    void VulkanImage2D::UpdateDescriptor()
    {
        if (m_Format == ImageFormat::DEPTH24STENCIL8 || m_Format == ImageFormat::DEPTH32F)
            m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        else
            m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        m_DescriptorImageInfo.imageView = m_Info.ImageView;
        m_DescriptorImageInfo.sampler = m_Info.Sampler;
    }

    VulkanImageCube::VulkanImageCube(ImageFormat format, uint32_t width, uint32_t height, const void* data)
        : m_Format(format), m_Width(width), m_Height(height)
    {
        if (data)
            m_ImageData = Buffer::Copy((byte*)data, Utils::GetImageMemorySize(format, width, height) * 6);
    }

    VulkanImageCube::~VulkanImageCube()
    {
        Release();
    }

    void VulkanImageCube::Invalidate()
    {
    }

    void VulkanImageCube::Release()
    {
        if (!m_Info.Image && !m_Info.ImageView && !m_Info.Sampler)
            return;

        VkImage image = m_Info.Image;
        VkImageView imageView = m_Info.ImageView;
        VkSampler sampler = m_Info.Sampler;
        VmaAllocation allocation = m_Info.MemoryAlloc;
        m_Info = {};

        Renderer::SubmitResourceFree([image, imageView, sampler, allocation]()
        {
            VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
            if (sampler)
                vkDestroySampler(device, sampler, nullptr);
            if (imageView)
                vkDestroyImageView(device, imageView, nullptr);
            if (image)
                VulkanAllocator::DestroyImage(image, allocation);
        });

        m_DescriptorImageInfo = {};
    }

    void VulkanImageCube::UpdateDescriptor()
    {
        m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        m_DescriptorImageInfo.imageView = m_Info.ImageView;
        m_DescriptorImageInfo.sampler = m_Info.Sampler;
    }

    namespace Utils
    {
        VkFormat VulkanImageFormat(ImageFormat format)
        {
            switch (format)
            {
                case ImageFormat::RGB:             return VK_FORMAT_R8G8B8_UNORM;
                case ImageFormat::SRGB:            return VK_FORMAT_R8G8B8A8_SRGB;
                case ImageFormat::RGBA:            return VK_FORMAT_R8G8B8A8_UNORM;
                case ImageFormat::RGBA16F:         return VK_FORMAT_R16G16B16A16_SFLOAT;
                case ImageFormat::RGBA32F:         return VK_FORMAT_R32G32B32A32_SFLOAT;
                case ImageFormat::RG32F:           return VK_FORMAT_R32G32_SFLOAT;
                case ImageFormat::DEPTH32F:        return VK_FORMAT_D32_SFLOAT;
                case ImageFormat::DEPTH24STENCIL8: return VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetDepthFormat();
            }
            PR_CORE_ASSERT(false, "Unknown image format");
            return VK_FORMAT_UNDEFINED;
        }
    }
}
