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

        virtual void Bind(uint32_t slot = 0) const override;
        virtual void BindImage(uint32_t slot, TextureAccess access, bool layered = true, uint32_t mipLevel = 0) const override;
        virtual uint32_t GetBinding() const override { return m_BindSlot; }

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

        virtual RendererID GetRendererID() const override { return m_Image.As<OpenGLImage2D>()->GetRendererID(); }

        virtual Ref<Image2D> GetImage() const override { return m_Image; }
        virtual uint64_t GetHash() const override { return m_Image->GetHash(); }


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
        OpenGLTextureCube(const std::string& path);
        virtual ~OpenGLTextureCube();

        virtual void Bind(uint32_t slot = 0) const;
        virtual void BindImage(uint32_t slot, TextureAccess access, bool layered = true, uint32_t mipLevel = 0) const override;
        virtual uint32_t GetBinding() const override { return m_BindSlot; }

        virtual ImageFormat GetFormat() const { return m_Format; }
        virtual uint32_t GetWidth() const { return m_Width; }
        virtual uint32_t GetHeight() const { return m_Height; }
        // This function currently returns the expected number of mips based on image size,
        // not present mips in data
        virtual uint32_t GetMipLevelCount() const override;

        virtual void GenerateMipMap() const override;
        virtual void CopyTo(Ref<TextureCube> destination) const override;

        virtual const std::string& GetPath() const override { return FilePath; }

        virtual RendererID GetRendererID() const override { return m_RendererID; }

        virtual uint64_t GetHash() const override { return (uint64_t)m_RendererID; }

    private:
        RendererID m_RendererID;
        mutable uint32_t m_BindSlot = 0;
        ImageFormat m_Format;
        uint32_t m_Width, m_Height;

        byte* m_ImageData;

    };
}
