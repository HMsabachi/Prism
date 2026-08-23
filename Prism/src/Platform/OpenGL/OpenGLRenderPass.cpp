#include "prpch.h"
#include "OpenGLRenderPass.h"
#include "OpenGLImage.h"
#include "Buffer/OpenGLUniformBuffer.h"
#include "Buffer/OpenGLShaderStorageBuffer.h"
#include "Buffer/OpenGLFrameBuffer.h"

#include "Prism/Renderer/Renderer.h"
#include "Prism/ShaderCompiler/PrismBindings.h"

#include <glad/glad.h>

namespace Prism {

    OpenGLRenderPass::OpenGLRenderPass(const RenderPassSpecification& spec)
        : m_Specification(spec)
    {

    }

    OpenGLRenderPass::~OpenGLRenderPass()
    {

    }


    void OpenGLRenderPass::SetInput(uint32_t binding, const Ref<Image2D>& image)
    {
        Ref<OpenGLRenderPass> instance = this;
        Renderer::Submit([instance, binding, image]() mutable {
            instance->m_Image2Ds[binding] = image.As<OpenGLImage2D>();
        });
    }
    void OpenGLRenderPass::SetInput(uint32_t binding, const Ref<ImageCube>& texture)
    {
        Ref<OpenGLRenderPass> instance = this;
        Renderer::Submit([instance, binding, texture]() mutable {
            instance->m_ImageCubes[binding] = texture.As<OpenGLImageCube>();
        });
    }
    void OpenGLRenderPass::SetInput(uint32_t binding, const Ref<UniformBuffer>& ubo)
    {
        Ref<OpenGLRenderPass> instance = this;
        Renderer::Submit([instance, binding, ubo]() mutable {
            instance->m_UniformBuffers[binding] = ubo.As<OpenGLUniformBuffer>();
        });
    }
    void OpenGLRenderPass::SetInput(uint32_t binding, const Ref<ShaderStorageBuffer>& ssbo)
    {
        Ref<OpenGLRenderPass> instance = this;
        Renderer::Submit([instance, binding, ssbo]() mutable {
            instance->m_ShaderStorageBuffers[binding] = ssbo.As<OpenGLShaderStorageBuffer>();
        });
    }

    Ref<Image2D> OpenGLRenderPass::GetOutput(uint32_t index) const
    {
        return m_Specification.TargetFramebuffer->GetImage(index);
    }
    Ref<Image2D> OpenGLRenderPass::GetDepthOutput() const
    {
        return m_Specification.TargetFramebuffer->GetDepthImage();
    }
    Ref<Framebuffer> OpenGLRenderPass::GetTargetFramebuffer() const
    {
        return m_Specification.TargetFramebuffer;
    }

    void OpenGLRenderPass::Bake() {} // NODE: 在OpenGL中，不需要显式地烘焙渲染通道

    void OpenGLRenderPass::RT_BindInputs() const
    {
        for (const auto& [binding, image] : m_Image2Ds)
        {
            if (!image) continue;
            uint32_t unit = Config::GL_TEX_BASE_RENDER_PASS + binding;
            glBindTextureUnit(unit, image->GetRendererID());
        }
        for (const auto& [binding, texture] : m_ImageCubes)
        {
            if (!texture) continue;
            uint32_t unit = Config::GL_TEX_BASE_RENDER_PASS + binding;
            glBindTextureUnit(unit, texture->GetRendererID());
        }
        for (const auto& [binding, ubo] : m_UniformBuffers)
        {
            if (!ubo) continue;
            uint32_t point = Config::GL_UBO_BASE_RENDER_PASS_NEW + binding;
            glBindBufferBase(GL_UNIFORM_BUFFER, point, ubo->GetRendererID());
        }
        for (const auto& [binding, ssbo] : m_ShaderStorageBuffers)
        {
            if (!ssbo) continue;
            uint32_t point = Config::GL_UBO_BASE_RENDER_PASS_NEW + binding;
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, point, ssbo->GetRendererID());
        }
    }

}
