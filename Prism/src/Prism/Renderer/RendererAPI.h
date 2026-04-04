#pragma once

namespace Prism
{
	using RendererID = uint32_t;

	enum class PRISM_API RendererAPIType
	{
		None = 0,
		OpenGL = 1
	};

	struct PRISM_API RenderAPICapabilities
	{
		std::string Vendor;
		std::string Renderer;
		std::string Version;

		int MaxSamples;
		float MaxAnisotropy;
	};

	class PRISM_API RendererAPI
	{
	private:

	public:
		static void Init();
		static void Shutdown();
		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
		static void Clear(float r, float g, float b, float a);
		static void SetClearColor(float r, float g, float b, float a);
		static void DrawIndexed(unsigned int const, bool depthTest = true);

		static RenderAPICapabilities& GetCapabilities()
		{
			static RenderAPICapabilities capabilities;
			return capabilities;
		}

		static RendererAPIType Current() { return s_CurrentRendererAPI; }
	private:
		static RendererAPIType s_CurrentRendererAPI;
	};
}