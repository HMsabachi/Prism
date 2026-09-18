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
        VulkanImage2D(const ImageSpecification& specification, Buffer buffer = Buffer());
        VulkanImage2D(const ImageSpecification& specification, std::vector<Buffer>&& mips);
        virtual ~VulkanImage2D();

        virtual void Resize(const uint32_t width, const uint32_t height) override;
        virtual void Invalidate() override;
        virtual void Release() override;

        virtual uint32_t GetWidth() const override { return m_Specification.Width; }
        virtual uint32_t GetHeight() const override { return m_Specification.Height; }
        virtual uint32_t GetSamples() const override { return m_Specification.Samples; }
        virtual ImageFormat GetFormat() const override { return m_Specification.Format; }
        virtual ImageUsage GetUsage() const override { return m_Specification.Usage; }

        virtual Buffer GetBuffer() const override { return m_ImageData; }
        virtual Buffer& GetBuffer() override { return m_ImageData; }

        VulkanImageInfo& GetImageInfo() { return m_Info; }
        const VulkanImageInfo& GetImageInfo() const { return m_Info; }

        const VkDescriptorImageInfo& GetDescriptor() const { return m_DescriptorImageInfo; }

        void SetSamplerWrap(TextureWrap wrap) { m_Wrap = wrap; }

        void RT_Resize(const uint32_t width, const uint32_t height);
        void RT_Invalidate();
        void RT_GenerateMips();
        void UpdateDescriptor();
        VkImageView GetOrCreateStorageImageView(uint32_t mip);
    private:
        ImageSpecification m_Specification;
        TextureWrap m_Wrap = TextureWrap::Repeat;

        Buffer m_ImageData;
        std::vector<Buffer> m_Mips; // DDS 预压缩 mip 链，含 level 0

        VulkanImageInfo m_Info;
        VkDescriptorImageInfo m_DescriptorImageInfo = {};
        std::map<uint32_t, VkImageView> m_StorageViews;
    };

    class PRISM_API VulkanImageCube : public ImageCube
    {
    public:
        VulkanImageCube(const ImageSpecification& specification, Buffer buffer = Buffer());
        virtual ~VulkanImageCube();

        virtual void Invalidate() override;
        virtual void Release() override;

        virtual uint32_t GetWidth() const override { return m_Specification.Width; }
        virtual uint32_t GetHeight() const override { return m_Specification.Height; }
        virtual uint32_t GetSamples() const override { return 1; }
        virtual ImageFormat GetFormat() const override { return m_Specification.Format; }
        virtual ImageUsage GetUsage() const override { return m_Specification.Usage; }

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
        ImageSpecification m_Specification;

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
