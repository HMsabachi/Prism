#pragma once

#include "Prism/Core.h"

namespace Prism
{
	enum class RendererAPI
	{
		None = 0,
		OpenGL = 1
	};

	class PRISM_API Renderer
	{
	public:
		inline static RendererAPI GetAPI() { return s_RendererAPI; }
	private:
		static RendererAPI s_RendererAPI;
	};
}