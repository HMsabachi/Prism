#include "prpch.h"
#include "Prism/Renderer/RendererAPI.h"

#include <Glad/glad.h>

namespace Prism
{

	static void OpenGLLogMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
			PR_CORE_ERROR("{0}", message);
	}

	void RendererAPI::Init()
	{
		glDebugMessageCallback(OpenGLLogMessage, nullptr); // 启用Debug输出
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

		// TODO: 临时创建VAO
		unsigned int vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glEnable(GL_DEPTH_TEST); // 启用深度测试
		//glEnable(GL_CULL_FACE);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // 启用 seamless cubemap
		glFrontFace(GL_CCW); // 设置默认的正面朝向

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		auto& caps = RendererAPI::GetCapabilities();

		caps.Vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
		caps.Renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
		caps.Version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

		glGetIntegerv(GL_MAX_SAMPLES, &caps.MaxSamples); // 获取最大采样数
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &caps.MaxAnisotropy); // 获取最大各向异性
	}

	void RendererAPI::Shutdown()
	{
	}

	void RendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void RendererAPI::Clear(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void RendererAPI::SetClearColor(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
	}

	void RendererAPI::DrawIndexed(unsigned int count, bool depthTest)
	{
		if (depthTest)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

}