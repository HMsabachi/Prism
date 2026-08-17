#include "prpch.h"

#include "OpenGLFramebuffer.h"

#include "Prism/Renderer/Renderer.h"
#include <glad/glad.h>

namespace Prism {

    namespace Utils
    {
        static GLenum TextureTarget(bool multisampled)
        {
            return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        }

        static GLenum DepthAttachmentType(ImageFormat format)
        {
            switch (format)
            {
                case ImageFormat::DEPTH32F:        return GL_DEPTH_ATTACHMENT;
                case ImageFormat::DEPTH24STENCIL8: return GL_DEPTH_STENCIL_ATTACHMENT;
            }
            PR_CORE_ASSERT(false, "Unknown depth format");
            return 0;
        }

        static Ref<Image2D> CreateAndAttachColorAttachment(uint32_t samples, ImageFormat format, uint32_t width, uint32_t height, int index)
        {
            Ref<Image2D> image = Image2D::Create(format, width, height, nullptr, samples);
            image->Invalidate();

            Ref<OpenGLImage2D> glImage = image.As<OpenGLImage2D>();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(samples > 1), glImage->GetRendererID(), 0);
            return image;
        }

        static Ref<Image2D> AttachDepthTexture(uint32_t samples, ImageFormat format, uint32_t width, uint32_t height)
        {
            Ref<Image2D> image = Image2D::Create(format, width, height, nullptr, samples);
            image->Invalidate();

            Ref<OpenGLImage2D> glImage = image.As<OpenGLImage2D>();
            glFramebufferTexture2D(GL_FRAMEBUFFER, Utils::DepthAttachmentType(format), TextureTarget(samples > 1), glImage->GetRendererID(), 0);
            return image;
        }

        static bool IsDepthFormat(ImageFormat format)
        {
            switch (format)
            {
                case ImageFormat::DEPTH24STENCIL8:
                case ImageFormat::DEPTH32F:
                    return true;
            }
            return false;
        }

    }

    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
        : m_Specification(spec), m_Width(spec.Width), m_Height(spec.Height)
    {
        for (auto format : m_Specification.Attachments.Attachments)
        {
            if (!Utils::IsDepthFormat(format.Format))
                m_ColorAttachmentFormats.emplace_back(format.Format);
            else
                m_DepthAttachmentFormat = format.Format;
        }

        Resize(spec.Width, spec.Height, true);
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        Ref<OpenGLFramebuffer> instance = this;
        Renderer::SubmitResourceFree([instance]() {
            glDeleteFramebuffers(1, &instance->m_RendererID);
        });
    }

    void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height, bool forceRecreate)
    {
        if (!forceRecreate && (m_Width == width && m_Height == height))
            return;

        m_Width = width;
        m_Height = height;

        Ref<OpenGLFramebuffer> instance = this;
        Renderer::Submit([instance, forceRecreate]() mutable
        {
            instance->RT_Resize(instance->m_Width, instance->m_Height, forceRecreate);
        });
    }


    void OpenGLFramebuffer::RT_Resize(uint32_t width, uint32_t height, bool forceRecreate /*= false*/)
    {
        if (!forceRecreate && (m_Width == width && m_Height == height)) return;
        m_Width = width;
        m_Height = height;
        if (m_RendererID)
        {
            glDeleteFramebuffers(1, &m_RendererID);
            m_ColorAttachments.clear();
            m_DepthAttachment.Reset();
        }

        glGenFramebuffers(1, &m_RendererID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

        if (m_ColorAttachmentFormats.size())
        {
            m_ColorAttachments.resize(m_ColorAttachmentFormats.size());
            for (size_t i = 0; i < m_ColorAttachments.size(); i++)
                m_ColorAttachments[i] = Utils::CreateAndAttachColorAttachment(m_Specification.Samples, m_ColorAttachmentFormats[i], m_Width, m_Height, (int)i);
        }

        if (m_DepthAttachmentFormat != ImageFormat::None)
        {
            m_DepthAttachment = Utils::AttachDepthTexture(m_Specification.Samples, m_DepthAttachmentFormat, m_Width, m_Height);
        }

        if (m_ColorAttachments.size() > 1)
        {
            PR_CORE_ASSERT(m_ColorAttachments.size() <= 4);
            GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
            glDrawBuffers((uint32_t)m_ColorAttachments.size(), buffers);
        }
        else if (m_ColorAttachments.size() == 0)
        {
            // Only depth-pass
            glDrawBuffer(GL_NONE);
        }

        PR_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::Bind() const
    {
        Ref<const OpenGLFramebuffer> instance = this;
        Renderer::Submit([instance]() {
            instance->RT_Bind();
        });
    }

    void OpenGLFramebuffer::Unbind() const
    {
        Ref<const OpenGLFramebuffer> instance = this;
        Renderer::Submit([instance]() {
            instance->RT_Unbind();
        });
    }


    void OpenGLFramebuffer::RT_Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        glViewport(0, 0, m_Width, m_Height);
    }


    void OpenGLFramebuffer::RT_Unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

}
