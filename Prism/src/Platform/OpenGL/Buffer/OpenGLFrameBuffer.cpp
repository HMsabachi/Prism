#include "prpch.h"

#include "OpenGLFramebuffer.h"

#include "Prism/Renderer/Renderer.h"
#include <glad/glad.h>

namespace Prism {


    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
        : m_Specification(spec)
    {
        Resize(spec.Width, spec.Height, true);
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        GLuint id = m_RendererID;
        GLuint color = m_ColorAttachment;
        GLuint depth = m_DepthAttachment;
        Renderer::Submit([id, color, depth](){
            if (id)
                glDeleteFramebuffers(1, &id);
            if (color)
                glDeleteTextures(1, &color);
            if (depth)
                glDeleteTextures(1, &depth);
        });
    }

    void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height, bool forceRecreate)
    {
        if ((m_Specification.Width == width && m_Specification.Height == height) && !forceRecreate)
            return;

        m_Specification.Width = width;
        m_Specification.Height = height;

        Ref<OpenGLFramebuffer> instance = this;

        Renderer::Submit([instance]() mutable
            {
                if (instance->m_RendererID)
                {
                    glDeleteTextures(1, &instance->m_ColorAttachment);
                    glDeleteTextures(1, &instance->m_DepthAttachment);
                    glDeleteFramebuffers(1, &instance->m_RendererID);
                    instance->m_RendererID = 0;
                    instance->m_ColorAttachment = 0;
                    instance->m_DepthAttachment = 0;
                }

                auto& spec = instance->m_Specification;
                glCreateFramebuffers(1, &instance->m_RendererID);

                if (spec.Format == FramebufferFormat::Depth)
                {
                    glCreateTextures(GL_TEXTURE_2D, 1, &instance->m_DepthAttachment);
                    glTextureStorage2D(instance->m_DepthAttachment, 1, GL_DEPTH_COMPONENT32, spec.Width, spec.Height);
                    glTextureParameteri(instance->m_DepthAttachment, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTextureParameteri(instance->m_DepthAttachment, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTextureParameteri(instance->m_DepthAttachment, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTextureParameteri(instance->m_DepthAttachment, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTextureParameteri(instance->m_DepthAttachment, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                    glTextureParameteri(instance->m_DepthAttachment, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

                    glNamedFramebufferTexture(instance->m_RendererID, GL_DEPTH_ATTACHMENT, instance->m_DepthAttachment, 0);
                    glNamedFramebufferDrawBuffer(instance->m_RendererID, GL_NONE);
                    glNamedFramebufferReadBuffer(instance->m_RendererID, GL_NONE);

                    GLenum fbStatus = glCheckNamedFramebufferStatus(instance->m_RendererID, GL_FRAMEBUFFER);
                    PR_CORE_ASSERT(fbStatus == GL_FRAMEBUFFER_COMPLETE, "Depth Framebuffer incomplete!");
                    return;
                }

                bool multisample = spec.Samples > 1;
                if (multisample)
                {
                    glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &instance->m_ColorAttachment);
                    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, instance->m_ColorAttachment);

                    if (spec.Format == FramebufferFormat::RGBA16F)
                        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, spec.Samples, GL_RGBA16F, spec.Width, spec.Height, GL_FALSE);
                    else if (spec.Format == FramebufferFormat::RGBA8)
                        glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, spec.Samples, GL_RGBA8, spec.Width, spec.Height, GL_FALSE);

                    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

                    glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &instance->m_DepthAttachment);
                    glTextureStorage2DMultisample(instance->m_DepthAttachment, spec.Samples, GL_DEPTH24_STENCIL8, spec.Width, spec.Height, GL_FALSE);
                    glNamedFramebufferTexture(instance->m_RendererID, GL_DEPTH_STENCIL_ATTACHMENT, instance->m_DepthAttachment, 0);
                    glNamedFramebufferTexture(instance->m_RendererID, GL_COLOR_ATTACHMENT0, instance->m_ColorAttachment, 0);
                }
                else
                {
                    glCreateTextures(GL_TEXTURE_2D, 1, &instance->m_ColorAttachment);
                    glTextureStorage2D(instance->m_ColorAttachment, 1, spec.Format == FramebufferFormat::RGBA16F ? GL_RGBA16F : GL_RGBA8, spec.Width, spec.Height);
                    glTextureParameteri(instance->m_ColorAttachment, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTextureParameteri(instance->m_ColorAttachment, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glNamedFramebufferTexture(instance->m_RendererID, GL_COLOR_ATTACHMENT0, instance->m_ColorAttachment, 0);

                    glCreateTextures(GL_TEXTURE_2D, 1, &instance->m_DepthAttachment);
                    glTextureStorage2D(instance->m_DepthAttachment, 1, GL_DEPTH24_STENCIL8, spec.Width, spec.Height);
                    glNamedFramebufferTexture(instance->m_RendererID, GL_DEPTH_STENCIL_ATTACHMENT, instance->m_DepthAttachment, 0);
                }

                GLenum fbStatus = glCheckNamedFramebufferStatus(instance->m_RendererID, GL_FRAMEBUFFER);
                PR_CORE_ASSERT(fbStatus == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");
            });
    }

    void OpenGLFramebuffer::Bind() const
    {
        Ref<const OpenGLFramebuffer> instance = this;
        Renderer::Submit([instance](){
            glBindFramebuffer(GL_FRAMEBUFFER, instance->m_RendererID);
            glViewport(0, 0, instance->m_Specification.Width, instance->m_Specification.Height);
        });
    }

    void OpenGLFramebuffer::Unbind() const
    {
        Ref<const OpenGLFramebuffer> instance = this;
        Renderer::Submit([instance](){
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        });
    }

    void OpenGLFramebuffer::BindTexture(uint32_t slot) const
    {
        Ref<const OpenGLFramebuffer> instance = this;
        Renderer::Submit([instance, slot](){
            glBindTextureUnit(slot, instance->m_ColorAttachment);
        });
    }

    void OpenGLFramebuffer::BindDepthTexture(uint32_t slot ) const
    {
        Ref<const OpenGLFramebuffer> instance = this;
        Renderer::Submit([instance, slot](){
            glBindTextureUnit(slot, instance->m_DepthAttachment);
        });
    }


}
