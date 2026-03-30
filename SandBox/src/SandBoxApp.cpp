#include <Prism.h>
#include "../../Prism/src/Platform/OpenGL/OpenGLTexture.h"


#include "imgui/imgui.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class ExampleLayer : public Prism::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_CameraController(1980.0f / 1080.f, true)
	{
		// 创建VertexArray 1
		m_VertexArray.reset(Prism::VertexArray::Create());
		float vertices[] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f, 1.0f,
			0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f, 1.0f,
			0.0f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f, 1.0f
		};
		Prism::Ref<Prism::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Prism::VertexBuffer::Create(vertices, sizeof(vertices)));
		Prism::BufferLayout layout = {
			{Prism::ShaderDataType::Float3, "aPos", Prism::VertexSemantic::Position},
			{Prism::ShaderDataType::Float3, "aNormal", Prism::VertexSemantic::Normal},
			{Prism::ShaderDataType::Float2, "aUV", Prism::VertexSemantic::TexCoord0 },

		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);
		uint32_t indeces[] = { 0, 1, 2 };
		Prism::Ref<Prism::IndexBuffer> indexBuffer;
		indexBuffer.reset(Prism::IndexBuffer::Create(indeces, sizeof(indeces) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		// 创建VertexArray 2
		m_SquareVA.reset(Prism::VertexArray::Create());
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

		// Shader
		std::string Src = "Assets/Shaders/FristShader.glsl";
		m_Shader = m_ShaderLibrary.Load(Src);
		PR_TRACE("Prism Shader Property {0}", m_Shader->GetProperties());

		std::string flatColorShader = "Assets/Shaders/FlatColor.glsl";
		auto m_FlatColorShader = m_ShaderLibrary.Load(flatColorShader);

		std::string textureShader = "Assets/Shaders/Texture.glsl";
		m_TextureShader = m_ShaderLibrary.Load(textureShader);

		m_TestTexture = Prism::Texture2D::Create("Assets/Textures/TestImage.png");
		std::dynamic_pointer_cast<Prism::OpenGLTexture2D>(m_TestTexture)->Bind();
		m_TextureShader->GetOriginalShader()->SetInt("_MainTex", 0);
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}
	void OnUpdate() override
	{
		m_CameraController.OnUpdata();
		// 渲染
		Prism::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 0.1f });
		Prism::RenderCommand::Clear();

		Prism::Renderer::BeginScene(m_CameraController.GetCamera());

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		auto& FlatColorShader = m_ShaderLibrary.Get("Custom/FlatColor");
		FlatColorShader->GetOriginalShader()->Bind();
		FlatColorShader->GetOriginalShader()->SetFloat3("_MainColor", m_SquareColor);
		

		for (int i = 0; i < 20; i++)
		{
			for (int j = 0; j < 20; j++)
			{
				glm::vec3 pos(i * 0.11f, j * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Prism::Renderer::Submit(FlatColorShader->GetOriginalShader(), m_SquareVA, transform);
			}
		}
		Prism::Renderer::Submit(m_TextureShader->GetOriginalShader(), m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		// 三角形 Triangle
		Prism::Renderer::Submit(m_Shader->GetOriginalShader(), m_VertexArray);

		Prism::Renderer::EndScene();
	}

	void OnEvent(Prism::Event& event) override
	{
		Prism::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Prism::KeyPressedEvent>(PR_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));
		m_CameraController.OnEvent(event);
	}
#pragma region 事件处理 Event Handling
	private:
		bool OnKeyPressedEvent(Prism::KeyPressedEvent& e)
		{
			if (e.GetKeyCode() == PR_KEY_T)
				PR_TRACE("Now delta time is {0}", Prism::Time::GetDeltaTime());
			return false;
		}
#pragma endregion
	
private:
	Prism::ShaderLibrary m_ShaderLibrary;
	Prism::Ref<Prism::PrismShader> m_Shader, m_TextureShader;
	Prism::Ref<Prism::VertexArray> m_VertexArray;

	Prism::Ref<Prism::VertexArray> m_SquareVA;
	Prism::Ref<Prism::Texture2D> m_TestTexture;

	Prism::OrthographicCameraController m_CameraController;
	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};

class Sandbox : public Prism::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}
	~Sandbox()
	{

	}
};

Prism::Application* Prism::CreateApplication()
{
	return new Sandbox();
}