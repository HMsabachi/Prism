#pragma once

#include "Prism/Renderer/Image.h"
#include "Prism/Renderer/RendererTypes.h"
#include <glad/glad.h>

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

        // Base external formats
        constexpr GLenum P_GL_RED = 0x1903;
        constexpr GLenum P_GL_RG = 0x8227;
        constexpr GLenum P_GL_RGB = 0x1907;
        constexpr GLenum P_GL_RGBA = 0x1908;
        constexpr GLenum P_GL_RED_INTEGER = 0x8D94;
        constexpr GLenum P_GL_RG_INTEGER = 0x8228;
        constexpr GLenum P_GL_RGBA_INTEGER = 0x8D99;
        constexpr GLenum P_GL_DEPTH_COMPONENT = 0x1902;
        constexpr GLenum P_GL_DEPTH_STENCIL = 0x84F9;

        // Internal formats
        constexpr GLenum P_GL_R8 = 0x8229;
        constexpr GLenum P_GL_RG8 = 0x822B;
        constexpr GLenum P_GL_RGB8 = 0x8051;
        constexpr GLenum P_GL_RGBA8 = 0x8058;
        constexpr GLenum P_GL_R8_SNORM = 0x8F94;
        constexpr GLenum P_GL_RG8_SNORM = 0x8F95;
        constexpr GLenum P_GL_RGB8_SNORM = 0x8F96;
        constexpr GLenum P_GL_RGBA8_SNORM = 0x8F97;
        constexpr GLenum P_GL_SR8 = 0x8FBD;        // GL_SR8_EXT
        constexpr GLenum P_GL_SRG8 = 0x8FBE;       // GL_SRG8_EXT
        constexpr GLenum P_GL_SRGB8 = 0x8C41;
        constexpr GLenum P_GL_SRGB8_ALPHA8 = 0x8C43;
        constexpr GLenum P_GL_R16F = 0x822D;
        constexpr GLenum P_GL_RG16F = 0x822F;
        constexpr GLenum P_GL_RGB16F = 0x881B;
        constexpr GLenum P_GL_RGBA16F = 0x881A;
        constexpr GLenum P_GL_R32F = 0x822E;
        constexpr GLenum P_GL_RG32F = 0x8230;
        constexpr GLenum P_GL_RGB32F = 0x8815;
        constexpr GLenum P_GL_RGBA32F = 0x8814;
        constexpr GLenum P_GL_R16UI = 0x8234;
        constexpr GLenum P_GL_RG16UI = 0x823A;
        constexpr GLenum P_GL_RGBA16UI = 0x8D76;
        constexpr GLenum P_GL_R32UI = 0x8236;
        constexpr GLenum P_GL_RG32UI = 0x823C;
        constexpr GLenum P_GL_RGBA32UI = 0x8D70;
        constexpr GLenum P_GL_R16I = 0x8233;
        constexpr GLenum P_GL_RG16I = 0x8239;
        constexpr GLenum P_GL_RGBA16I = 0x8D88;
        constexpr GLenum P_GL_R32I = 0x8235;
        constexpr GLenum P_GL_RG32I = 0x823B;
        constexpr GLenum P_GL_RGBA32I = 0x8D82;

        // Packed internal formats
        constexpr GLenum P_GL_RGB565 = 0x8D62;
        constexpr GLenum P_GL_RGBA4 = 0x8056;
        constexpr GLenum P_GL_RGB5_A1 = 0x8057;
        constexpr GLenum P_GL_RGB10_A2 = 0x8059;
        constexpr GLenum P_GL_R11F_G11F_B10F = 0x8C3A;
        constexpr GLenum P_GL_RGB9_E5 = 0x8C3D;

        // Depth / Stencil internal formats
        constexpr GLenum P_GL_DEPTH_COMPONENT16 = 0x81A5;
        constexpr GLenum P_GL_DEPTH24_STENCIL8 = 0x88F0;
        constexpr GLenum P_GL_DEPTH_COMPONENT32F = 0x8CAC;
        constexpr GLenum P_GL_DEPTH32F_STENCIL8 = 0x8CAD;

        // Data types
        constexpr GLenum P_GL_UNSIGNED_BYTE = 0x1401;
        constexpr GLenum P_GL_UNSIGNED_SHORT = 0x1403;
        constexpr GLenum P_GL_SHORT = 0x1402;
        constexpr GLenum P_GL_UNSIGNED_INT = 0x1405;
        constexpr GLenum P_GL_INT = 0x1404;
        constexpr GLenum P_GL_HALF_FLOAT = 0x140B;
        constexpr GLenum P_GL_FLOAT = 0x1406;
        constexpr GLenum P_GL_UNSIGNED_SHORT_5_6_5 = 0x8363;
        constexpr GLenum P_GL_UNSIGNED_SHORT_4_4_4_4 = 0x8033;
        constexpr GLenum P_GL_UNSIGNED_SHORT_5_5_5_1 = 0x8034;
        constexpr GLenum P_GL_UNSIGNED_INT_2_10_10_10_REV = 0x8368;
        constexpr GLenum P_GL_UNSIGNED_INT_10F_11F_11F_REV = 0x8C3B;
        constexpr GLenum P_GL_UNSIGNED_INT_5_9_9_9_REV = 0x8C3E;
        constexpr GLenum P_GL_UNSIGNED_INT_24_8 = 0x84FA;
        constexpr GLenum P_GL_FLOAT_32_UNSIGNED_INT_24_8_REV = 0x8DAD;

        // Block-compressed (BC)
        constexpr GLenum P_GL_COMPRESSED_RGBA_S3TC_DXT1_EXT = 0x83F1;       // BC1
        constexpr GLenum P_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT = 0x8C4D; // BC1 sRGB
        constexpr GLenum P_GL_COMPRESSED_RGBA_S3TC_DXT3_EXT = 0x83F2;       // BC2
        constexpr GLenum P_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT = 0x8C4E; // BC2 sRGB
        constexpr GLenum P_GL_COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3;       // BC3
        constexpr GLenum P_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT = 0x8C4F; // BC3 sRGB
        constexpr GLenum P_GL_COMPRESSED_RED_RGTC1 = 0x8DBB;                // BC4
        constexpr GLenum P_GL_COMPRESSED_RG_RGTC2 = 0x8DBD;                 // BC5
        constexpr GLenum P_GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT = 0x8E8F;  // BC6H UF16
        constexpr GLenum P_GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT = 0x8E8E;    // BC6H SF16
        constexpr GLenum P_GL_COMPRESSED_RGBA_BPTC_UNORM = 0x8E8C;          // BC7
        constexpr GLenum P_GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM = 0x8E8D;    // BC7 sRGB

        // ETC2
        constexpr GLenum P_GL_COMPRESSED_RGB8_ETC2 = 0x9274;
        constexpr GLenum P_GL_COMPRESSED_SRGB8_ETC2 = 0x9275;
        constexpr GLenum P_GL_COMPRESSED_RGBA8_ETC2_EAC = 0x9278;
        constexpr GLenum P_GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC = 0x9279;

        // ASTC
        constexpr GLenum P_GL_COMPRESSED_RGBA_ASTC_4x4_KHR = 0x93B0;
        constexpr GLenum P_GL_COMPRESSED_RGBA_ASTC_5x5_KHR = 0x93B1;
        constexpr GLenum P_GL_COMPRESSED_RGBA_ASTC_6x6_KHR = 0x93B2;
        constexpr GLenum P_GL_COMPRESSED_RGBA_ASTC_8x8_KHR = 0x93B5;

        inline GLenum OpenGLImageFormat(ImageFormat format)
        {
            switch (format)
            {
                case ImageFormat::R8:
                case ImageFormat::R8_SRGB:
                case ImageFormat::R8_SNORM:
                case ImageFormat::R16F:
                case ImageFormat::R32F:      return P_GL_RED;

                case ImageFormat::RG8:
                case ImageFormat::RG8_SRGB:
                case ImageFormat::RG8_SNORM:
                case ImageFormat::RG16F:
                case ImageFormat::RG32F:     return P_GL_RG;

                case ImageFormat::RGB8:
                case ImageFormat::RGB8_SRGB:
                case ImageFormat::RGB8_SNORM:
                case ImageFormat::RGB16F:
                case ImageFormat::RGB32F:
                case ImageFormat::RGB565:
                case ImageFormat::RG11B10F:
                case ImageFormat::RGB9E5:    return P_GL_RGB;

                case ImageFormat::RGBA8:
                case ImageFormat::RGBA8_SRGB:
                case ImageFormat::RGBA8_SNORM:
                case ImageFormat::RGBA16F:
                case ImageFormat::RGBA32F:
                case ImageFormat::RGBA4:
                case ImageFormat::RGB5A1:
                case ImageFormat::RGB10A2:   return P_GL_RGBA;

                case ImageFormat::R16_UINT:
                case ImageFormat::R32_UINT:
                case ImageFormat::R16_SINT:
                case ImageFormat::R32_SINT:    return P_GL_RED_INTEGER;

                case ImageFormat::RG16_UINT:
                case ImageFormat::RG32_UINT:
                case ImageFormat::RG16_SINT:
                case ImageFormat::RG32_SINT:   return P_GL_RG_INTEGER;

                case ImageFormat::RGBA16_UINT:
                case ImageFormat::RGBA32_UINT:
                case ImageFormat::RGBA16_SINT:
                case ImageFormat::RGBA32_SINT: return P_GL_RGBA_INTEGER;

                case ImageFormat::DEPTH16:
                case ImageFormat::DEPTH32F:         return P_GL_DEPTH_COMPONENT;
                case ImageFormat::DEPTH24STENCIL8:
                case ImageFormat::DEPTH32FSTENCIL8: return P_GL_DEPTH_STENCIL;
            }
            PR_CORE_ASSERT(false, "Unknown image format");
            return 0;
        }

        inline GLenum OpenGLImageInternalFormat(ImageFormat format)
        {
            switch (format)
            {
                case ImageFormat::R8:         return P_GL_R8;
                case ImageFormat::RG8:        return P_GL_RG8;
                case ImageFormat::RGB8:       return P_GL_RGB8;
                case ImageFormat::RGBA8:      return P_GL_RGBA8;

                case ImageFormat::R8_SRGB:    return P_GL_SR8;
                case ImageFormat::RG8_SRGB:   return P_GL_SRG8;
                case ImageFormat::RGB8_SRGB:  return P_GL_SRGB8;
                case ImageFormat::RGBA8_SRGB: return P_GL_SRGB8_ALPHA8;

                case ImageFormat::R8_SNORM:    return P_GL_R8_SNORM;
                case ImageFormat::RG8_SNORM:   return P_GL_RG8_SNORM;
                case ImageFormat::RGB8_SNORM:  return P_GL_RGB8_SNORM;
                case ImageFormat::RGBA8_SNORM: return P_GL_RGBA8_SNORM;

                case ImageFormat::R16F:    return P_GL_R16F;
                case ImageFormat::RG16F:   return P_GL_RG16F;
                case ImageFormat::RGB16F:  return P_GL_RGB16F;
                case ImageFormat::RGBA16F: return P_GL_RGBA16F;

                case ImageFormat::R32F:    return P_GL_R32F;
                case ImageFormat::RG32F:   return P_GL_RG32F;
                case ImageFormat::RGB32F:  return P_GL_RGB32F;
                case ImageFormat::RGBA32F: return P_GL_RGBA32F;

                case ImageFormat::R16_UINT:    return P_GL_R16UI;
                case ImageFormat::RG16_UINT:   return P_GL_RG16UI;
                case ImageFormat::RGBA16_UINT: return P_GL_RGBA16UI;
                case ImageFormat::R32_UINT:    return P_GL_R32UI;
                case ImageFormat::RG32_UINT:   return P_GL_RG32UI;
                case ImageFormat::RGBA32_UINT: return P_GL_RGBA32UI;

                case ImageFormat::R16_SINT:    return P_GL_R16I;
                case ImageFormat::RG16_SINT:   return P_GL_RG16I;
                case ImageFormat::RGBA16_SINT: return P_GL_RGBA16I;
                case ImageFormat::R32_SINT:    return P_GL_R32I;
                case ImageFormat::RG32_SINT:   return P_GL_RG32I;
                case ImageFormat::RGBA32_SINT: return P_GL_RGBA32I;

                case ImageFormat::RGB565:   return P_GL_RGB565;
                case ImageFormat::RGBA4:    return P_GL_RGBA4;
                case ImageFormat::RGB5A1:   return P_GL_RGB5_A1;
                case ImageFormat::RGB10A2:  return P_GL_RGB10_A2;
                case ImageFormat::RG11B10F: return P_GL_R11F_G11F_B10F;
                case ImageFormat::RGB9E5:   return P_GL_RGB9_E5;

                case ImageFormat::DEPTH16:          return P_GL_DEPTH_COMPONENT16;
                case ImageFormat::DEPTH24STENCIL8:  return P_GL_DEPTH24_STENCIL8;
                case ImageFormat::DEPTH32F:         return P_GL_DEPTH_COMPONENT32F;
                case ImageFormat::DEPTH32FSTENCIL8: return P_GL_DEPTH32F_STENCIL8;

                case ImageFormat::BC1:       return P_GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                case ImageFormat::BC1_SRGB:  return P_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
                case ImageFormat::BC2:       return P_GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
                case ImageFormat::BC2_SRGB:  return P_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
                case ImageFormat::BC3:       return P_GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                case ImageFormat::BC3_SRGB:  return P_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
                case ImageFormat::BC4:       return P_GL_COMPRESSED_RED_RGTC1;
                case ImageFormat::BC5:       return P_GL_COMPRESSED_RG_RGTC2;
                case ImageFormat::BC6H_UF16: return P_GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
                case ImageFormat::BC6H_SF16: return P_GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
                case ImageFormat::BC7:       return P_GL_COMPRESSED_RGBA_BPTC_UNORM;
                case ImageFormat::BC7_SRGB:  return P_GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;

                case ImageFormat::ETC2_RGB8:       return P_GL_COMPRESSED_RGB8_ETC2;
                case ImageFormat::ETC2_RGB8_SRGB:  return P_GL_COMPRESSED_SRGB8_ETC2;
                case ImageFormat::ETC2_RGBA8:      return P_GL_COMPRESSED_RGBA8_ETC2_EAC;
                case ImageFormat::ETC2_RGBA8_SRGB: return P_GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;

                case ImageFormat::ASTC_4x4: return P_GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
                case ImageFormat::ASTC_5x5: return P_GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
                case ImageFormat::ASTC_6x6: return P_GL_COMPRESSED_RGBA_ASTC_6x6_KHR;
                case ImageFormat::ASTC_8x8: return P_GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
            }
            PR_CORE_ASSERT(false, "Unknown image format");
            return 0;
        }

        inline GLenum OpenGLFormatDataType(ImageFormat format)
        {
            switch (format)
            {
                case ImageFormat::R8:
                case ImageFormat::RG8:
                case ImageFormat::RGB8:
                case ImageFormat::RGBA8:
                case ImageFormat::R8_SRGB:
                case ImageFormat::RG8_SRGB:
                case ImageFormat::RGB8_SRGB:
                case ImageFormat::RGBA8_SRGB:
                case ImageFormat::R8_SNORM:
                case ImageFormat::RG8_SNORM:
                case ImageFormat::RGB8_SNORM:
                case ImageFormat::RGBA8_SNORM: return P_GL_UNSIGNED_BYTE;

                case ImageFormat::R16F:
                case ImageFormat::RG16F:
                case ImageFormat::RGB16F:
                case ImageFormat::RGBA16F: return P_GL_HALF_FLOAT;

                case ImageFormat::R32F:
                case ImageFormat::RG32F:
                case ImageFormat::RGB32F:
                case ImageFormat::RGBA32F: return P_GL_FLOAT;

                case ImageFormat::R16_UINT:
                case ImageFormat::RG16_UINT:
                case ImageFormat::RGBA16_UINT: return P_GL_UNSIGNED_SHORT;
                case ImageFormat::R32_UINT:
                case ImageFormat::RG32_UINT:
                case ImageFormat::RGBA32_UINT: return P_GL_UNSIGNED_INT;

                case ImageFormat::R16_SINT:
                case ImageFormat::RG16_SINT:
                case ImageFormat::RGBA16_SINT: return P_GL_SHORT;
                case ImageFormat::R32_SINT:
                case ImageFormat::RG32_SINT:
                case ImageFormat::RGBA32_SINT: return P_GL_INT;

                case ImageFormat::RGB565:   return P_GL_UNSIGNED_SHORT_5_6_5;
                case ImageFormat::RGBA4:    return P_GL_UNSIGNED_SHORT_4_4_4_4;
                case ImageFormat::RGB5A1:   return P_GL_UNSIGNED_SHORT_5_5_5_1;
                case ImageFormat::RGB10A2:  return P_GL_UNSIGNED_INT_2_10_10_10_REV;
                case ImageFormat::RG11B10F: return P_GL_UNSIGNED_INT_10F_11F_11F_REV;
                case ImageFormat::RGB9E5:   return P_GL_UNSIGNED_INT_5_9_9_9_REV;

                case ImageFormat::DEPTH16:          return P_GL_UNSIGNED_SHORT;
                case ImageFormat::DEPTH24STENCIL8:  return P_GL_UNSIGNED_INT_24_8;
                case ImageFormat::DEPTH32F:         return P_GL_FLOAT;
                case ImageFormat::DEPTH32FSTENCIL8: return P_GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
            }
            PR_CORE_ASSERT(false, "Unknown image format");
            return 0;
        }

    }

}
