#include "prpch.h"
#include "OpenGLContect.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <GL/GL.h>

namespace Prism {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		PR_CORE_ASSERT(windowHandle, "Window handle is null!")
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		PR_CORE_ASSERT(status, "Failed to initialize Glad!");

		PR_CORE_INFO("OpenGL 信息 OpenGL Info:");
		PR_CORE_INFO("  设备商 Vendor: {0}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
		PR_CORE_INFO("  显卡 Renderer: {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
		PR_CORE_INFO("  版本 Version: {0}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}

}