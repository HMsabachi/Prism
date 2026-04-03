#pragma once

namespace Prism
{
	using RendererID = unsigned int;

	enum class PRISM_API RendererAPIType
	{
		None = 0,
		OpenGL = 1
	};

	class PRISM_API RendererAPI
	{
	private:

	public:
		static void Init();
		static void Shutdown();
		static void Clear(float r, float g, float b, float a);
		static void SetClearColor(float r, float g, float b, float a);
		static void DrawIndexed(unsigned int const);

		static RendererAPIType Current() { return s_CurrentRendererAPI; }
	private:
		static RendererAPIType s_CurrentRendererAPI;
	};
}