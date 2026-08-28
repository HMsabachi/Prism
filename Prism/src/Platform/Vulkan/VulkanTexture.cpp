#include "prpch.h"
#include "Platform/Vulkan/VulkanTexture.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Prism/Core/RenderThread.h"
#include "Prism/Renderer/Renderer.h"

#include "stb_image.h"

namespace Prism
{
    //////////////////////////////////////////////////////////////////////////////////
    // Texture2D
    //////////////////////////////////////////////////////////////////////////////////

    VulkanTexture2D::VulkanTexture2D(const std::string& path, bool srgb)
        : m_Path(path)
    {
        int width, height, channels;
        Buffer imageData;
        if (stbi_is_hdr(path.c_str()))
        {
            PR_CORE_INFO("Loading HDR texture {0}, srgb={1}", path, srgb);
            float* data = stbi_loadf(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            PR_CORE_ASSERT(data, "Could not read HDR image!");
            m_Format = ImageFormat::RGBA32F;
            imageData = Buffer::Copy((byte*)data, width * height * 4 * sizeof(float));
            stbi_image_free(data);
        }
        else
        {
            PR_CORE_INFO("Loading texture {0}, srgb={1}", path, srgb);
            stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            PR_CORE_ASSERT(data, "Could not read image!");
            m_Format = srgb ? ImageFormat::SRGB : ImageFormat::RGBA;
            imageData = Buffer::Copy((byte*)data, width * height * 4);
            stbi_image_free(data);
        }

        m_Width = width;
        m_Height = height;
        m_Loaded = true;

        m_Image = Image2D::Create(m_Format, m_Width, m_Height, std::move(imageData));

        if (RenderThread::IsCurrentThreadRT())
            Invalidate();
        else
        {
            Ref<VulkanTexture2D> instance = this;
            Renderer::Submit([instance]() mutable
            {
                instance->Invalidate();
            });
        }
    }

    VulkanTexture2D::VulkanTexture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data, TextureWrap wrap)
        : m_Format(format), m_Wrap(wrap), m_Width(width), m_Height(height)
    {
        m_Loaded = true;

        m_Image = Image2D::Create(format, width, height, data);
        if (!data)
            m_Image->GetBuffer().Allocate(Utils::GetImageMemorySize(format, width, height));

        if (RenderThread::IsCurrentThreadRT())
            Invalidate();
        else
        {
            Ref<VulkanTexture2D> instance = this;
            Renderer::Submit([instance]() mutable
            {
                instance->Invalidate();
            });
        }
    }

    VulkanTexture2D::~VulkanTexture2D()
    {
        if (m_Image)
            m_Image->Release();
    }

    void VulkanTexture2D::Invalidate()
    {
        m_Image.As<VulkanImage2D>()->SetSamplerWrap(m_Wrap);
        m_Image->Invalidate();
    }

    void VulkanTexture2D::Lock()
    {
    }

    void VulkanTexture2D::Unlock()
    {
    }

    Buffer VulkanTexture2D::GetWriteableBuffer()
    {
        return m_Image->GetBuffer();
    }

    uint32_t VulkanTexture2D::GetMipLevelCount() const
    {
        return Utils::CalculateMipCount(m_Width, m_Height);
    }

    //////////////////////////////////////////////////////////////////////////////////
    // TextureCube
    //////////////////////////////////////////////////////////////////////////////////

    VulkanTextureCube::VulkanTextureCube(ImageFormat format, uint32_t width, uint32_t height, const void* data)
        : m_Format(format), m_Width(width), m_Height(height)
    {
        m_Image = ImageCube::Create(format, width, height, data);

        if (RenderThread::IsCurrentThreadRT())
            Invalidate();
        else
        {
            Ref<VulkanTextureCube> instance = this;
            Renderer::Submit([instance]() mutable
            {
                instance->Invalidate();
            });
        }
    }

    VulkanTextureCube::~VulkanTextureCube()
    {
        if (m_Image)
            m_Image->Release();
    }

    void VulkanTextureCube::Invalidate()
    {
        m_Image->Invalidate();
    }

    uint32_t VulkanTextureCube::GetMipLevelCount() const
    {
        return Utils::CalculateMipCount(m_Width, m_Height);
    }

    void VulkanTextureCube::CopyTo(Ref<TextureCube> destination) const
    {
        VkImage srcImage = m_Image.As<VulkanImageCube>()->GetImageInfo().Image;
        VkImage dstImage = destination->GetImage().As<VulkanImageCube>()->GetImageInfo().Image;
        uint32_t width = m_Width, height = m_Height;

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

            // 双方均处 GENERAL（VulkanImageCube 终态），vkCmdCopyImage 在 GENERAL 布局下合法
            vkCmdCopyImage(copyCmd,
                srcImage, VK_IMAGE_LAYOUT_GENERAL,
                dstImage, VK_IMAGE_LAYOUT_GENERAL,
                1, &region);

            device->FlushCommandBuffer(copyCmd, true);
        });
    }

    void VulkanTextureCube::RT_GenerateMips(bool readonly)
    {
        m_Image.As<VulkanImageCube>()->RT_GenerateMips(readonly);
    }
}
