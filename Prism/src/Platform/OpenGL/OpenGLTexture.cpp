#include "prpch.h"
#include "OpenGLTexture.h"

#include "Prism/Renderer/RendererAPI.h"
#include "Prism/Renderer/Renderer.h"
#include "Prism/Utilities/TextureUtils.h"
#include "Prism/Core/RenderThread.h"

#include <glad/glad.h>
#include "stb_image.h"

namespace Prism {

    static GLenum PrismToOpenGLTextureFormat(ImageFormat format)
    {
        switch (format)
        {
        case ImageFormat::RGB8:       return GL_RGB8;
        case ImageFormat::RGBA8_SRGB: return GL_SRGB8_ALPHA8;
        case ImageFormat::RGBA8:      return GL_RGBA8;
        case ImageFormat::RGBA16F:    return GL_RGBA16F;
        case ImageFormat::RGBA32F:    return GL_RGBA32F;
        }
        PR_CORE_ASSERT(false, "Unknown texture format!");
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////////////
    // Texture2D
    //////////////////////////////////////////////////////////////////////////////////

    OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification, Buffer imageData)
        : m_Specification(specification)
    {
        ImageSpecification imageSpecification{};
        imageSpecification.Format = m_Specification.Format;
        imageSpecification.Width = m_Specification.Width;
        imageSpecification.Height = m_Specification.Height;
        imageSpecification.CreateSampler = false;

        bool hasData = imageData;
        if (hasData)
            m_Image = Image2D::Create(imageSpecification, std::move(imageData));
        else
            m_Image = Image2D::Create(imageSpecification);

        // Allocate CPU buffer for Lock/Unlock/GetWriteableBuffer when no initial data
        // (callers Lock/Write immediately after construction, e.g. C# Texture2D wrapper)
        if (!hasData)
            m_Image->GetBuffer().Allocate(Utils::GetImageMemorySize(m_Specification.Format, m_Specification.Width, m_Specification.Height));

        if (RenderThread::IsCurrentThreadRT())
        {
            RT_Init();
        }
        else
        {
            Ref<OpenGLTexture2D> instance = this;
            Renderer::Submit([instance]() mutable { instance->RT_Init(); });
        }
    }

    void OpenGLTexture2D::RT_Init()
    {
        m_Image->Invalidate();

        Ref<OpenGLImage2D> image = m_Image.As<OpenGLImage2D>();
        RendererID rid = image->GetRendererID();
        bool mipmapSampler = m_Specification.GenerateMips && image->GetMipLevelCount() > 1;
        glTextureParameteri(rid, GL_TEXTURE_MIN_FILTER, Utils::OpenGLSamplerFilter(m_Specification.SamplerFilter, mipmapSampler));
        glTextureParameteri(rid, GL_TEXTURE_MAG_FILTER, Utils::OpenGLSamplerFilter(m_Specification.SamplerFilter, false));
        glTextureParameteri(rid, GL_TEXTURE_WRAP_S, Utils::OpenGLSamplerWrap(m_Specification.SamplerWrap));
        glTextureParameteri(rid, GL_TEXTURE_WRAP_T, Utils::OpenGLSamplerWrap(m_Specification.SamplerWrap));
        glTextureParameterf(rid, GL_TEXTURE_MAX_ANISOTROPY, Renderer::GetCapabilities().MaxAnisotropy);
    }

    OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification, const std::string& path)
        : m_Specification(specification)
    {
        FilePath = path;
        PR_PROFILE_FUNCTION();

        if (IsDDSFile(path))
        {
            TextureLoadResult dds;
            if (!LoadDDS(path, dds))
            {
                PR_CORE_ERROR("Failed to load DDS: {0}", path);
                return;
            }
            m_Specification.Format = dds.Format;
            m_Specification.Width = dds.Width;
            m_Specification.Height = dds.Height;
            m_Loaded = true;

            ImageSpecification imageSpecification{};
            imageSpecification.Format = m_Specification.Format;
            imageSpecification.Width = m_Specification.Width;
            imageSpecification.Height = m_Specification.Height;
            imageSpecification.CreateSampler = false;
            m_Image = Image2D::Create(imageSpecification, std::move(dds.Mips));

            if (RenderThread::IsCurrentThreadRT())
            {
                RT_Init();
            }
            else
            {
                Ref<OpenGLTexture2D> instance = this;
                Renderer::Submit([instance]() mutable { instance->RT_Init(); });
            }
            return;
        }

        void* data = nullptr;
        int width, height, channels;
        if (stbi_is_hdr(path.c_str()))
        {
            PR_CORE_INFO("Loading HDR texture {0}, srgb={1}", path, Utils::IsSRGBFormat(m_Specification.Format));
            data = stbi_loadf(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            // PR_CORE_ASSERT(data, "Could not read HDR image!");
            if (!data) { PR_CORE_ERROR("Could not read image: {0}", path); return; }
            m_IsHDR = true;
            m_Specification.Format = ImageFormat::RGBA32F;
            m_Specification.Width = width;
            m_Specification.Height = height;
            uint32_t size = width * height * 4 * sizeof(float);
            ImageSpecification imageSpecification{};
            imageSpecification.Format = m_Specification.Format;
            imageSpecification.Width = m_Specification.Width;
            imageSpecification.Height = m_Specification.Height;
            imageSpecification.CreateSampler = false;
            m_Image = Image2D::Create(imageSpecification, Buffer::Copy(data, size));
            stbi_image_free(data);
        }
        else
        {
            PR_CORE_INFO("Loading texture {0}, srgb={1}", path, Utils::IsSRGBFormat(m_Specification.Format));
            data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            // PR_CORE_ASSERT(data, "Could not read image!");
            if (!data) { PR_CORE_ERROR("Could not read image: {0}", path); return; }
            m_Specification.Format = Utils::IsSRGBFormat(m_Specification.Format) ? ImageFormat::RGBA8_SRGB : ImageFormat::RGBA8;
            m_Specification.Width = width;
            m_Specification.Height = height;
            uint32_t size = Utils::GetImageMemorySize(m_Specification.Format, m_Specification.Width, m_Specification.Height);
            ImageSpecification imageSpecification{};
            imageSpecification.Format = m_Specification.Format;
            imageSpecification.Width = m_Specification.Width;
            imageSpecification.Height = m_Specification.Height;
            imageSpecification.CreateSampler = false;
            m_Image = Image2D::Create(imageSpecification, Buffer::Copy(data, size));
            stbi_image_free(data);
        }

        m_Loaded = true;

        if (RenderThread::IsCurrentThreadRT())
        {
            RT_Init();
        }
        else
        {
            Ref<OpenGLTexture2D> instance = this;
            Renderer::Submit([instance]() mutable { instance->RT_Init(); });
        }
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        Ref<Image2D> image = m_Image;
        Renderer::SubmitResourceFree([image]() mutable {
            // image->Release();
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
            glTextureSubImage2D(rid, 0, 0, 0, instance->m_Specification.Width, instance->m_Specification.Height, Utils::OpenGLImageFormat(format), Utils::OpenGLFormatDataType(format), instance->m_Image->GetBuffer().Data);
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
        return Utils::CalculateMipCount(m_Specification.Width, m_Specification.Height);
    }

    //////////////////////////////////////////////////////////////////////////////////
    // TextureCube
    //////////////////////////////////////////////////////////////////////////////////

    OpenGLTextureCube::OpenGLTextureCube(const TextureSpecification& specification, Buffer imageData)
        : m_Specification(specification)
    {
        ImageSpecification imageSpecification{};
        imageSpecification.Format = m_Specification.Format;
        imageSpecification.Width = m_Specification.Width;
        imageSpecification.Height = m_Specification.Height;
        imageSpecification.CreateSampler = false;

        if (imageData)
            m_Image = ImageCube::Create(imageSpecification, std::move(imageData));
        else
            m_Image = ImageCube::Create(imageSpecification);

        if (RenderThread::IsCurrentThreadRT())
        {
            RT_Init();
        }
        else
        {
            Ref<OpenGLTextureCube> instance = this;
            Renderer::Submit([instance]() mutable { instance->RT_Init(); });
        }
    }

    void OpenGLTextureCube::RT_Init()
    {
        m_Image->Invalidate();

        RendererID rid = m_Image.As<OpenGLImageCube>()->GetRendererID();
        glTextureParameteri(rid, GL_TEXTURE_MIN_FILTER, Utils::OpenGLSamplerFilter(m_Specification.SamplerFilter, m_Specification.GenerateMips));
        glTextureParameteri(rid, GL_TEXTURE_MAG_FILTER, Utils::OpenGLSamplerFilter(m_Specification.SamplerFilter, false));
        glTextureParameteri(rid, GL_TEXTURE_WRAP_S, Utils::OpenGLSamplerWrap(m_Specification.SamplerWrap));
        glTextureParameteri(rid, GL_TEXTURE_WRAP_T, Utils::OpenGLSamplerWrap(m_Specification.SamplerWrap));
        glTextureParameteri(rid, GL_TEXTURE_WRAP_R, Utils::OpenGLSamplerWrap(m_Specification.SamplerWrap));
    }

    OpenGLTextureCube::~OpenGLTextureCube()
    {
    }

    uint32_t OpenGLTextureCube::GetMipLevelCount() const
    {
        return Utils::CalculateMipCount(m_Specification.Width, m_Specification.Height);
    }


    void OpenGLTextureCube::RT_Bind(uint32_t slot) const
    {
        m_BindSlot = slot;
        glBindTextureUnit(slot, m_Image.As<OpenGLImageCube>()->GetRendererID());
    }

}
