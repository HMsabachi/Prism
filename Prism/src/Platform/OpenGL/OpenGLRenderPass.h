#pragma once
#include "Prism/Renderer/RenderPass.h"

#include "Prism/Utilities/StaticVector.h"

namespace Prism {
    class OpenGLImage2D;
    class OpenGLImageCube;
    class OpenGLUniformBuffer;
    class OpenGLShaderStorageBuffer;
    class OpenGLFramebuffer;
}

namespace Prism {

    class OpenGLRenderPass : public RenderPass
    {
    public:
        OpenGLRenderPass(const RenderPassSpecification& spec);
        virtual ~OpenGLRenderPass();

        virtual RenderPassSpecification& GetSpecification() override { return m_Specification; }
        virtual const RenderPassSpecification& GetSpecification() const override { return m_Specification; }

        virtual void SetInput(uint32_t binding, const Ref<Image2D>& image) override;
        virtual void SetInput(uint32_t binding, const Ref<ImageCube>& texture) override;
        virtual void SetInput(uint32_t binding, const Ref<UniformBuffer>& ubo) override;
        virtual void SetInput(uint32_t binding, const Ref<ShaderStorageBuffer>& ssbo) override;

        virtual Ref<Image2D> GetOutput(uint32_t index) const override;
        virtual Ref<Image2D> GetDepthOutput() const override;
        virtual Ref<Framebuffer> GetTargetFramebuffer() const override;
        virtual void Bake() override;

        void RT_BindInputs() const;

    private:
        RenderPassSpecification m_Specification;
        std::unordered_map<uint32_t, Ref<OpenGLImage2D>> m_Image2Ds;
        std::unordered_map<uint32_t, Ref<OpenGLImageCube>> m_ImageCubes;
        std::unordered_map<uint32_t, Ref<OpenGLUniformBuffer>> m_UniformBuffers;
        std::unordered_map<uint32_t, Ref<OpenGLShaderStorageBuffer>> m_ShaderStorageBuffers;
    };

}
