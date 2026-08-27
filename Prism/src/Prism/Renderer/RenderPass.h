#pragma once

#include "Prism/Core/Core.h"

namespace Prism {
    class Framebuffer;
    class Image2D;
    class ImageCube;
    class UniformBuffer;
    class ShaderStorageBuffer;
}

namespace Prism {

    struct PRISM_API RenderPassSpecification
    {
        Ref<Framebuffer> TargetFramebuffer;
    };

    class PRISM_API RenderPass : public RefCounted
    {
    public:
        virtual ~RenderPass() {}

        virtual RenderPassSpecification& GetSpecification() = 0;
        virtual const RenderPassSpecification& GetSpecification() const = 0;

        virtual void SetInput(uint32_t binding, const Ref<Image2D>& image) = 0;
        virtual void SetInput(uint32_t binding, const Ref<ImageCube>& texture) = 0;
        virtual void SetInput(uint32_t binding, const Ref<UniformBuffer>& ubo) = 0;
        virtual void SetInput(uint32_t binding, const Ref<ShaderStorageBuffer>& ssbo) = 0;

        virtual Ref<Image2D> GetOutput(uint32_t index = 0) const = 0;
        virtual Ref<Image2D> GetDepthOutput() const = 0;
        virtual Ref<Framebuffer> GetTargetFramebuffer() const = 0;
        virtual void Bake() = 0;

        static Ref<RenderPass> Create(const RenderPassSpecification& spec);
    };

}
