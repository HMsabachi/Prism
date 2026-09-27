#include "prpch.h"
#include "Platform/Vulkan/VulkanTexture.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Prism/Core/RenderThread.h"
#include "Prism/Renderer/Renderer.h"
#include "Prism/Utilities/TextureUtils.h"

#include "stb_image.h"

namespace Prism
{
    //////////////////////////////////////////////////////////////////////////////////
    // Texture2D
    //////////////////////////////////////////////////////////////////////////////////

    VulkanTexture2D::VulkanTexture2D(const TextureSpecification& specification, const std::string& path)
        : m_Path(path), m_Specification(specification)
    {
        if (IsDDSFile(path))
        {
            TextureLoadResult dds;
            if (!LoadDDS(path, dds))
            {
                PR_CORE_ERROR("Failed to load DDS: {0}", path);
                return;
            }
            m_Specification.Format = dds.Format;
            m_Specification.Width = dds.Width;
            m_Specification.Height = dds.Height;
            m_Loaded = true;

            ImageSpecification imageSpecification{};
            imageSpecification.Format = m_Specification.Format;
            imageSpecification.Width = m_Specification.Width;
            imageSpecification.Height = m_Specification.Height;
            imageSpecification.CreateSampler = false;
            m_Image = Image2D::Create(imageSpecification, std::move(dds.Mips));

            if (RenderThread::IsCurrentThreadRT())
                Invalidate();
            else
            {
                Ref<VulkanTexture2D> instance = this;
                Renderer::Submit([instance]() mutable { instance->Invalidate(); });
            }
            return;
        }

        int width, height, channels;
        Buffer imageData;
        void* data = nullptr;
        if (stbi_is_hdr(path.c_str()))
        {
            PR_CORE_INFO("Loading HDR texture {0}, srgb={1}", path, Utils::IsSRGBFormat(m_Specification.Format));
            data = stbi_loadf(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            // PR_CORE_ASSERT(data, "Could not read HDR image!");
            if (!data) { PR_CORE_ERROR("Could not read image: {0}", path); return; }
            m_Specification.Format = ImageFormat::RGBA32F;
            imageData = Buffer::Copy((byte*)data, width * height * 4 * sizeof(float));
            stbi_image_free(data);
        }
        else
        {
            PR_CORE_INFO("Loading texture {0}, srgb={1}", path, Utils::IsSRGBFormat(m_Specification.Format));
            data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            // PR_CORE_ASSERT(data, "Could not read image!");
            if (!data) { PR_CORE_ERROR("Could not read image: {0}", path); return; }
            m_Specification.Format = Utils::IsSRGBFormat(m_Specification.Format) ? ImageFormat::RGBA8_SRGB : ImageFormat::RGBA8;
            imageData = Buffer::Copy((byte*)data, width * height * 4);
            stbi_image_free(data);
        }

        m_Specification.Width = width;
        m_Specification.Height = height;
        m_Loaded = true;

        ImageSpecification imageSpecification{};
        imageSpecification.Format = m_Specification.Format;
        imageSpecification.Width = m_Specification.Width;
        imageSpecification.Height = m_Specification.Height;
        imageSpecification.CreateSampler = false;
        m_Image = Image2D::Create(imageSpecification, std::move(imageData));

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

    VulkanTexture2D::VulkanTexture2D(const TextureSpecification& specification, Buffer imageData)
        : m_Specification(specification)
    {
        m_Loaded = true;

        ImageSpecification imageSpecification{};
        imageSpecification.Format = m_Specification.Format;
        imageSpecification.Width = m_Specification.Width;
        imageSpecification.Height = m_Specification.Height;
        imageSpecification.CreateSampler = false;

        bool hasData = imageData;
        if (hasData)
            m_Image = Image2D::Create(imageSpecification, std::move(imageData));
        else
            m_Image = Image2D::Create(imageSpecification);

        if (!hasData)
            m_Image->GetBuffer().Allocate(Utils::GetImageMemorySize(m_Specification.Format, m_Specification.Width, m_Specification.Height));

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
    }

    static VkSampler CreateSampler(const TextureSpecification& specification)
    {
        VkSamplerCreateInfo sampler{};
        sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler.magFilter = Utils::VulkanSamplerFilter(specification.SamplerFilter);
        sampler.minFilter = Utils::VulkanSamplerFilter(specification.SamplerFilter);
        sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler.addressModeU = Utils::VulkanSamplerWrap(specification.SamplerWrap);
        sampler.addressModeV = Utils::VulkanSamplerWrap(specification.SamplerWrap);
        sampler.addressModeW = Utils::VulkanSamplerWrap(specification.SamplerWrap);
        sampler.mipLodBias = 0.0f;
        sampler.compareOp = VK_COMPARE_OP_NEVER;
        sampler.minLod = 0.0f;
        sampler.maxLod = VK_LOD_CLAMP_NONE;
        sampler.maxAnisotropy = 1.0f;
        sampler.anisotropyEnable = VK_FALSE;
        sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        VkSampler result = VK_NULL_HANDLE;
        VK_CHECK_RESULT(vkCreateSampler(VulkanContext::GetCurrentDevice()->GetVulkanDevice(), &sampler, nullptr, &result));
        return result;
    }

    void VulkanTexture2D::Invalidate()
    {
        m_Image->Invalidate();

        Ref<VulkanImage2D> image = m_Image.As<VulkanImage2D>();
        image->GetImageInfo().Sampler = CreateSampler(m_Specification);
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
        return m_Image->GetBuffer();
    }

    uint32_t VulkanTexture2D::GetMipLevelCount() const
    {
        return Utils::CalculateMipCount(m_Specification.Width, m_Specification.Height);
    }

    //////////////////////////////////////////////////////////////////////////////////
    // TextureCube
    //////////////////////////////////////////////////////////////////////////////////

    VulkanTextureCube::VulkanTextureCube(const TextureSpecification& specification, Buffer imageData)
        : m_Specification(specification)
    {
        ImageSpecification imageSpecification{};
        imageSpecification.Format = m_Specification.Format;
        imageSpecification.Width = m_Specification.Width;
        imageSpecification.Height = m_Specification.Height;
        imageSpecification.CreateSampler = false;

        if (imageData)
            m_Image = ImageCube::Create(imageSpecification, std::move(imageData));
        else
            m_Image = ImageCube::Create(imageSpecification);

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
    }

    void VulkanTextureCube::Invalidate()
    {
        m_Image->Invalidate();

        Ref<VulkanImageCube> image = m_Image.As<VulkanImageCube>();
        image->GetImageInfo().Sampler = CreateSampler(m_Specification);
        image->UpdateDescriptor();
    }

    uint32_t VulkanTextureCube::GetMipLevelCount() const
    {
        return Utils::CalculateMipCount(m_Specification.Width, m_Specification.Height);
    }
}
