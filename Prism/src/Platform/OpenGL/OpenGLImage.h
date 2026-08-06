#pragma once

#include "Prism/Renderer/Image.h"
#include "Prism/Renderer/RendererTypes.h"

namespace Prism {

    class PRISM_API OpenGLImage2D : public Image2D
    {
    public:
        OpenGLImage2D(ImageFormat format, uint32_t width, uint32_t height, Buffer buffer, uint32_t samples = 1);
        OpenGLImage2D(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr, uint32_t samples = 1);
        virtual ~OpenGLImage2D();

        virtual void Invalidate() override;
        virtual void Release() override;

        virtual ImageFormat GetFormat() const override { return m_Format; }
        virtual uint32_t GetWidth() const override { return m_Width; }
        virtual uint32_t GetHeight() const override { return m_Height; }
        virtual uint32_t GetSamples() const override { return m_Samples; }

        virtual Buffer GetBuffer() const override { return m_ImageData; }
        virtual Buffer& GetBuffer() override { return m_ImageData; }

        void Bind(uint32_t slot) const;

        RendererID& GetRendererID() { return m_RendererID; }
        RendererID GetRendererID() const { return m_RendererID; }

        RendererID& GetSamplerRendererID() { return m_SamplerRendererID; }
        RendererID GetSamplerRendererID() const { return m_SamplerRendererID; }

        virtual uint64_t GetHash() const override { return (uint64_t)m_RendererID; }
    private:
        RendererID m_RendererID = 0;
        RendererID m_SamplerRendererID = 0;
        uint32_t m_Width, m_Height;
        uint32_t m_Samples = 1;
        ImageFormat m_Format;

        Buffer m_ImageData;
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

        void Bind(uint32_t slot) const;

        RendererID& GetRendererID() { return m_RendererID; }
        RendererID GetRendererID() const { return m_RendererID; }

        virtual uint64_t GetHash() const override { return (uint64_t)m_RendererID; }
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
        constexpr GLenum P_GL_DEPTH24_STENCIL8 = 0x88F0;
        constexpr GLenum P_GL_DEPTH_COMPONENT32F = 0x8CAC;
        constexpr GLenum P_GL_UNSIGNED_BYTE = 0x1401;
        constexpr GLenum P_GL_FLOAT = 0x1406;

        inline GLenum OpenGLImageFormat(ImageFormat format)
        {
            switch (format)
            {
                case ImageFormat::RGB:     return P_GL_RGB;
                case ImageFormat::SRGB:    return P_GL_RGB;
                case ImageFormat::RGBA:
                case ImageFormat::RGBA16F:
                case ImageFormat::RGBA32F: return P_GL_RGBA;
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
                case ImageFormat::DEPTH24STENCIL8: return P_GL_DEPTH24_STENCIL8;
                case ImageFormat::DEPTH32F:        return P_GL_DEPTH_COMPONENT32F;
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
