#include "prpch.h"
#include "Renderer_Legacy.h"
#include "../Renderer2D.h"
#include "../Shader/Shader.h"
#include "../RenderCommand.h"
#include "../Camera/OrthographicCamera.h"
#include "../VertexArray.h"
#include "../Texture.h"

#include "Prism/Core/Time.h"
#include "Prism/Core/Application.h"


#include "Platform/OpenGL/Shader/OpenGLShader.h"
// TODO: 暂时使用OpenGLShader

namespace Prism
{
	Renderer_Legacy::SceneData* Renderer_Legacy::m_SceneData = new Renderer_Legacy::SceneData;
	PrismGlobalsUBO Renderer_Legacy::s_GlobalUBO{};
	//Ref<GlobalUniforms> Renderer::s_GlobalUniforms = GlobalUniforms::Create();

	void Renderer_Legacy::Init()
	{
		PR_PROFILE_FUNCTION();

		Renderer2D::Init();
		RenderCommand::Init();
		GlobalUniforms::Init();
		Texture2D::Init();
	}
	void Renderer_Legacy::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}
	void Renderer_Legacy::BeginScene(OrthographicCamera& camera)
	{
		m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();

		UpdateGlobalUniform(camera);
	}
	
	void Renderer_Legacy::EndScene()
	{
	}
	void Renderer_Legacy::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		shader->Bind();
		shader->SetMat4("Prism_Model", transform);
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}


	void Renderer_Legacy::UpdateGlobalUniform(const Prism::OrthographicCamera& camera)
	{
		PR_PROFILE_FUNCTION();

		float totalTime = Time::GetTime();
		float deltaTime = Time::GetDeltaTime();

		s_GlobalUBO.Time = glm::vec4(totalTime / 20.0f, totalTime, totalTime * 2.0f, totalTime * 3.0f);
		s_GlobalUBO.DeltaTime = deltaTime;
		s_GlobalUBO.CameraPosition = camera.GetPosition();
		s_GlobalUBO.View = camera.GetViewMatrix();
		s_GlobalUBO.Projection = camera.GetProjectionMatrix();
		s_GlobalUBO.ViewProjection = s_GlobalUBO.Projection * s_GlobalUBO.View;
		s_GlobalUBO.Resolution = glm::vec2(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
		s_GlobalUBO.AspectRatio = s_GlobalUBO.Resolution.x / s_GlobalUBO.Resolution.y;
		GlobalUniforms::UpdateGlobalUniform(s_GlobalUBO);
	}
}