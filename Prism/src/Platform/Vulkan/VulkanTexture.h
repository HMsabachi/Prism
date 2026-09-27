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
        VulkanTexture2D(const TextureSpecification& specification, Buffer imageData = Buffer());
        VulkanTexture2D(const TextureSpecification& specification, const std::string& path);
        virtual ~VulkanTexture2D();

        void Invalidate();

        virtual ImageFormat GetFormat() const override { return m_Specification.Format; }
        virtual uint32_t GetWidth() const override { return m_Specification.Width; }
        virtual uint32_t GetHeight() const override { return m_Specification.Height; }
        virtual uint32_t GetMipLevelCount() const override;

        virtual Ref<Image2D> GetImage() const override { return m_Image; }

        virtual void Lock() override;
        virtual void Unlock() override;

        virtual Buffer GetWriteableBuffer() override;

        virtual bool Loaded() const override { return m_Loaded; }
        virtual const std::string& GetPath() const override { return m_Path; }


        const VkDescriptorImageInfo& GetVulkanDescriptorInfo() const { return m_Image.As<VulkanImage2D>()->GetDescriptor(); }
    private:
        std::string m_Path;
        TextureSpecification m_Specification;

        Ref<Image2D> m_Image;

        bool m_Loaded = false;
    };

    class PRISM_API VulkanTextureCube : public TextureCube
    {
    public:
        VulkanTextureCube(const TextureSpecification& specification, Buffer imageData = Buffer());
        virtual ~VulkanTextureCube();

        virtual const std::string& GetPath() const override { return m_Path; }

        virtual ImageFormat GetFormat() const override { return m_Specification.Format; }
        virtual uint32_t GetWidth() const override { return m_Specification.Width; }
        virtual uint32_t GetHeight() const override { return m_Specification.Height; }
        virtual uint32_t GetMipLevelCount() const override;

        virtual Ref<ImageCube> GetImage() const override { return m_Image; }

        const VkDescriptorImageInfo& GetVulkanDescriptorInfo() const { return m_Image.As<VulkanImageCube>()->GetDescriptor(); }

    private:
        void Invalidate();
    private:
        std::string m_Path;
        TextureSpecification m_Specification;

        Ref<ImageCube> m_Image;
    };
}
