#pragma once

#include "Prism/Renderer/Image.h"
#include "Prism/Renderer/Texture.h"
#include "Platform/Vulkan/Vulkan.h"

#include "VulkanMemoryAllocator/vk_mem_alloc.h"

namespace Prism
{
    struct VulkanImageInfo
    {
        VkImage Image = VK_NULL_HANDLE;
        VkImageView ImageView = VK_NULL_HANDLE;
        VkSampler Sampler = VK_NULL_HANDLE;
        VmaAllocation MemoryAlloc = nullptr;
    };

    class PRISM_API VulkanImage2D : public Image2D
    {
    public:
        VulkanImage2D(ImageFormat format, uint32_t width, uint32_t height, Buffer buffer, uint32_t samples = 1);
        VulkanImage2D(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr, uint32_t samples = 1);
        virtual ~VulkanImage2D();

        virtual void Resize(const uint32_t width, const uint32_t height) override;
        virtual void Invalidate() override;
        virtual void Release() override;

        virtual uint32_t GetWidth() const override { return m_Width; }
        virtual uint32_t GetHeight() const override { return m_Height; }
        virtual uint32_t GetSamples() const override { return m_Samples; }
        virtual ImageFormat GetFormat() const override { return m_Format; }

        virtual Buffer GetBuffer() const override { return m_ImageData; }
        virtual Buffer& GetBuffer() override { return m_ImageData; }

        virtual uint64_t GetHash() const override { return (uint64_t)m_Info.Image; }

        VulkanImageInfo& GetImageInfo() { return m_Info; }
        const VulkanImageInfo& GetImageInfo() const { return m_Info; }

        const VkDescriptorImageInfo& GetDescriptor() const { return m_DescriptorImageInfo; }

        void SetSamplerWrap(TextureWrap wrap) { m_Wrap = wrap; }

        void RT_Resize(const uint32_t width, const uint32_t height);
        void RT_Invalidate();
        void GenerateMips();
        void UpdateDescriptor();
    private:
        ImageFormat m_Format = ImageFormat::None;
        uint32_t m_Width = 0, m_Height = 0;
        uint32_t m_Samples = 1;
        TextureWrap m_Wrap = TextureWrap::Repeat;

        Buffer m_ImageData;

        VulkanImageInfo m_Info;
        VkDescriptorImageInfo m_DescriptorImageInfo = {};
    };

    class PRISM_API VulkanImageCube : public ImageCube
    {
    public:
        VulkanImageCube(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr);
        virtual ~VulkanImageCube();

        virtual void Invalidate() override;
        virtual void Release() override;

        virtual uint32_t GetWidth() const override { return m_Width; }
        virtual uint32_t GetHeight() const override { return m_Height; }
        virtual uint32_t GetSamples() const override { return 1; }
        virtual ImageFormat GetFormat() const override { return m_Format; }

        virtual Buffer GetBuffer() const override { return m_ImageData; }
        virtual Buffer& GetBuffer() override { return m_ImageData; }

        virtual uint64_t GetHash() const override { return (uint64_t)m_Info.Image; }

        VulkanImageInfo& GetImageInfo() { return m_Info; }
        const VulkanImageInfo& GetImageInfo() const { return m_Info; }

        const VkDescriptorImageInfo& GetDescriptor() const { return m_DescriptorImageInfo; }

        void GenerateMips(bool readonly = false);
        void UpdateDescriptor();
    private:
        ImageFormat m_Format = ImageFormat::None;
        uint32_t m_Width = 0, m_Height = 0;

        Buffer m_ImageData;

        VulkanImageInfo m_Info;
        VkDescriptorImageInfo m_DescriptorImageInfo = {};
    };

    namespace Utils
    {
        VkFormat VulkanImageFormat(ImageFormat format);
    }
}
