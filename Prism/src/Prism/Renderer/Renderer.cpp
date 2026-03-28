#include "prpch.h"
#include "Renderer.h"
#include "Shader/Shader.h"
#include "RenderCommand.h"
#include "Camera/OrthographicCamera.h"
#include "VertexArray.h"
#include "Texture.h"

#include "Prism/Core/Time.h"
#include "Prism/Application.h"


#include "Platform/OpenGL/Shader/OpenGLShader.h"
// TODO: 暂时使用OpenGLShader

namespace Prism
{
	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;
	PrismGlobalsUBO Renderer::s_GlobalUBO{};
	//Ref<GlobalUniforms> Renderer::s_GlobalUniforms = GlobalUniforms::Create();

	void Renderer::Init()
	{
		RenderCommand::Init();
		GlobalUniforms::Init();
		Texture2D::Init();
	}
	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();

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
	void Renderer::EndScene()
	{
	}
	void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		shader->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("Prism_Model", transform);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}