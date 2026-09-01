#include "prpch.h"
#include "OpenGLImage.h"
#include <glad/glad.h>
#include "Prism/Renderer/Renderer.h"

namespace Prism {

    OpenGLImage2D::OpenGLImage2D(ImageFormat format, uint32_t width, uint32_t height, const void* data, uint32_t samples)
        : m_Width(width), m_Height(height), m_Samples(samples), m_Format(format)
    {
        // TODO: Local storage should be optional
        if (data)
            m_ImageData = Buffer::Copy(data, Utils::GetImageMemorySize(format, width, height));
    }

    OpenGLImage2D::OpenGLImage2D(ImageFormat format, uint32_t width, uint32_t height, Buffer buffer, uint32_t samples)
        : m_Width(width), m_Height(height), m_Samples(samples), m_Format(format), m_ImageData(buffer)
    {
    }

    OpenGLImage2D::OpenGLImage2D(ImageFormat format, uint32_t width, uint32_t height, std::vector<Buffer>&& mips)
        : m_Width(width), m_Height(height), m_Format(format), m_Mips(std::move(mips))
    {
    }

    OpenGLImage2D::~OpenGLImage2D()
    {
        // Should this be submitted?
        m_ImageData.Release();
        if (m_RendererID)
        {
            RendererID rendererID = m_RendererID;
            Renderer::SubmitResourceFree([rendererID]()
            {
                glDeleteTextures(1, &rendererID);
            });
        }
    }


    void OpenGLImage2D::Resize(const uint32_t width, const uint32_t height)
    {
        Ref<OpenGLImage2D> instance = this;
        Renderer::Submit([instance, width, height]() mutable
        {
            instance->RT_Resize(width, height);
        });
    }

    void OpenGLImage2D::RT_Resize(const uint32_t width, const uint32_t height)
    {
        m_Width = width;
        m_Height = height;
        Invalidate();
    }

    void OpenGLImage2D::Invalidate()
    {
        if (m_RendererID)
            Release();

        GLenum internalFormat = Utils::OpenGLImageInternalFormat(m_Format);

        if (m_Samples > 1)
        {
            glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &m_RendererID);
            glTextureStorage2DMultisample(m_RendererID, m_Samples, internalFormat, m_Width, m_Height, GL_FALSE);
            return;
        }

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

        if (!m_Mips.empty())
        {
            uint32_t mipCount = (uint32_t)m_Mips.size();
            glTextureStorage2D(m_RendererID, mipCount, internalFormat, m_Width, m_Height);
            bool compressed = Utils::IsCompressedFormat(m_Format);
            for (uint32_t i = 0; i < mipCount; i++)
            {
                uint32_t w = std::max(1u, m_Width >> i);
                uint32_t h = std::max(1u, m_Height >> i);
                if (compressed)
                {
                    uint32_t bw = (w + 3) & ~3u;
                    uint32_t bh = (h + 3) & ~3u;
                    glCompressedTextureSubImage2D(m_RendererID, i, 0, 0, bw, bh, internalFormat, (GLsizei)m_Mips[i].Size, m_Mips[i].Data);
                }
                else
                {
                    GLenum format = Utils::OpenGLImageFormat(m_Format);
                    GLenum dataType = Utils::OpenGLFormatDataType(m_Format);
                    glTextureSubImage2D(m_RendererID, i, 0, 0, w, h, format, dataType, m_Mips[i].Data);
                }
            }
        }
        else if (m_ImageData)
        {
            uint32_t mipCount = Utils::CalculateMipCount(m_Width, m_Height);
            glTextureStorage2D(m_RendererID, mipCount, internalFormat, m_Width, m_Height);
            GLenum format = Utils::OpenGLImageFormat(m_Format);
            GLenum dataType = Utils::OpenGLFormatDataType(m_Format);
            glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, format, dataType, m_ImageData.Data);
            glGenerateTextureMipmap(m_RendererID); // TODO: optional
        }
        else
        {
            // TODO: Framebuffer 附件无 CPU 数据,单层+非mipmap filter 避免纹理 incomplete(dead sampler 未 bind)
            glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);
            glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        // Sampler
        // TODO: should be separate from Image2D
        glCreateSamplers(1, &m_SamplerRendererID);
        glSamplerParameteri(m_SamplerRendererID, GL_TEXTURE_MIN_FILTER, (m_ImageData || !m_Mips.empty()) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glSamplerParameteri(m_SamplerRendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(m_SamplerRendererID, GL_TEXTURE_WRAP_R, GL_REPEAT);
        glSamplerParameteri(m_SamplerRendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(m_SamplerRendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    void OpenGLImage2D::Release()
    {
        if (m_RendererID)
        {
            glDeleteTextures(1, &m_RendererID);
            m_RendererID = 0;
        }
        m_ImageData.Release();
    }


    void OpenGLImage2D::RT_Bind(uint32_t slot) const
    {
        glBindTextureUnit(slot, m_RendererID);
    }

    //////////////////////////////////////////////////////////////////////////////////
    // ImageCube
    //////////////////////////////////////////////////////////////////////////////////

    OpenGLImageCube::OpenGLImageCube(ImageFormat format, uint32_t width, uint32_t height, const void* data)
        : m_Width(width), m_Height(height), m_Format(format)
    {
        if (data)
            m_ImageData = Buffer::Copy(data, (uint64_t)Utils::GetImageFormatBPP(format) * width * height * 6);
    }

    OpenGLImageCube::~OpenGLImageCube()
    {
        m_ImageData.Release();
        if (m_RendererID)
        {
            RendererID rendererID = m_RendererID;
            Renderer::SubmitResourceFree([rendererID]()
            {
                glDeleteTextures(1, &rendererID);
            });
        }
    }

    void OpenGLImageCube::Invalidate()
    {
        if (m_RendererID)
            Release();

        uint32_t levels = Utils::CalculateMipCount(m_Width, m_Height);
        GLenum internalFormat = Utils::OpenGLImageInternalFormat(m_Format);

        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, levels, internalFormat, m_Width, m_Height);

        if (m_ImageData)
        {
            GLenum format = Utils::OpenGLImageFormat(m_Format);
            GLenum dataType = Utils::OpenGLFormatDataType(m_Format);
            glTextureSubImage3D(m_RendererID, 0, 0, 0, 0, m_Width, m_Height, 6, format, dataType, m_ImageData.Data);
        }

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    void OpenGLImageCube::Release()
    {
        if (m_RendererID)
        {
            glDeleteTextures(1, &m_RendererID);
            m_RendererID = 0;
        }
        m_ImageData.Release();
    }


    void OpenGLImageCube::GenerateMipMap()
    {
        Ref<OpenGLImageCube> instance = this;
        Renderer::Submit([instance]() mutable
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, instance->GetRendererID());
            glGenerateTextureMipmap(instance->GetRendererID());
        });
    }
    void OpenGLImageCube::CopyTo(Ref<ImageCube> destination) const
    {
        Ref<const OpenGLImageCube> instance = this;
        Renderer::Submit([instance, destination]() mutable
        {
            glCopyImageSubData(instance->GetRendererID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0, destination.As<OpenGLImageCube>()->GetRendererID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0, instance->GetWidth(), instance->GetHeight(), 6);
        });
    }

    void OpenGLImageCube::RT_Bind(uint32_t slot) const
    {
        glBindTextureUnit(slot, m_RendererID);
    }

}
