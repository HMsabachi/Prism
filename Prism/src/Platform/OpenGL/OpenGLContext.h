#pragma once

#include "Prism/Renderer/RendererContext.h"

struct GLFWwindow;

namespace Prism
{
    class PRISM_API OpenGLContext : public RendererContext
    {
    public:
        OpenGLContext(GLFWwindow* windowHandle);
        virtual ~OpenGLContext();

        virtual void Create() override;
        virtual void BeginFrame() override {}
        virtual void SwapBuffers() override;
        virtual void OnResize(uint32_t width, uint32_t height) override {}
    private:
        GLFWwindow* m_WindowHandle;
    };
}
