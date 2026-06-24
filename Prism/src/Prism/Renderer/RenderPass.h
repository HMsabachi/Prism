#pragma once

#include "Prism/Core/Core.h"

namespace Prism {

    class Framebuffer;

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

        static Ref<RenderPass> Create(const RenderPassSpecification& spec);
    };

}