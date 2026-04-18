#pragma once
#include <cstdint>

namespace Prism 
{
	using RendererID = uint32_t;
	enum class PrimitiveType
	{
		None = 0, Triangles, Lines
	};
}