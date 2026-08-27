#pragma once

#include "Prism/Renderer/RendererAPI.h"
#include "Prism/Renderer/Texture.h"
#include "Platform/OpenGL/OpenGLImage.h"

namespace Prism {

    class PRISM_API OpenGLTexture2D : public Texture2D
    {
    public:
        OpenGLTexture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr);

        OpenGLTexture2D(const std::string& path, bool srgb);
        virtual ~OpenGLTexture2D();

        virtual ImageFormat GetFormat() const override { return m_Image->GetFormat(); }
        virtual uint32_t GetWidth() const override { return m_Width; }
        virtual uint32_t GetHeight() const override { return m_Height; }
        // This function currently returns the expected number of mips based on image size,
        // not present mips in data
        virtual uint32_t GetMipLevelCount() const override;

        virtual const std::string& GetPath() const override { return FilePath; }

        virtual bool Loaded() const override { return m_Loaded; }

        virtual void Lock() override;
        virtual void Unlock() override;

        virtual Buffer GetWriteableBuffer() override;
        virtual Ref<Image2D> GetImage() const override { return m_Image; }
        virtual uint64_t GetHash() const override { return m_Image->GetHash(); }

        void RT_Bind(uint32_t slot) const;
        void RT_Init(bool mipmapSampler);

        RendererID GetRendererID() const { return m_Image.As<OpenGLImage2D>()->GetRendererID(); }
        uint32_t GetBinding() const { return m_BindSlot; }



    private:
        Ref<Image2D> m_Image;
        mutable uint32_t m_BindSlot = 0;
        TextureWrap m_Wrap = TextureWrap::Clamp;
        uint32_t m_Width, m_Height;

        bool m_IsHDR = false;

        bool m_Locked = false;
        bool m_Loaded = false;

    };

    class PRISM_API OpenGLTextureCube : public TextureCube
    {
    public:
        OpenGLTextureCube(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr);
        virtual ~OpenGLTextureCube();

        virtual ImageFormat GetFormat() const override { return m_Image->GetFormat(); }
        virtual uint32_t GetWidth() const override { return m_Image->GetWidth(); }
        virtual uint32_t GetHeight() const override { return m_Image->GetHeight(); }
        virtual uint32_t GetMipLevelCount() const override;

        void GenerateMipMap() const;
        void CopyTo(Ref<TextureCube> destination) const;

        virtual const std::string& GetPath() const override { return FilePath; }
        virtual uint64_t GetHash() const override { return m_Image->GetHash(); }
        virtual Ref<ImageCube> GetImage() const override { return m_Image; }

        void RT_Bind(uint32_t slot) const;

        RendererID GetRendererID() const { return m_Image.As<OpenGLImageCube>()->GetRendererID(); }
        uint32_t GetBinding() const { return m_BindSlot; }
    private:
        Ref<ImageCube> m_Image;
        mutable uint32_t m_BindSlot = 0;
    };
}
