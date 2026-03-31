#include "Sandbox2D.h"

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Sandbox2D::Sandbox2D()
	:Layer("Sandbox2D"), m_CameraController(1920.0f / 1080.0f, true)
{

}

void Sandbox2D::OnAttach()
{
	Layer::OnAttach();
	PR_CORE_TRACE("Shader Library: {0} \n", m_ShaderLibrary);
}

void Sandbox2D::OnDetach()
{
	Layer::OnDetach();
}

void Sandbox2D::OnUpdate()
{
	Layer::OnUpdate();
	m_CameraController.OnUpdata();
	// 渲染
	Prism::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 0.1f });
	Prism::RenderCommand::Clear();

	Prism::Renderer2D::BeginScene(m_CameraController.GetCamera());

	Prism::Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 1.0f, 1.0f }, m_SquareColor);
	Prism::Renderer2D::EndScene();

	/*glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f));
	m_Shader->GetOriginalShader()->Bind();
	m_Shader->GetOriginalShader()->SetFloat3("_MainColor", m_SquareColor);
	Prism::Renderer::Submit(m_Shader->GetOriginalShader(), m_SquareVA, scale);*/

}

void Sandbox2D::OnEvent(Prism::Event& event)
{
	Layer::OnEvent(event);
	Prism::EventDispatcher dispatcher(event);
	m_CameraController.OnEvent(event);
}

void Sandbox2D::OnImGuiRender()
{
	Layer::OnImGuiRender();
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}
