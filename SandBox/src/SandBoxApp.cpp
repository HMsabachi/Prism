#include <Prism.h>

#include "../../Prism/src/Platform/OpenGL/Shader/OpenGLShader.h"
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
		std::string Src = R"(
// Prism Shader Language v1.0
Shader "Custom/MyFirstShader"
{
    // ==================== Properties（材质参数） ====================
    Properties
    {
        _MainColor("主颜色", Color) = (1, 1, 1, 1)
        _MainTex("基础贴图", Texture2D) = "white" {}
        _Gloss("光泽度", Float) = 0.5
        _Cutoff("透明裁剪", Range(0, 1)) = 0.5
    }
    SubShader
    {
        
        Pass
        {
            Tags { "Queue" = "Geometry" "RenderType" = "Opaque" }
            Name "ForwardBase"
            GLSL
            {
                #include "PrismBuiltin.glsl"
                attribute vec3 aPos : POSITION;
                attribute vec2 aUV : TEXCOORD0;
                attribute vec3 aNormal : NORMAL;
                VARYING vec2 vUV; // Prism 引擎定义的跨阶段变量
                VARYING vec3 vNormal;
                void main()
                {
                    gl_Position = Prism_ViewProjection * Prism_Model * vec4(aPos, 1.0); // Prism_ 前缀是引擎全局变量
                    vUV = aUV;
                }

                void frag()
                {
                    vec4 col = vec4(1.0, 1.0, 1.0, 1.0);
					col.r *= sin(Prism_Time.x * 2.0) * 0.5 + 0.5;
					col.g *= sin(Prism_Time.y * 2.0) * 0.5 + 0.5;
					col.b *= sin(Prism_Time.z * 2.0) * 0.5 + 0.5;
					FragColor = col;
                }
            }
        }
    }
}
)";

		m_Shader = Prism::PrismShader::Create(Src);
		PR_TRACE("Prism Shader Property {0}", m_Shader->GetProperties());
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
			
			out vec4 color;

			in vec3 v_Position;
			uniform vec3 u_Color;

			void main()
			{
				color = vec4(u_Color, 1.0);
			}
		)";

		m_FlatColorShader = Prism::Shader::Create(flatColorShaderVertexSrc, flatColorShaderFragmentSrc);
		std::string textureShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 2) in vec2 a_TexCoord;
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

		m_TextureShader = Prism::Shader::Create(textureShaderVertexSrc, textureShaderFragmentSrc);
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
		Prism::Renderer::Submit(m_Shader->GetOriginalShader(), m_VertexArray);

		Prism::Renderer::EndScene();
	}

	void OnEvent(Prism::Event& event) override
	{
		Prism::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Prism::KeyPressedEvent>(PR_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<Prism::WindowResizeEvent>([&](Prism::WindowResizeEvent& e)
			{
				m_Camera.OnWindowResize(e.GetWidth(), e.GetHeight());
				return false;
			});
		dispatcher.Dispatch<Prism::MouseScrolledEvent>([&](Prism::MouseScrolledEvent& e)
			{
				float offset = e.GetYOffset();
				m_CameraZoomLevel *= 1.0f - offset * m_CameraZoomSpeed;
				m_Camera.SetZoomLevel(m_CameraZoomLevel);
				return false;
			});
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

	Prism::Ref<Prism::PrismShader> m_Shader;
	Prism::Ref<Prism::VertexArray> m_VertexArray;

	Prism::Ref<Prism::Shader> m_FlatColorShader, m_TextureShader;
	Prism::Ref<Prism::VertexArray> m_SquareVA;
	Prism::Ref<Prism::Texture2D> m_TestTexture;

	Prism::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
	float m_CameraRotation = 0.0f;
	float m_CameraMoveSpeed = 3.0f;
	float m_CameraRotationSpeed = 10.0f;
	float m_CameraZoomLevel = 1.0f;
	float m_CameraZoomSpeed = 0.1f;
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