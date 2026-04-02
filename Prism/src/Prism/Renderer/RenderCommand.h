#pragma once

#include "Prism/Core/Core.h"
#include "Legacy/RendererAPI_Legacy.h"

#include <glm/glm.hpp>
// 前向声明
namespace Prism {
	class RendererAPI_Legacy;
	class VertexArray;
}

namespace Prism
{
	class PRISM_API RenderCommand
	{
	public:
#pragma region 即将弃用
		inline static void Init()
		{
			s_RendererAPI->Init();
		}
		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}
		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}
		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}
		inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
		{
			s_RendererAPI->DrawIndexed(vertexArray);
		}
#pragma endregion

	public:
		static unsigned int Clear(void* datablock);
	private:
		static RendererAPI_Legacy* s_RendererAPI;
	};
}