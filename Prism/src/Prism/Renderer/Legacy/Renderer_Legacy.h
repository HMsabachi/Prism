#pragma once
// 这个文件即将弃用
#include "Prism/Core/Core.h"
#include "Prism/Renderer/RenderCommand.h"
#include "../Shader/GlobalUniforms.h"

namespace Prism
{
	class OrthographicCamera;
	class Shader;
}

namespace Prism
{
	/// <summary>
	/// 即将被弃用
	/// </summary>
	class PRISM_API Renderer_Legacy
	{
	public:
		static void Init();
		static void OnWindowResize(uint32_t width, uint32_t height);
		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();
		static void Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

		inline static RendererAPI_Legacy::API GetAPI() { return RendererAPI_Legacy::GetAPI(); }
	public:
		static void UpdateGlobalUniform(const Prism::OrthographicCamera& camera);

	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};
		static PrismGlobalsUBO s_GlobalUBO;

		static SceneData* m_SceneData;
	};
}