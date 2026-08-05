#include "prpch.h"
#include "OpenGLTexture.h"

#include "Prism/Renderer/RendererAPI.h"
#include "Prism/Renderer/Renderer.h"

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
    static GLenum PrismToOpenGLTextureAccess(TextureAccess access)
    {
        switch (access)
        {
        case Prism::TextureAccess::ReadOnly:  return GL_READ_ONLY;
        case Prism::TextureAccess::WriteOnly: return GL_WRITE_ONLY;
        case Prism::TextureAccess::ReadWrite: return GL_READ_WRITE;
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
        PR_PROFILE_FUNCTION();

        m_Image = Image2D::Create(format, width, height, data);
        // Allocate CPU buffer for Lock/Unlock/GetWriteableBuffer when no initial data
        // (callers Lock/Write immediately after construction, e.g. C# Texture2D wrapper)
        if (!data)
            m_Image->GetBuffer().Allocate((uint64_t)width * height * Utils::GetImageFormatBPP(format));

        Ref<OpenGLTexture2D> instance = this;
        Renderer::Submit([instance]() mutable
            {
            instance->m_Image->Invalidate();

            // Prism 采样配置（保留 TextureWrap/anisotropy，方案B）
            RendererID rid = instance->m_Image.As<OpenGLImage2D>()->GetRendererID();
            GLenum wrap = instance->m_Wrap == TextureWrap::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;
            glTextureParameteri(rid, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(rid, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(rid, GL_TEXTURE_WRAP_S, wrap);
            glTextureParameteri(rid, GL_TEXTURE_WRAP_T, wrap);
            glTextureParameterf(rid, GL_TEXTURE_MAX_ANISOTROPY, Renderer::GetCapabilities().MaxAnisotropy);
            });
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool srgb)
    {
        FilePath = path;
        PR_PROFILE_FUNCTION();

        int width, height, channels;
        if (stbi_is_hdr(path.c_str()))
        {
            PR_CORE_INFO("Loading HDR texture {0}, srgb={1}", path, srgb);
            float* data = stbi_loadf(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            PR_CORE_ASSERT(data, "Could not read HDR image!");
            m_IsHDR = true;
            uint32_t size = width * height * 4 * sizeof(float);
            m_Image = Image2D::Create(ImageFormat::RGBA16F, width, height, Buffer::Copy(data, size));
            stbi_image_free(data);
        }
        else
        {
            PR_CORE_INFO("Loading texture {0}, srgb={1}", path, srgb);
            stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, srgb ? STBI_rgb : STBI_rgb_alpha);
            PR_CORE_ASSERT(data, "Could not read image!");
            ImageFormat format = srgb ? ImageFormat::SRGB : ImageFormat::RGBA;
            uint32_t size = width * height * Utils::GetImageFormatBPP(format);
            m_Image = Image2D::Create(format, width, height, Buffer::Copy(data, size));
            stbi_image_free(data);
        }

        m_Width = width;
        m_Height = height;
        m_Loaded = true;

        Ref<OpenGLTexture2D> instance = this;
        Renderer::Submit([instance]() mutable
            {
            instance->m_Image->Invalidate();

            // Prism 采样配置
            RendererID rid = instance->m_Image.As<OpenGLImage2D>()->GetRendererID();
            GLenum wrap = instance->m_Wrap == TextureWrap::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;
            glTextureParameteri(rid, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTextureParameteri(rid, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(rid, GL_TEXTURE_WRAP_S, wrap);
            glTextureParameteri(rid, GL_TEXTURE_WRAP_T, wrap);
            glTextureParameterf(rid, GL_TEXTURE_MAX_ANISOTROPY, Renderer::GetCapabilities().MaxAnisotropy);
            });
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        Ref<Image2D> image = m_Image;
        Renderer::Submit([image]() mutable {
            image->Release();
        });
    }

    void OpenGLTexture2D::Bind(unsigned int slot) const
    {
        Ref<const OpenGLTexture2D> instance = this;
        Renderer::Submit([instance, slot]()
        {
                instance->m_BindSlot = slot;
            glBindTextureUnit(slot, instance->m_Image.As<OpenGLImage2D>()->GetRendererID());
        });
    }

    void OpenGLTexture2D::BindImage(uint32_t slot, TextureAccess access, bool layered, uint32_t mipLevel) const
    {
        Ref<const OpenGLTexture2D> instance = this;
        Renderer::Submit([=]()
        {
            instance->m_BindSlot = slot;
            glBindImageTexture(slot, instance->m_Image.As<OpenGLImage2D>()->GetRendererID(), mipLevel, layered, 0, PrismToOpenGLTextureAccess(access), PrismToOpenGLTextureFormat(instance->m_Image->GetFormat()));
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

        Ref<OpenGLTextureCube> instance = this;
        Renderer::Submit([instance]() mutable
        {
            instance->m_Image->Invalidate();
        });
    }

    OpenGLTextureCube::~OpenGLTextureCube()
    {
    }

    void OpenGLTextureCube::Bind(unsigned int slot) const
    {
        Ref<const OpenGLTextureCube> instance = this;
        Renderer::Submit([instance, slot]() mutable {
            instance->m_BindSlot = slot;
            glBindTextureUnit(slot, instance->GetRendererID());
            });
    }

    void OpenGLTextureCube::BindImage(uint32_t slot, TextureAccess access, bool layered /*= true*/, uint32_t mipLevel) const
    {
        Ref<const OpenGLTextureCube> instance = this;
        Renderer::Submit([=]() mutable
        {
            instance->m_BindSlot = slot;
            glBindImageTexture(slot, instance->GetRendererID(), mipLevel, layered, 0, PrismToOpenGLTextureAccess(access), PrismToOpenGLTextureFormat(instance->GetFormat()));
        });
    }

    uint32_t OpenGLTextureCube::GetMipLevelCount() const
    {
        return Texture::CalculateMipMapCount(m_Image->GetWidth(), m_Image->GetHeight());
    }

    void OpenGLTextureCube::GenerateMipMap() const
    {
        Ref<const OpenGLTextureCube> instance = this;
        Renderer::Submit([instance]() mutable
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, instance->GetRendererID());
            glGenerateTextureMipmap(instance->GetRendererID());
        });
    }

    void OpenGLTextureCube::CopyTo(Ref<TextureCube> destination) const
    {
        Ref<const OpenGLTextureCube> instance = this;
        Renderer::Submit([instance, destination]() mutable
        {
            glCopyImageSubData(instance->GetRendererID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0, destination.As<OpenGLTextureCube>()->GetRendererID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0, instance->GetWidth(), instance->GetHeight(), 6);
        });
    }

}
