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
        :m_Format(format), m_Width(width), m_Height(height), m_RendererID(0)
    {

        uint32_t levels = Texture::CalculateMipMapCount(width, height);

        // Deep copy initial data (deferred submit may outlive caller's buffer)
        Buffer localData;
        if (data)
            localData = Buffer::Copy(data, (uint64_t)Utils::GetImageFormatBPP(format) * width * height * 6);

        Ref<OpenGLTextureCube> instance = this;
        Renderer::Submit([instance, levels, localData]() mutable
        {
            glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &instance->m_RendererID);
            glTextureStorage2D(instance->m_RendererID, levels, PrismToOpenGLTextureFormat(instance->m_Format), instance->m_Width, instance->m_Height);
            if (localData.Data)
                glTextureSubImage3D(instance->m_RendererID, 0, 0, 0, 0, instance->m_Width, instance->m_Height, 6, Utils::OpenGLImageFormat(instance->m_Format), Utils::OpenGLFormatDataType(instance->m_Format), localData.Data);
            glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            glTextureParameteri(instance->m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            // glTextureParameterf(m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, 16);
            });
    }

    OpenGLTextureCube::OpenGLTextureCube(const std::string& path)
        : m_RendererID(0)
    {
        FilePath = path;
        PR_PROFILE_FUNCTION();

        int width, height, channels;
        stbi_set_flip_vertically_on_load(false);
        m_ImageData = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb);

        m_Width = width;
        m_Height = height;
        m_Format = ImageFormat::RGB;

        uint32_t faceWidth = m_Width / 4;
        uint32_t faceHeight = m_Height / 3;
        PR_CORE_ASSERT(faceWidth == faceHeight, "Non-square faces!");

        std::array<uint8_t*, 6> faces;
        for (size_t i = 0; i < faces.size(); i++)
            faces[i] = new uint8_t[faceWidth * faceHeight * 3]; // 3 BPP

        int faceIndex = 0;

        for (size_t i = 0; i < 4; i++)
        {
            for (size_t y = 0; y < faceHeight; y++)
            {
                size_t yOffset = y + faceHeight;
                for (size_t x = 0; x < faceWidth; x++)
                {
                    size_t xOffset = x + i * faceWidth;
                    faces[faceIndex][(x + y * faceWidth) * 3 + 0] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 0];
                    faces[faceIndex][(x + y * faceWidth) * 3 + 1] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 1];
                    faces[faceIndex][(x + y * faceWidth) * 3 + 2] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 2];
                }
            }
            faceIndex++;
        }

        for (size_t i = 0; i < 3; i++)
        {
            // Skip the middle one
            if (i == 1)
                continue;

            for (size_t y = 0; y < faceHeight; y++)
            {
                size_t yOffset = y + i * faceHeight;
                for (size_t x = 0; x < faceWidth; x++)
                {
                    size_t xOffset = x + faceWidth;
                    faces[faceIndex][(x + y * faceWidth) * 3 + 0] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 0];
                    faces[faceIndex][(x + y * faceWidth) * 3 + 1] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 1];
                    faces[faceIndex][(x + y * faceWidth) * 3 + 2] = m_ImageData[(xOffset + yOffset * m_Width) * 3 + 2];
                }
            }
            faceIndex++;
        }

        Ref<OpenGLTextureCube> instance = this;
        Renderer::Submit([instance, faceWidth, faceHeight, faces]() mutable
        {
            glGenTextures(1, &instance->m_RendererID);
            glBindTexture(GL_TEXTURE_CUBE_MAP, instance->m_RendererID);

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTextureParameterf(instance->m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, Renderer::GetCapabilities().MaxAnisotropy);

            auto format = PrismToOpenGLTextureFormat(instance->m_Format);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[2]);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[0]);

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[4]);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[5]);

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[1]);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[3]);

            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

            glBindTexture(GL_TEXTURE_2D, 0);

            for (size_t i = 0; i < faces.size(); i++)
                delete[] faces[i];

            stbi_image_free(instance->m_ImageData);
            });
    }

    OpenGLTextureCube::~OpenGLTextureCube()
    {
        GLuint rendererID = m_RendererID;
        Renderer::Submit([rendererID]() mutable {
            glDeleteTextures(1, &rendererID);
        });
    }

    void OpenGLTextureCube::Bind(unsigned int slot) const
    {
        Ref<const OpenGLTextureCube> instance = this;
        Renderer::Submit([instance, slot]() mutable {
            instance->m_BindSlot = slot;
            glBindTextureUnit(slot, instance->m_RendererID);
            });
    }

    void OpenGLTextureCube::BindImage(uint32_t slot, TextureAccess access, bool layered /*= true*/, uint32_t mipLevel) const
    {
        Ref<const OpenGLTextureCube> instance = this;
        Renderer::Submit([=]() mutable
        {
            instance->m_BindSlot = slot;
            glBindImageTexture(slot, instance->m_RendererID, mipLevel, layered, 0, PrismToOpenGLTextureAccess(access), PrismToOpenGLTextureFormat(instance->m_Format));
        });
    }

    uint32_t OpenGLTextureCube::GetMipLevelCount() const
    {
        return Texture::CalculateMipMapCount(m_Width, m_Height);
    }

    void OpenGLTextureCube::GenerateMipMap() const
    {
        Ref<const OpenGLTextureCube> instance = this;
        Renderer::Submit([instance]() mutable
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, instance->m_RendererID);
            glGenerateTextureMipmap(instance->m_RendererID);
        });
    }

    void OpenGLTextureCube::CopyTo(Ref<TextureCube> destination) const
    {
        Ref<const OpenGLTextureCube> instance = this;
        Renderer::Submit([instance, destination]() mutable
        {
            glCopyImageSubData(instance->m_RendererID, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0, destination->GetRendererID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0, instance->m_Width, instance->m_Height, 6);
        });
    }

}
