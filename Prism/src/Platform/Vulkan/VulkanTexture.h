#pragma once

#include "Prism/Renderer/Texture.h"
#include "Platform/Vulkan/Vulkan.h"

#include "Platform/Vulkan/VulkanImage.h"

#include <string>

namespace Prism
{
    class PRISM_API VulkanTexture2D : public Texture2D
    {
    public:
        VulkanTexture2D(const std::string& path, bool srgb = false);
        VulkanTexture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data, TextureWrap wrap = TextureWrap::Clamp);
        virtual ~VulkanTexture2D();

        void Invalidate();

        virtual ImageFormat GetFormat() const override { return m_Format; }
        virtual uint32_t GetWidth() const override { return m_Width; }
        virtual uint32_t GetHeight() const override { return m_Height; }
        virtual uint32_t GetMipLevelCount() const override;

        virtual Ref<Image2D> GetImage() const override { return m_Image; }

        virtual void Lock() override;
        virtual void Unlock() override;

        virtual Buffer GetWriteableBuffer() override;

        virtual bool Loaded() const override { return m_Loaded; }
        virtual const std::string& GetPath() const override { return m_Path; }

        virtual uint64_t GetHash() const override { return m_Image ? m_Image->GetHash() : 0; }

        const VkDescriptorImageInfo& GetVulkanDescriptorInfo() const { return m_Image.As<VulkanImage2D>()->GetDescriptor(); }
    private:
        std::string m_Path;
        uint32_t m_Width = 0, m_Height = 0;
        TextureWrap m_Wrap = TextureWrap::Clamp;

        Ref<Image2D> m_Image;

        ImageFormat m_Format = ImageFormat::None;
        bool m_Loaded = false;
    };

    class PRISM_API VulkanTextureCube : public TextureCube
    {
    public:
        VulkanTextureCube(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr);
        virtual ~VulkanTextureCube();

        virtual const std::string& GetPath() const override { return m_Path; }

        virtual ImageFormat GetFormat() const override { return m_Format; }
        virtual uint32_t GetWidth() const override { return m_Width; }
        virtual uint32_t GetHeight() const override { return m_Height; }
        virtual uint32_t GetMipLevelCount() const override;

        virtual Ref<ImageCube> GetImage() const override { return m_Image; }

        virtual uint64_t GetHash() const override { return m_Image ? m_Image->GetHash() : 0; }

        const VkDescriptorImageInfo& GetVulkanDescriptorInfo() const { return m_Image.As<VulkanImageCube>()->GetDescriptor(); }

        void CopyTo(Ref<TextureCube> destination) const;
        void GenerateMips(bool readonly = false);
    private:
        void Invalidate();
    private:
        std::string m_Path;
        ImageFormat m_Format = ImageFormat::None;
        uint32_t m_Width = 0, m_Height = 0;

        Ref<ImageCube> m_Image;
    };
}
