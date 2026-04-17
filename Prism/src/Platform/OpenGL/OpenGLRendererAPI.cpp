#include "prpch.h"
#include "Prism/Renderer/RendererAPI.h"

#include "OpenGLStateCache.h"

#include <Glad/glad.h>

namespace Prism
{
	static GLbitfield PrismToOpenGLMemoryBarrier(RendererAPI::BarrierFlags flags)
	{
		GLbitfield result = 0;
		if (flags & RendererAPI::Barrier::ShaderStorage) result |= GL_SHADER_STORAGE_BARRIER_BIT;
		if (flags & RendererAPI::Barrier::VertexAttribArray) result |= GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT;
		if (flags & RendererAPI::Barrier::ElementArray) result |= GL_ELEMENT_ARRAY_BARRIER_BIT;
		if (flags & RendererAPI::Barrier::ImageAccess) result |= GL_FRAMEBUFFER_BARRIER_BIT;
		if (flags & RendererAPI::Barrier::TextureFetch) result |= GL_TEXTURE_FETCH_BARRIER_BIT;
		if (flags & RendererAPI::Barrier::TextureUpdate) result |= GL_TEXTURE_UPDATE_BARRIER_BIT;
		if (flags & RendererAPI::Barrier::Framebuffer) result |= GL_FRAMEBUFFER_BARRIER_BIT;
		if (flags & RendererAPI::Barrier::Command) result |= GL_COMMAND_BARRIER_BIT;
		if (flags & RendererAPI::Barrier::All) result |= GL_ALL_BARRIER_BITS;
		return result;
	}

	static void HandleCapabilities()
	{
		auto& caps = RendererAPI::GetCapabilities();

		caps.Vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR)); // 获取 vendor
		caps.Renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER)); // 获取 renderer
		caps.Version = reinterpret_cast<const char*>(glGetString(GL_VERSION)); // 获取版本

		glGetIntegerv(GL_MAX_SAMPLES, &caps.MaxSamples); // 获取最大采样数
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &caps.MaxAnisotropy); // 获取最大各向异性

		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &caps.MaxGroupCount[0]); // 获取最大工作组数量
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &caps.MaxGroupSize[0]); // 获取最大工作组大小
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &caps.MaxInvocations); // 
	}

	static void OpenGLLogMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:
			PR_CORE_ERROR("[OpenGL Debug HIGH] {0}", message);
			PR_CORE_ASSERT(false, "GL_DEBUG_SEVERITY_HIGH");
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			PR_CORE_WARN("[OpenGL Debug MEDIUM] {0}", message);
			break;
		case GL_DEBUG_SEVERITY_LOW:
			PR_CORE_INFO("[OpenGL Debug LOW] {0}", message);
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			// PR_CORE_TRACE("[OpenGL Debug NOTIFICATION] {0}", message);
			break;
		}
	}

	void RendererAPI::Init()
	{
		glDebugMessageCallback(OpenGLLogMessage, nullptr); // 启用Debug输出
		glEnable(GL_DEBUG_OUTPUT);
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

		HandleCapabilities();

		GLenum error = glGetError();
		while (error != GL_NO_ERROR)
		{
			PR_CORE_ERROR("OpenGL Error {0}", error);
			error = glGetError();
		}

		OpenGLStateCache::Init();

		LoadRequiredAssets();
	}

	void RendererAPI::LoadRequiredAssets()
	{
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

	void RendererAPI::DrawIndexed(unsigned int count, bool depthTest) // 默认启用深度测试
	{
		if(!depthTest)
			glDisable(GL_DEPTH_TEST);
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
		if (!depthTest)
			glEnable(GL_DEPTH_TEST);
	}

	void RendererAPI::MemoryBarriers(BarrierFlags flags)
	{
		glMemoryBarrier(PrismToOpenGLMemoryBarrier(flags));
	}

}