#include <Prism.h>

#include "../../Prism/src/Platform/OpenGL/OpenGLShader.h"
#include "../../Prism/src/Platform/OpenGL/OpenGLTexture.h"


#include "imgui/imgui.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class ExampleLayer : public Prism::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.7f, 1.7f, -1.0f, 1.0f)
	{
		// 创建VertexArray 1
		m_VertexArray.reset(Prism::VertexArray::Create());
		float vertices[] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			0.0f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};
		Prism::Ref<Prism::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Prism::VertexBuffer::Create(vertices, sizeof(vertices)));
		Prism::BufferLayout layout = {
			{Prism::ShaderDataType::Float3, "aPos"},
			{Prism::ShaderDataType::Float4, "aColor"}
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
			{ Prism::ShaderDataType::Float3, "a_Position" },
			{ Prism::ShaderDataType::Float2, "a_TexCoord" }
			});
		m_SquareVA->AddVertexBuffer(squareVB);
		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Prism::Ref<Prism::IndexBuffer> squareIB;
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
		m_Shader.reset(Prism::Shader::Create(vertexSrc, fragmentSrc));
		std::string flatColorShaderVertexSrc = R"(
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

		std::string flatColorShaderFragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			uniform vec3 u_Color;

			void main()
			{
				color = vec4(u_Color, 1.0);
			}
		)";

		m_FlatColorShader.reset(Prism::Shader::Create(flatColorShaderVertexSrc, flatColorShaderFragmentSrc));
		std::string textureShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec2 a_TexCoord;
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			out vec2 v_TexCoord;

			void main()
			{
				v_TexCoord = a_TexCoord;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string textureShaderFragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec2 v_TexCoord;
			uniform sampler2D u_Texture;

			void main()
			{
				color = texture(u_Texture, v_TexCoord);
			}
		)";

		m_TextureShader.reset(Prism::Shader::Create(textureShaderVertexSrc, textureShaderFragmentSrc));
		m_TestTexture = Prism::Texture2D::Create("Assets/Textures/TestImage.png");
		std::dynamic_pointer_cast<Prism::OpenGLTexture2D>(m_TestTexture)->Bind();
		std::dynamic_pointer_cast<Prism::OpenGLShader>(m_TextureShader)->UploadUniformInt("u_Texture", 0);
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}
	void OnUpdate() override
	{
		HeadleCameraTransform(); // 处理摄像机位置和旋转
		// 渲染
		Prism::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 0.1f });
		Prism::RenderCommand::Clear();

		Prism::Renderer::BeginScene(m_Camera);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		std::dynamic_pointer_cast<Prism::OpenGLShader>(m_FlatColorShader)->Bind();
		std::dynamic_pointer_cast<Prism::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color", m_SquareColor);

		for (int i = 0; i < 20; i++)
		{
			for (int j = 0; j < 20; j++)
			{
				glm::vec3 pos(i * 0.11f, j * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Prism::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
			}
		}
		Prism::Renderer::Submit(m_TextureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		// 三角形 Triangle
		//Prism::Renderer::Submit(m_Shader, m_VertexArray);

		Prism::Renderer::EndScene();
	}

	void OnEvent(Prism::Event& event) override
	{
		Prism::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Prism::KeyPressedEvent>(PR_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));

	}
#pragma region 事件处理 Event Handling
	private:
		bool OnKeyPressedEvent(Prism::KeyPressedEvent& e)
		{
			if (e.GetKeyCode() == PR_KEY_T)
				PR_TRACE("Now delta time is {0}", Prism::Time::GetDeltaTime());
			return false;
		}
		void HeadleCameraTransform()
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
		}
#pragma endregion
	
private:

	Prism::Ref<Prism::Shader> m_Shader;
	Prism::Ref<Prism::VertexArray> m_VertexArray;

	Prism::Ref<Prism::Shader> m_FlatColorShader, m_TextureShader;
	Prism::Ref<Prism::VertexArray> m_SquareVA;
	Prism::Ref<Prism::Texture2D> m_TestTexture;

	Prism::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
	float m_CameraRotation = 0.0f;
	float m_CameraMoveSpeed = 3.0f;
	float m_CameraRotationSpeed = 10.0f;
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