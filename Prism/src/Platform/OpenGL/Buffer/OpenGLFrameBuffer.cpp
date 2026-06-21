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

        static void CreateTextures(bool multisampled, RendererID* outID, uint32_t count)
        {
            glCreateTextures(TextureTarget(multisampled), count, outID);
        }

        static void BindTexture(bool multisampled, RendererID id)
        {
            glBindTexture(TextureTarget(multisampled), id);
        }

        static GLenum PrismFBTextureFormatToGL(FramebufferTextureFormat format)
        {
            switch (format)
            {
                case FramebufferTextureFormat::RGBA8:    return GL_RGBA8;
                case FramebufferTextureFormat::RGBA16F:  return GL_RGBA16F;
                case FramebufferTextureFormat::RGBA32F:  return GL_RGBA32F;
                case FramebufferTextureFormat::RG32F:    return GL_RG32F;
                case FramebufferTextureFormat::DEPTH32F: return GL_DEPTH_COMPONENT32F;
                case FramebufferTextureFormat::DEPTH24STENCIL8: return GL_DEPTH24_STENCIL8;
            }
            PR_CORE_ASSERT(false, "Unknown format!");
            return 0;
        }

        static GLenum DataType(GLenum format)
        {
            switch (format)
            {
                case GL_RGBA8: return GL_UNSIGNED_BYTE;
                case GL_RG16F:
                case GL_RG32F:
                case GL_RGBA16F:
                case GL_RGBA32F: return GL_FLOAT;
                case GL_DEPTH24_STENCIL8: return GL_UNSIGNED_INT_24_8;
            }
            PR_CORE_ASSERT(false, "Unknown format!");
            return 0;
        }

        static void AttachColorTexture(RendererID id, int samples, GLenum format, uint32_t width, uint32_t height, int index)
        {
            bool multisampled = samples > 1;
            if (multisampled)
            {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, DataType(format), nullptr);

                glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
        }

        static void AttachDepthTexture(RendererID id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height)
        {
            bool multisampled = samples > 1;
            if (multisampled)
            {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
            }
            else
            {
                glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

                glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(TextureTarget(multisampled), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
        }

        static bool IsDepthFormat(FramebufferTextureFormat format)
        {
            switch (format)
            {
                case FramebufferTextureFormat::DEPTH24STENCIL8:
                case FramebufferTextureFormat::DEPTH32F:
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
            if (!Utils::IsDepthFormat(format.TextureFormat))
                m_ColorAttachmentFormats.emplace_back(format.TextureFormat);
            else
                m_DepthAttachmentFormat = format.TextureFormat;
        }

        Resize(spec.Width, spec.Height, true);
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        Ref<OpenGLFramebuffer> instance = this;
        Renderer::Submit([instance]() {
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
        Renderer::Submit([instance]() mutable
        {
            if (instance->m_RendererID)
            {
                glDeleteFramebuffers(1, &instance->m_RendererID);
                glDeleteTextures((uint32_t)instance->m_ColorAttachments.size(), instance->m_ColorAttachments.data());
                glDeleteTextures(1, &instance->m_DepthAttachment);
                instance->m_ColorAttachments.clear();
                instance->m_DepthAttachment = 0;
            }

            glGenFramebuffers(1, &instance->m_RendererID);
            glBindFramebuffer(GL_FRAMEBUFFER, instance->m_RendererID);

            bool multisample = instance->m_Specification.Samples > 1;

            if (instance->m_ColorAttachmentFormats.size())
            {
                instance->m_ColorAttachments.resize(instance->m_ColorAttachmentFormats.size());
                Utils::CreateTextures(multisample, instance->m_ColorAttachments.data(), (uint32_t)instance->m_ColorAttachments.size());

                for (int i = 0; i < instance->m_ColorAttachments.size(); i++)
                {
                    Utils::BindTexture(multisample, instance->m_ColorAttachments[i]);
                    GLenum glFormat = Utils::PrismFBTextureFormatToGL(instance->m_ColorAttachmentFormats[i]);
                    Utils::AttachColorTexture(instance->m_ColorAttachments[i], instance->m_Specification.Samples, glFormat, instance->m_Width, instance->m_Height, i);
                }
            }

            if (instance->m_DepthAttachmentFormat != FramebufferTextureFormat::None)
            {
                Utils::CreateTextures(multisample, &instance->m_DepthAttachment, 1);
                Utils::BindTexture(multisample, instance->m_DepthAttachment);

                GLenum glFormat = Utils::PrismFBTextureFormatToGL(instance->m_DepthAttachmentFormat);
                GLenum attachmentType = GL_DEPTH_ATTACHMENT;

                // Setup depth comparison for shadow maps
                if (instance->m_DepthAttachmentFormat == FramebufferTextureFormat::DEPTH32F)
                {
                    glTexParameteri(Utils::TextureTarget(multisample), GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                    glTexParameteri(Utils::TextureTarget(multisample), GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
                }
                else if (instance->m_DepthAttachmentFormat == FramebufferTextureFormat::DEPTH24STENCIL8)
                {
                    attachmentType = GL_DEPTH_STENCIL_ATTACHMENT;
                }

                Utils::AttachDepthTexture(instance->m_DepthAttachment, instance->m_Specification.Samples, glFormat, attachmentType, instance->m_Width, instance->m_Height);
            }

            if (instance->m_ColorAttachments.size() > 1)
            {
                PR_CORE_ASSERT(instance->m_ColorAttachments.size() <= 4);
                GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
                glDrawBuffers((uint32_t)instance->m_ColorAttachments.size(), buffers);
            }
            else if (instance->m_ColorAttachments.size() == 0)
            {
                // Only depth-pass
                glDrawBuffer(GL_NONE);
            }

            PR_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        });
    }

    void OpenGLFramebuffer::Bind() const
    {
        Ref<const OpenGLFramebuffer> instance = this;
        Renderer::Submit([instance]() {
            glBindFramebuffer(GL_FRAMEBUFFER, instance->m_RendererID);
            glViewport(0, 0, instance->m_Width, instance->m_Height);
        });
    }

    void OpenGLFramebuffer::Unbind() const
    {
        Renderer::Submit([]() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        });
    }

    void OpenGLFramebuffer::BindTexture(uint32_t attachmentIndex, uint32_t slot) const
    {
        Ref<const OpenGLFramebuffer> instance = this;
        Renderer::Submit([instance, attachmentIndex, slot]() {
            glBindTextureUnit(slot, instance->m_ColorAttachments[attachmentIndex]);
        });
    }

    void OpenGLFramebuffer::BindDepthTexture(uint32_t slot) const
    {
        Ref<const OpenGLFramebuffer> instance = this;
        Renderer::Submit([instance, slot]() {
            glBindTextureUnit(slot, instance->m_DepthAttachment);
        });
    }

}
