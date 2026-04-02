#pragma once

namespace Prism
{
	class PRISM_API RendererAPI
	{
	private:

	public:
		static void Clear(float r, float g, float b, float a);
		static void SetClearColor(float r, float g, float b, float a);
	};
}