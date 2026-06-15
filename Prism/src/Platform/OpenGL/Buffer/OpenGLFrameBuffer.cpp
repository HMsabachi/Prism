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

        auto rendererID = m_RendererID;
        auto colorAttachment = m_ColorAttachment;
        auto depthAttachment = m_DepthAttachment;
        auto spec = m_Specification;

        Renderer::Submit([rendererID, colorAttachment, depthAttachment, spec]() mutable
            {
                GLuint localRendererID = rendererID;
                GLuint localColor = colorAttachment;
                GLuint localDepth = depthAttachment;

                if (localRendererID)
                {
                    glDeleteTextures(1, &localColor);
                    glDeleteTextures(1, &localDepth);
                    glDeleteFramebuffers(1, &localRendererID);
                    localRendererID = 0;
                    localColor = 0;
                    localDepth = 0;
                }

                glCreateFramebuffers(1, &localRendererID);

                if (spec.Format == FramebufferFormat::Depth)
                {
                    glCreateTextures(GL_TEXTURE_2D, 1, &localDepth);
                    glTextureStorage2D(localDepth, 1, GL_DEPTH_COMPONENT32, spec.Width, spec.Height);
                    glTextureParameteri(localDepth, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTextureParameteri(localDepth, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTextureParameteri(localDepth, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTextureParameteri(localDepth, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTextureParameteri(localDepth, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                    glTextureParameteri(localDepth, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

                    glNamedFramebufferTexture(localRendererID, GL_DEPTH_ATTACHMENT, localDepth, 0);
                    glNamedFramebufferDrawBuffer(localRendererID, GL_NONE);
                    glNamedFramebufferReadBuffer(localRendererID, GL_NONE);

                    GLenum fbStatus = glCheckNamedFramebufferStatus(localRendererID, GL_FRAMEBUFFER);
                    PR_CORE_ASSERT(fbStatus == GL_FRAMEBUFFER_COMPLETE, "深度 Framebuffer 不完整!");
                    return;
                }

                glBindFramebuffer(GL_FRAMEBUFFER, localRendererID);

                bool multisample = spec.Samples > 1;
                if (multisample)
                {
                    glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &localColor);
                    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, localColor);

                    if (spec.Format == FramebufferFormat::RGBA16F)
                    {
                        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, spec.Samples, GL_RGBA16F, spec.Width, spec.Height, GL_FALSE);
                    }
                    else if (spec.Format == FramebufferFormat::RGBA8)
                    {
                        glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, spec.Samples, GL_RGBA8, spec.Width, spec.Height, GL_FALSE);
                    }
                    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
                }
                else
                {
                    glCreateTextures(GL_TEXTURE_2D, 1, &localColor);
                    glTextureStorage2D(localColor, 1, spec.Format == FramebufferFormat::RGBA16F ? GL_RGBA16F : GL_RGBA8, spec.Width, spec.Height);
                    glTextureParameteri(localColor, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTextureParameteri(localColor, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glNamedFramebufferTexture(localRendererID, GL_COLOR_ATTACHMENT0, localColor, 0);
                }

                if (multisample)
                {
                    glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &localDepth);
                    glTextureStorage2DMultisample(localDepth, spec.Samples, GL_DEPTH24_STENCIL8, spec.Width, spec.Height, GL_FALSE);
                    glNamedFramebufferTexture(localRendererID, GL_DEPTH_STENCIL_ATTACHMENT, localDepth, 0);
                }
                else
                {
                    glCreateTextures(GL_TEXTURE_2D, 1, &localDepth);
                    glTextureStorage2D(localDepth, 1, GL_DEPTH24_STENCIL8, spec.Width, spec.Height);
                    glNamedFramebufferTexture(localRendererID, GL_DEPTH_STENCIL_ATTACHMENT, localDepth, 0);
                }
                if (multisample)
                    glNamedFramebufferTexture(localRendererID, GL_COLOR_ATTACHMENT0, localColor, 0);

                GLenum fbStatus = glCheckNamedFramebufferStatus(localRendererID, GL_FRAMEBUFFER);
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
