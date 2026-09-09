#pragma once

#include "Prism/Core/Core.h"
#include "Prism/Core/Ref.h"
#include "Prism/Core/Buffer.h"

#include <cmath>
#include <vector>
#include <glm/glm.hpp>

namespace Prism {

    enum class ImageFormat
    {
        None = 0,

        // 8-bit
        R8,
        RG8,
        RGB8,
        RGBA8,

        R8_SRGB,
        RG8_SRGB,
        RGB8_SRGB,
        RGBA8_SRGB,

        R8_SNORM,
        RG8_SNORM,
        RGB8_SNORM,
        RGBA8_SNORM,

        // 16-bit Float
        R16F,
        RG16F,
        RGB16F,
        RGBA16F,

        // 32-bit Float
        R32F,
        RG32F,
        RGB32F,
        RGBA32F,

        // Integer
        R16_UINT,
        RG16_UINT,
        RGBA16_UINT,

        R32_UINT,
        RG32_UINT,
        RGBA32_UINT,

        R16_SINT,
        RG16_SINT,
        RGBA16_SINT,

        R32_SINT,
        RG32_SINT,
        RGBA32_SINT,

        // Packed
        RGB565,
        RGBA4,
        RGB5A1,
        RGB10A2,
        RG11B10F,
        RGB9E5,

        // Depth / Stencil
        DEPTH16,
        DEPTH24STENCIL8,
        DEPTH32F,
        DEPTH32FSTENCIL8,

        // BC
        BC1,
        BC1_SRGB,
        BC2,
        BC2_SRGB,
        BC3,
        BC3_SRGB,
        BC4,
        BC5,
        BC6H_UF16,
        BC6H_SF16,
        BC7,
        BC7_SRGB,

        // ETC2
        ETC2_RGB8,
        ETC2_RGB8_SRGB,
        ETC2_RGBA8,
        ETC2_RGBA8_SRGB,

        // ASTC
        ASTC_4x4,
        ASTC_5x5,
        ASTC_6x6,
        ASTC_8x8,

        // Defaults
        Depth = DEPTH24STENCIL8
    };

    class PRISM_API Image : public RefCounted
    {
    public:
        virtual ~Image() {}

        virtual void Invalidate() = 0;
        virtual void Release() = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual uint32_t GetSamples() const = 0;

        virtual ImageFormat GetFormat() const = 0;

        virtual Buffer GetBuffer() const = 0;
        virtual Buffer& GetBuffer() = 0;
    };

    class PRISM_API Image2D : public Image
    {
        virtual void Resize(const uint32_t width, const uint32_t height) = 0;
    public:
        static Ref<Image2D> Create(ImageFormat format, uint32_t width, uint32_t height, Buffer buffer, uint32_t samples = 1);
        static Ref<Image2D> Create(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr, uint32_t samples = 1);
        static Ref<Image2D> Create(ImageFormat format, uint32_t width, uint32_t height, std::vector<Buffer>&& mips);
    };

    class PRISM_API ImageCube : public Image
    {
    public:
        virtual void GenerateMipMap() = 0;
        virtual void CopyTo(Ref<ImageCube> destination) const = 0;

    public:
        static Ref<ImageCube> Create(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr);
    };

    namespace Utils {

        inline bool IsCompressedFormat(ImageFormat format)
        {
            return format >= ImageFormat::BC1;
        }

        inline uint32_t GetCompressedBlockSize(ImageFormat format)
        {
            switch (format)
            {
                case ImageFormat::BC1:
                case ImageFormat::BC1_SRGB:
                case ImageFormat::BC4:
                    return 8;
                case ImageFormat::BC2:
                case ImageFormat::BC2_SRGB:
                case ImageFormat::BC3:
                case ImageFormat::BC3_SRGB:
                case ImageFormat::BC5:
                case ImageFormat::BC6H_UF16:
                case ImageFormat::BC6H_SF16:
                case ImageFormat::BC7:
                case ImageFormat::BC7_SRGB:
                    return 16;
                case ImageFormat::ETC2_RGB8:
                case ImageFormat::ETC2_RGB8_SRGB:
                case ImageFormat::ETC2_RGBA8:
                case ImageFormat::ETC2_RGBA8_SRGB:
                    return 8;
                case ImageFormat::ASTC_4x4:
                case ImageFormat::ASTC_5x5:
                case ImageFormat::ASTC_6x6:
                case ImageFormat::ASTC_8x8:
                    return 16;
            }
            PR_CORE_ASSERT(false, "Not a compressed format");
            return 0;
        }

