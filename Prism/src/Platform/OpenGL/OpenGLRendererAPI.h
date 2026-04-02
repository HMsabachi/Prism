#pragma once
#include "Prism/Renderer/Legacy/RendererAPI_Legacy.h"

namespace Prism
{
	class OpenGLRendererAPI : public RendererAPI_Legacy
	{
	public:
		virtual void Init() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;
		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;

	};
}