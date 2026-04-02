#pragma once

#include "RenderCommandQueue.h"
#include "Legacy/RendererAPI_Legacy.h"

namespace Prism
{
	class PRISM_API Renderer
	{
	private:

	public:
		static void Clear();
		static void Clear(float r, float g, float b, float a = 1.0f);
		static void SetClearColor(float r, float g, float b, float a);

		static void ClearMagenta();

		void Init();

		static void Submit(const std::function<void()>& command)
		{

		}
		void WaitAndRender();

		inline static Renderer& Get() { return *s_Instance; }

	private:
		static Renderer* s_Instance;

		RenderCommandQueue m_CommandQueue;
	};
#define PR_RENDER(x) ::Prism::Renderer::Submit([=](){ RendererAPI::x; })
}