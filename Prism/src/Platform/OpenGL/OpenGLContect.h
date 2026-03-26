#pragma once

#include "Prism/Core/Core.h"

#include "Prism/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Prism
{
	class PRISM_API OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;
	private:
		GLFWwindow* m_WindowHandle;
	};
}

