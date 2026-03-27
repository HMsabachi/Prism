#pragma once

#include "Prism/Core/Core.h"
#include "RendererAPI.h"

#include <glm/glm.hpp>
// 前向声明
namespace Prism {
	class RendererAPI;
	class VertexArray;
}

namespace Prism
{
	class PRISM_API RenderCommand
	{
	public:
		inline static void Init()
		{
			s_RendererAPI->Init();
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
	private:
		static RendererAPI* s_RendererAPI;
	};
}