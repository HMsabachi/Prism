#include "prpch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include "Prism/Renderer/Renderer.h"

#include "Prism/Core/Log.h"

namespace Prism {

    OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
        : m_WindowHandle(windowHandle)
    {
        PR_CORE_ASSERT(windowHandle, "Window handle is null!")
    }

    OpenGLContext::~OpenGLContext()
    {
    }

    void OpenGLContext::Create()
    {
        PR_PROFILE_FUNCTION();
        Renderer::Submit([this]() {
            glfwMakeContextCurrent(m_WindowHandle);
            int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
            PR_CORE_ASSERT(status, "Failed to initialize Glad!");

            PR_CORE_INFO("OpenGL 信息 OpenGL Info:");
            PR_CORE_INFO("  设备商 Vendor: {0}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
            PR_CORE_INFO("  显卡 Renderer: {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
            PR_CORE_INFO("  版本 Version: {0}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

#ifdef PR_ENABLE_ASSERTS
            int versionMajor;
            int versionMinor;
            glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
            glGetIntegerv(GL_MINOR_VERSION, &versionMinor);

            PR_CORE_ASSERT(versionMajor > 4 || (versionMajor == 4 && versionMinor >= 5), "Prism requires at least OpenGL version 4.5!");
#endif
        });
    }


    void OpenGLContext::SwapBuffers()
    {
        PR_PROFILE_FUNCTION();

        glfwSwapBuffers(m_WindowHandle);
    }

    void OpenGLContext::SetVSync(bool enabled)
    {
        Renderer::Submit([this, enabled]() {
            glfwSwapInterval(enabled ? 1 : 0);
        });
    }

}