        inline uint32_t GetImageFormatBPP(ImageFormat format)
        {
            switch (format)
            {
                // 8-bit
                case ImageFormat::R8:    return 1;
                case ImageFormat::RG8:   return 2;
                case ImageFormat::RGB8:  return 3;
                case ImageFormat::RGBA8: return 4;

                case ImageFormat::R8_SRGB:    return 1;
                case ImageFormat::RG8_SRGB:   return 2;
                case ImageFormat::RGB8_SRGB:  return 3;
                case ImageFormat::RGBA8_SRGB: return 4;

                case ImageFormat::R8_SNORM:    return 1;
                case ImageFormat::RG8_SNORM:   return 2;
                case ImageFormat::RGB8_SNORM:  return 3;
                case ImageFormat::RGBA8_SNORM: return 4;

                // 16-bit Float
                case ImageFormat::R16F:    return 2;
                case ImageFormat::RG16F:   return 4;
                case ImageFormat::RGB16F:  return 6;
                case ImageFormat::RGBA16F: return 8;

                // 32-bit Float
                case ImageFormat::R32F:    return 4;
                case ImageFormat::RG32F:   return 8;
                case ImageFormat::RGB32F:  return 12;
                case ImageFormat::RGBA32F: return 16;

                // Integer
                case ImageFormat::R16_UINT:    return 2;
                case ImageFormat::RG16_UINT:   return 4;
                case ImageFormat::RGBA16_UINT: return 8;
                case ImageFormat::R32_UINT:    return 4;
                case ImageFormat::RG32_UINT:   return 8;
                case ImageFormat::RGBA32_UINT: return 16;

                case ImageFormat::R16_SINT:    return 2;
                case ImageFormat::RG16_SINT:   return 4;
                case ImageFormat::RGBA16_SINT: return 8;
                case ImageFormat::R32_SINT:    return 4;
                case ImageFormat::RG32_SINT:   return 8;
                case ImageFormat::RGBA32_SINT: return 16;

                // Packed
                case ImageFormat::RGB565:   return 2;
                case ImageFormat::RGBA4:    return 2;
                case ImageFormat::RGB5A1:   return 2;
                case ImageFormat::RGB10A2:  return 4;
                case ImageFormat::RG11B10F: return 4;
                case ImageFormat::RGB9E5:   return 4;

                // Depth / Stencil
                case ImageFormat::DEPTH16:          return 2;
                case ImageFormat::DEPTH24STENCIL8:  return 4;
                case ImageFormat::DEPTH32F:         return 4;
                case ImageFormat::DEPTH32FSTENCIL8: return 8;

                // Block-compressed
                case ImageFormat::BC1:
                case ImageFormat::BC1_SRGB:
                case ImageFormat::BC2:
                case ImageFormat::BC2_SRGB:
                case ImageFormat::BC3:
                case ImageFormat::BC3_SRGB:
                case ImageFormat::BC4:
                case ImageFormat::BC5:
                case ImageFormat::BC6H_UF16:
                case ImageFormat::BC6H_SF16:
                case ImageFormat::BC7:
                case ImageFormat::BC7_SRGB:
                case ImageFormat::ETC2_RGB8:
                case ImageFormat::ETC2_RGB8_SRGB:
                case ImageFormat::ETC2_RGBA8:
                case ImageFormat::ETC2_RGBA8_SRGB:
                case ImageFormat::ASTC_4x4:
                case ImageFormat::ASTC_5x5:
                case ImageFormat::ASTC_6x6:
                case ImageFormat::ASTC_8x8: return 0;
            }
            PR_CORE_ASSERT(false, "Unknown image format");
            return 0;
        }

        inline uint32_t CalculateMipCount(uint32_t width, uint32_t height)
        {
            return (uint32_t)std::floor(std::log2(glm::min(width, height))) + 1;
        }

        inline uint32_t GetImageMemorySize(ImageFormat format, uint32_t width, uint32_t height)
        {
            return width * height * GetImageFormatBPP(format);
        }

    }

}
