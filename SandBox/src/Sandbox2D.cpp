#include "Sandbox2D.h"

#include "imgui/imgui.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"



Sandbox2D::Sandbox2D()
	:Layer("Sandbox2D"), m_CameraController(1920.0f / 1080.0f, true)
{

}

void Sandbox2D::OnAttach()
{
	Layer::OnAttach();

	// 创建VertexArray 2
	m_SquareVA = Prism::VertexArray::Create();
	float squareVertices[5 * 4] = {
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
		-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
	};
	Prism::Ref<Prism::VertexBuffer> squareVB;
	squareVB.reset(Prism::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
	squareVB->SetLayout({
		{ Prism::ShaderDataType::Float3, "a_Position", Prism::VertexSemantic::Position },
		{ Prism::ShaderDataType::Float2, "a_TexCoord", Prism::VertexSemantic::TexCoord0}
		});
	m_SquareVA->AddVertexBuffer(squareVB);
	uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
	Prism::Ref<Prism::IndexBuffer> squareIB;
	squareIB.reset(Prism::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
	m_SquareVA->SetIndexBuffer(squareIB);

	std::string flatColorShader = "Assets/Shaders/FlatColor.glsl";
	auto m_FlatColorShader = m_ShaderLibrary.Load(flatColorShader);

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

	Prism::Renderer::BeginScene(m_CameraController.GetCamera());

	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f));

	auto& FlatColorShader = m_ShaderLibrary.Get("Custom/FlatColor");
	FlatColorShader->GetOriginalShader()->Bind();
	FlatColorShader->GetOriginalShader()->SetFloat3("_MainColor", m_SquareColor);
	
	Prism::Renderer::Submit(FlatColorShader->GetOriginalShader(), m_SquareVA, scale);

	Prism::Renderer::EndScene();

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
	ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}
