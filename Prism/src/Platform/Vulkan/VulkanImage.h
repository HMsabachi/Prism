#pragma once

#include "Prism/Renderer/Image.h"
#include "Prism/Renderer/Texture.h"
#include "Platform/Vulkan/Vulkan.h"

#include "VulkanMemoryAllocator/vk_mem_alloc.h"

#include <map>

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
        VulkanImage2D(ImageFormat format, uint32_t width, uint32_t height, std::vector<Buffer>&& mips);
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

        VulkanImageInfo& GetImageInfo() { return m_Info; }
        const VulkanImageInfo& GetImageInfo() const { return m_Info; }

        const VkDescriptorImageInfo& GetDescriptor() const { return m_DescriptorImageInfo; }

        void SetSamplerWrap(TextureWrap wrap) { m_Wrap = wrap; }
        void SetExtraUsage(VkImageUsageFlags extraUsage) { m_ExtraUsage = extraUsage; }

        void RT_Resize(const uint32_t width, const uint32_t height);
        void RT_Invalidate();
        void RT_GenerateMips();
        void UpdateDescriptor();
        VkImageView GetOrCreateStorageImageView(uint32_t mip);
    private:
        ImageFormat m_Format = ImageFormat::None;
        uint32_t m_Width = 0, m_Height = 0;
        uint32_t m_Samples = 1;
        TextureWrap m_Wrap = TextureWrap::Repeat;
        VkImageUsageFlags m_ExtraUsage = 0;

        Buffer m_ImageData;
        std::vector<Buffer> m_Mips; // DDS 预压缩 mip 链，含 level 0

        VulkanImageInfo m_Info;
        VkDescriptorImageInfo m_DescriptorImageInfo = {};
        std::map<uint32_t, VkImageView> m_StorageViews;
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

        virtual void GenerateMipMap() override;
        virtual void CopyTo(Ref<ImageCube> destination) const override;

        VulkanImageInfo& GetImageInfo() { return m_Info; }
        const VulkanImageInfo& GetImageInfo() const { return m_Info; }

        const VkDescriptorImageInfo& GetDescriptor() const { return m_DescriptorImageInfo; }

        void RT_GenerateMips(bool readonly = false);
        void UpdateDescriptor();
        VkImageView GetOrCreateStorageImageView(uint32_t mip);
    private:
        ImageFormat m_Format = ImageFormat::None;
        uint32_t m_Width = 0, m_Height = 0;

        Buffer m_ImageData;

        VulkanImageInfo m_Info;
        VkDescriptorImageInfo m_DescriptorImageInfo = {};
        std::map<uint32_t, VkImageView> m_StorageViews;
    };

    namespace Utils
    {
        VkFormat VulkanImageFormat(ImageFormat format);
    }
}
