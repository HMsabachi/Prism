#include <Prism.h>

#include "imgui/imgui.h"

#include "glm/gtc/matrix_transform.hpp"

class ExampleLayer : public Prism::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.0f, 1.0f, -1.0f, 1.0f)
	{
		// 创建VertexArray 1
		m_VertexArray.reset(Prism::VertexArray::Create());
		float vertices[] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			0.0f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};
		std::shared_ptr<Prism::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Prism::VertexBuffer::Create(vertices, sizeof(vertices)));
		Prism::BufferLayout layout = {
			{Prism::ShaderDataType::Float3, "aPos"},
			{Prism::ShaderDataType::Float4, "aColor"}
		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);
		uint32_t indeces[] = { 0, 1, 2 };
		std::shared_ptr<Prism::IndexBuffer> indexBuffer;
		indexBuffer.reset(Prism::IndexBuffer::Create(indeces, sizeof(indeces) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		// 创建VertexArray 2
		m_SquareVA.reset(Prism::VertexArray::Create());
		float squareVertices[3 * 4] = {
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.5f,  0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f
		};
		std::shared_ptr<Prism::VertexBuffer> squareVB;
		squareVB.reset(Prism::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{ Prism::ShaderDataType::Float3, "a_Position" }
			});
		m_SquareVA->AddVertexBuffer(squareVB);
		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		std::shared_ptr<Prism::IndexBuffer> squareIB;
		squareIB.reset(Prism::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);

		// Shader
		std::string vertexSrc = R"(
			#version 330 core
			layout (location = 0) in vec3 aPos;
			layout (location = 1) in vec4 aColor;
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			out vec4 vColor;
			out vec3 vPos;
			void main()
			{
				vPos = aPos;
				vColor = aColor;
				gl_Position = u_ViewProjection * u_Transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);
			}
		)";
		std::string fragmentSrc = R"(
			#version 330 core
			in vec3 vPos;
			in vec4 vColor;
			out vec4 FragColor;
			void main()
			{
				FragColor = vec4(vColor);
			}
		)";
		m_Shader.reset(new Prism::Shader(vertexSrc, fragmentSrc));
		std::string blueShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string blueShaderFragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			void main()
			{
				color = vec4(0.2, 0.3, 0.8, 1.0);
			}
		)";

		m_BlueShader.reset(new Prism::Shader(blueShaderVertexSrc, blueShaderFragmentSrc));
	}

	void OnImGuiRender() override
	{

	}
	void OnUpdate() override
	{
		float delta = Prism::Time::GetDeltaTime();
		if (Prism::Input::IsKeyPressed(PR_KEY_LEFT))
			m_CameraPosition.x -= m_CameraMoveSpeed * delta;
		if (Prism::Input::IsKeyPressed(PR_KEY_RIGHT))
			m_CameraPosition.x += m_CameraMoveSpeed * delta;
		if (Prism::Input::IsKeyPressed(PR_KEY_UP))
			m_CameraPosition.y += m_CameraMoveSpeed * delta;
		if (Prism::Input::IsKeyPressed(PR_KEY_DOWN))
			m_CameraPosition.y -= m_CameraMoveSpeed * delta;
		if (Prism::Input::IsKeyPressed(PR_KEY_A))
			m_CameraRotation += m_CameraRotationSpeed * delta;
		if (Prism::Input::IsKeyPressed(PR_KEY_D))
			m_CameraRotation -= m_CameraRotationSpeed * delta;
		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);
		Prism::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 0.1f });
		Prism::RenderCommand::Clear();

		Prism::Renderer::BeginScene(m_Camera);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		for (int i = 0; i < 20; i++)
		{
			for (int j = 0; j < 20; j++)
			{
				glm::vec3 pos(i * 0.31f, j * 0.31f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Prism::Renderer::Submit(m_BlueShader, m_SquareVA, transform);
			}
		}

		Prism::Renderer::Submit(m_Shader, m_VertexArray);

		Prism::Renderer::EndScene();
	}

	void OnEvent(Prism::Event& event) override
	{
		Prism::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Prism::KeyPressedEvent>(PR_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));

	}
#pragma region 事件处理 Event Handling
	public:
		bool OnKeyPressedEvent(Prism::KeyPressedEvent& e)
		{
			if (e.GetKeyCode() == PR_KEY_T)
				PR_TRACE("Now delta time is {0}", Prism::Time::GetDeltaTime());
			return false;
		}

#pragma endregion

private:

	std::shared_ptr<Prism::Shader> m_Shader;
	std::shared_ptr<Prism::VertexArray> m_VertexArray;

	std::shared_ptr<Prism::Shader> m_BlueShader;
	std::shared_ptr<Prism::VertexArray> m_SquareVA;

	Prism::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
	float m_CameraRotation = 0.0f;
	float m_CameraMoveSpeed = 3.0f;
	float m_CameraRotationSpeed = 10.0f;
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