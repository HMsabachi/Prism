#pragma once

#include "Prism/Renderer/Image.h"
#include "Prism/Renderer/RendererTypes.h"

namespace Prism {

    class PRISM_API OpenGLImage2D : public Image2D
    {
    public:
        OpenGLImage2D(ImageFormat format, uint32_t width, uint32_t height, Buffer buffer, uint32_t samples = 1);
        OpenGLImage2D(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr, uint32_t samples = 1);
        OpenGLImage2D(ImageFormat format, uint32_t width, uint32_t height, std::vector<Buffer>&& mips);
        virtual ~OpenGLImage2D();

        virtual void Resize(const uint32_t width, const uint32_t height) override;
        virtual void Invalidate() override;
        virtual void Release() override;

        virtual ImageFormat GetFormat() const override { return m_Format; }
        virtual uint32_t GetWidth() const override { return m_Width; }
        virtual uint32_t GetHeight() const override { return m_Height; }
        virtual uint32_t GetSamples() const override { return m_Samples; }

        virtual Buffer GetBuffer() const override { return m_ImageData; }
        virtual Buffer& GetBuffer() override { return m_ImageData; }

        void RT_Resize(const uint32_t width, const uint32_t height);
        void RT_Bind(uint32_t slot) const;

        RendererID& GetRendererID() { return m_RendererID; }
        RendererID GetRendererID() const { return m_RendererID; }

        RendererID& GetSamplerRendererID() { return m_SamplerRendererID; }
        RendererID GetSamplerRendererID() const { return m_SamplerRendererID; }

    private:
        RendererID m_RendererID = 0;
        RendererID m_SamplerRendererID = 0;
        uint32_t m_Width, m_Height;
        uint32_t m_Samples = 1;
        ImageFormat m_Format;

        Buffer m_ImageData;
        std::vector<Buffer> m_Mips; // DDS 预压缩 mip 链，含 level 0
    };

    class PRISM_API OpenGLImageCube : public ImageCube
    {
    public:
        OpenGLImageCube(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr);
        virtual ~OpenGLImageCube();

        virtual void Invalidate() override;
        virtual void Release() override;

        virtual ImageFormat GetFormat() const override { return m_Format; }
        virtual uint32_t GetWidth() const override { return m_Width; }
        virtual uint32_t GetHeight() const override { return m_Height; }
        virtual uint32_t GetSamples() const override { return 1; }

        virtual Buffer GetBuffer() const override { return m_ImageData; }
        virtual Buffer& GetBuffer() override { return m_ImageData; }

        virtual void GenerateMipMap() override;
        virtual void CopyTo(Ref<ImageCube> destination) const override;

        void RT_Bind(uint32_t slot) const;

        RendererID& GetRendererID() { return m_RendererID; }
        RendererID GetRendererID() const { return m_RendererID; }
    private:
        RendererID m_RendererID = 0;
        uint32_t m_Width, m_Height;
        ImageFormat m_Format;

        Buffer m_ImageData;
    };

    namespace Utils {
        typedef unsigned int GLenum;
        constexpr GLenum P_GL_RGB = 0x1907;
        constexpr GLenum P_GL_RGBA = 0x1908;
        constexpr GLenum P_GL_RGB8 = 0x8051;
        constexpr GLenum P_GL_RGBA8 = 0x8058;
        constexpr GLenum P_GL_SRGB8 = 0x8C41;
        constexpr GLenum P_GL_RGBA16F = 0x881A;
        constexpr GLenum P_GL_RGBA32F = 0x8814;
        constexpr GLenum P_GL_RG = 0x8227;
        constexpr GLenum P_GL_RG32F = 0x8230;
        constexpr GLenum P_GL_DEPTH24_STENCIL8 = 0x88F0;
        constexpr GLenum P_GL_DEPTH_COMPONENT32F = 0x8CAC;
        constexpr GLenum P_GL_UNSIGNED_BYTE = 0x1401;
        constexpr GLenum P_GL_FLOAT = 0x1406;

        // block-compressed
        constexpr GLenum P_GL_COMPRESSED_RGBA_S3TC_DXT1_EXT = 0x83F1;       // BC1
        constexpr GLenum P_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT = 0x8C4D; // BC1 sRGB
        constexpr GLenum P_GL_COMPRESSED_RGBA_S3TC_DXT3_EXT = 0x83F2;       // BC2
        constexpr GLenum P_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT = 0x8C4E; // BC2 sRGB
        constexpr GLenum P_GL_COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3;       // BC3
        constexpr GLenum P_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT = 0x8C4F; // BC3 sRGB
        constexpr GLenum P_GL_COMPRESSED_RED_RGTC1 = 0x8DBB;                // BC4
        constexpr GLenum P_GL_COMPRESSED_RG_RGTC2 = 0x8DBD;                 // BC5
        constexpr GLenum P_GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT = 0x8E8F;  // BC6H
        constexpr GLenum P_GL_COMPRESSED_RGBA_BPTC_UNORM = 0x8E8C;          // BC7
        constexpr GLenum P_GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM = 0x8E8D;    // BC7 sRGB

        inline GLenum OpenGLImageFormat(ImageFormat format)
        {
            switch (format)
            {
                case ImageFormat::RGB:     return P_GL_RGB;
                case ImageFormat::SRGB:    return P_GL_RGB;
                case ImageFormat::RGBA:
                case ImageFormat::RGBA16F:
                case ImageFormat::RGBA32F: return P_GL_RGBA;
                case ImageFormat::RG32F:    return P_GL_RG;
            }
            PR_CORE_ASSERT(false, "Unknown image format");
            return 0;
        }

        inline GLenum OpenGLImageInternalFormat(ImageFormat format)
        {
            switch (format)
            {
                case ImageFormat::RGB:             return P_GL_RGB8;
                case ImageFormat::SRGB:            return P_GL_SRGB8;
                case ImageFormat::RGBA:            return P_GL_RGBA8;
                case ImageFormat::RGBA16F:         return P_GL_RGBA16F;
                case ImageFormat::RGBA32F:         return P_GL_RGBA32F;
                case ImageFormat::RG32F:           return P_GL_RG32F;
                case ImageFormat::DEPTH24STENCIL8: return P_GL_DEPTH24_STENCIL8;
                case ImageFormat::DEPTH32F:        return P_GL_DEPTH_COMPONENT32F;
                case ImageFormat::BC1:             return P_GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                case ImageFormat::BC1SRGB:         return P_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
                case ImageFormat::BC2:             return P_GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
                case ImageFormat::BC2SRGB:         return P_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
                case ImageFormat::BC3:             return P_GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                case ImageFormat::BC3SRGB:         return P_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
                case ImageFormat::BC4:             return P_GL_COMPRESSED_RED_RGTC1;
                case ImageFormat::BC5:             return P_GL_COMPRESSED_RG_RGTC2;
                case ImageFormat::BC6H:            return P_GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
                case ImageFormat::BC7:             return P_GL_COMPRESSED_RGBA_BPTC_UNORM;
                case ImageFormat::BC7SRGB:         return P_GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
            }
            PR_CORE_ASSERT(false, "Unknown image format");
            return 0;
        }

        inline GLenum OpenGLFormatDataType(ImageFormat format)
        {
            switch (format)
            {
                case ImageFormat::RGB:
                case ImageFormat::SRGB:
                case ImageFormat::RGBA:    return P_GL_UNSIGNED_BYTE;
                case ImageFormat::RGBA16F:
                case ImageFormat::RGBA32F: return P_GL_FLOAT;
            }
            PR_CORE_ASSERT(false, "Unknown image format");
            return 0;
        }

    }

}
