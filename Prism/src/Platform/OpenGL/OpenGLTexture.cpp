#include "prpch.h"
#include "OpenGLTexture.h"

#include "Prism/Renderer/RendererAPI.h"
#include "Prism/Renderer/Renderer.h"
#include "Prism/Core/RenderThread.h"

#include <glad/glad.h>
#include "stb_image.h"

namespace Prism {

    static GLenum PrismToOpenGLTextureFormat(ImageFormat format)
    {
        switch (format)
        {
        case ImageFormat::RGB:     return GL_RGB;
        case ImageFormat::SRGB:    return GL_SRGB8;
        case ImageFormat::RGBA:    return GL_RGBA;
        case ImageFormat::RGBA16F: return GL_RGBA16F;
        case ImageFormat::RGBA32F: return GL_RGBA32F;
        }
        PR_CORE_ASSERT(false, "Unknown texture format!");
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////////////
    // Texture2D
    //////////////////////////////////////////////////////////////////////////////////

    OpenGLTexture2D::OpenGLTexture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data)
        : m_Width(width), m_Height(height)
    {
        m_Image = Image2D::Create(format, width, height, data);
        // Allocate CPU buffer for Lock/Unlock/GetWriteableBuffer when no initial data
        // (callers Lock/Write immediately after construction, e.g. C# Texture2D wrapper)
        if (!data)
            m_Image->GetBuffer().Allocate((uint64_t)width * height * Utils::GetImageFormatBPP(format));

        if (RenderThread::IsCurrentThreadRT())
        {
            RT_Init(false);
        }
        else
        {
            Ref<OpenGLTexture2D> instance = this;
            Renderer::Submit([instance]() mutable { instance->RT_Init(false); });
        }
    }

    void OpenGLTexture2D::RT_Init(bool mipmapSampler)
    {
        m_Image->Invalidate();

        RendererID rid = m_Image.As<OpenGLImage2D>()->GetRendererID();
        GLenum wrap = m_Wrap == TextureWrap::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;
        glTextureParameteri(rid, GL_TEXTURE_MIN_FILTER, mipmapSampler ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTextureParameteri(rid, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(rid, GL_TEXTURE_WRAP_S, wrap);
        glTextureParameteri(rid, GL_TEXTURE_WRAP_T, wrap);
        glTextureParameterf(rid, GL_TEXTURE_MAX_ANISOTROPY, Renderer::GetCapabilities().MaxAnisotropy);
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool srgb)
    {
        FilePath = path;
        PR_PROFILE_FUNCTION();
        void* data = nullptr;
        int width, height, channels;
        if (stbi_is_hdr(path.c_str()))
        {
            PR_CORE_INFO("Loading HDR texture {0}, srgb={1}", path, srgb);
            data = stbi_loadf(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            // PR_CORE_ASSERT(data, "Could not read HDR image!");
            if (!data) { PR_CORE_ERROR("Could not read image: {0}", path); return; }
            m_IsHDR = true;
            uint32_t size = width * height * 4 * sizeof(float);
            m_Image = Image2D::Create(ImageFormat::RGBA32F, width, height, Buffer::Copy(data, size));
            stbi_image_free(data);
        }
        else
        {
            PR_CORE_INFO("Loading texture {0}, srgb={1}", path, srgb);
            data = stbi_load(path.c_str(), &width, &height, &channels, srgb ? STBI_rgb : STBI_rgb_alpha);
            // PR_CORE_ASSERT(data, "Could not read image!");
            if (!data) { PR_CORE_ERROR("Could not read image: {0}", path); return; }
            ImageFormat format = srgb ? ImageFormat::SRGB : ImageFormat::RGBA;
            uint32_t size = width * height * Utils::GetImageFormatBPP(format);
            m_Image = Image2D::Create(format, width, height, Buffer::Copy(data, size));
            stbi_image_free(data);
        }

        m_Width = width;
        m_Height = height;
        m_Loaded = true;

        if (RenderThread::IsCurrentThreadRT())
        {
            RT_Init(true);
        }
        else
        {
            Ref<OpenGLTexture2D> instance = this;
            Renderer::Submit([instance]() mutable { instance->RT_Init(true); });
        }
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        Ref<Image2D> image = m_Image;
        Renderer::SubmitResourceFree([image]() mutable {
            image->Release();
        });
    }

    void OpenGLTexture2D::Lock()
    {
        m_Locked = true;
    }

    void OpenGLTexture2D::Unlock()
    {
        m_Locked = false;
        Ref<OpenGLTexture2D> instance = this;
        Renderer::Submit([instance]() {
            RendererID rid = instance->m_Image.As<OpenGLImage2D>()->GetRendererID();
            ImageFormat format = instance->m_Image->GetFormat();
            glTextureSubImage2D(rid, 0, 0, 0, instance->m_Width, instance->m_Height, Utils::OpenGLImageFormat(format), Utils::OpenGLFormatDataType(format), instance->m_Image->GetBuffer().Data);
        });
    }

    Buffer OpenGLTexture2D::GetWriteableBuffer()
    {
        PR_CORE_ASSERT(m_Locked, "Texture must be locked!");
        return m_Image->GetBuffer();
    }


    void OpenGLTexture2D::RT_Bind(uint32_t slot) const
    {
        m_BindSlot = slot;
        glBindTextureUnit(slot, m_Image.As<OpenGLImage2D>()->GetRendererID());
    }

    uint32_t OpenGLTexture2D::GetMipLevelCount() const
    {
        return Texture::CalculateMipMapCount(m_Width, m_Height);
    }

    //////////////////////////////////////////////////////////////////////////////////
    // TextureCube
    //////////////////////////////////////////////////////////////////////////////////

    OpenGLTextureCube::OpenGLTextureCube(ImageFormat format, uint32_t width, uint32_t height, const void* data)
    {
        m_Image = ImageCube::Create(format, width, height, data);

        if (RenderThread::IsCurrentThreadRT())
        {
            m_Image->Invalidate();
        }
        else
        {
            Ref<OpenGLTextureCube> instance = this;
            Renderer::Submit([instance]() mutable
            {
                instance->m_Image->Invalidate();
            });
        }
    }

    OpenGLTextureCube::~OpenGLTextureCube()
    {
    }

    uint32_t OpenGLTextureCube::GetMipLevelCount() const
    {
        return Texture::CalculateMipMapCount(m_Image->GetWidth(), m_Image->GetHeight());
    }


    void OpenGLTextureCube::RT_Bind(uint32_t slot) const
    {
        m_BindSlot = slot;
        glBindTextureUnit(slot, m_Image.As<OpenGLImageCube>()->GetRendererID());
    }

}
