#pragma once

#include "Prism/Core/Ref.h"

struct GLFWwindow;

namespace Prism {

    class PRISM_API RendererContext : public RefCounted
    {
    public:
        RendererContext() = default;
        virtual ~RendererContext() = default;

        virtual void Create() = 0;
        virtual void BeginFrame() = 0;
        virtual void SwapBuffers() = 0;
        virtual void OnResize(uint32_t width, uint32_t height) = 0;
        virtual void SetVSync(bool enabled) = 0;

        virtual uint32_t GetCurrentFrameIndex() const = 0;
        virtual uint32_t GetImageCount() const = 0;



        static Ref<RendererContext> Create(GLFWwindow* windowHandle);
    };

}
