#pragma once

#include "Prism/Core/Core.h"
#include "Prism/Core/Ref.h"
#include "Prism/Core/Buffer.h"

#include <cmath>
#include <vector>
#include <glm/glm.hpp>

namespace Prism {

    enum class PRISM_API ImageFormat
    {
        None = 0,
        RGB,
        RGBA,
        RGBA16F,
        RGBA32F,
        RG32F,

        SRGB,

        DEPTH32F,
        DEPTH24STENCIL8,

        // Defaults
        Depth = DEPTH24STENCIL8,

        // Block-compressed formats (DDS)
        BC1, BC1SRGB,
        BC2, BC2SRGB,
        BC3, BC3SRGB,
        BC4,
        BC5,
        BC6H,
        BC7, BC7SRGB
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
                case ImageFormat::BC1SRGB:
                case ImageFormat::BC4:
                    return 8;
                case ImageFormat::BC2:
                case ImageFormat::BC2SRGB:
                case ImageFormat::BC3:
                case ImageFormat::BC3SRGB:
                case ImageFormat::BC5:
                case ImageFormat::BC6H:
                case ImageFormat::BC7:
                case ImageFormat::BC7SRGB:
                    return 16;
            }
            PR_CORE_ASSERT(false, "Not a compressed format");
            return 0;
        }

        inline uint32_t GetImageFormatBPP(ImageFormat format)
        {
            switch (format)
            {
                case ImageFormat::RGB:
                case ImageFormat::SRGB:    return 3;
                case ImageFormat::RGBA:    return 4;
                case ImageFormat::RGBA16F: return 2 * 4;
                case ImageFormat::RGBA32F: return 4 * 4;
                case ImageFormat::RG32F:   return 2 * 4;

                case ImageFormat::BC1:
                case ImageFormat::BC1SRGB:
                case ImageFormat::BC2:
                case ImageFormat::BC2SRGB:
                case ImageFormat::BC3:
                case ImageFormat::BC3SRGB:
                case ImageFormat::BC4:
                case ImageFormat::BC5:
                case ImageFormat::BC6H:
                case ImageFormat::BC7:
                case ImageFormat::BC7SRGB: return 0;
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
