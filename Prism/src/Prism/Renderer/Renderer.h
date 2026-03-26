#pragma once
#include "Prism/Core/Core.h"
#include "Prism/Renderer/RenderCommand.h"

namespace Prism
{
	class OrthographicCamera;
	class Shader;
}

namespace Prism
{

	class PRISM_API Renderer
	{
	public:
		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();
		static void Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* m_SceneData;
	};
}