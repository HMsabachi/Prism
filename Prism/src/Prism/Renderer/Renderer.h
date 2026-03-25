#pragma once
#include "Prism/Core.h"
#include "Prism/Renderer/RenderCommand.h"

namespace Prism
{

	class PRISM_API Renderer
	{
	public:
		static void BeginScene();
		static void EndScene();
		static void Submit(const std::shared_ptr<VertexArray>& vertexArray);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	};
}