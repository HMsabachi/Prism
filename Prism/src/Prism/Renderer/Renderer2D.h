#pragma once
#include "Prism/Core/Core.h"
#include "Camera/OrthographicCamera.h"
#include "Renderer.h"

namespace Prism
{
	class PRISM_API Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
	private:

		static PrismGlobalsUBO s_GlobalUBO;

	};
}