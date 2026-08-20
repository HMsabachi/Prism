#pragma once
#include "Prism/Renderer/RendererContext.h"

namespace Prism
{
    class PRISM_API VulkanContext : public RendererContext
    {
    public:
        VulkanContext();
        virtual ~VulkanContext();
        virtual void Create() override;
        virtual void BeginFrame() override;
        virtual void SwapBuffers() override;
        virtual void OnResize(uint32_t width, uint32_t height) override;
    private:
        // Vulkan specific members would go here
    };
}
