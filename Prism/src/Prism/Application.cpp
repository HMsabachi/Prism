#include "prpch.h"
#include "Application.h"

#include "Prism/Log.h"
#include "Prism/Input.h"

// TODO: 这里还有未被封装的OpenGL函数 There are still some OpenGL functions not wrapped here
#include <glad/glad.h>

namespace Prism
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)
#define PR_ERROR_COLOR 1, 0, 1, 1

	Application* Application::s_Instance = nullptr;
	
	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:    return GL_FLOAT;
			case ShaderDataType::Float2:   return GL_FLOAT;
			case ShaderDataType::Float3:   return GL_FLOAT;
			case ShaderDataType::Float4:   return GL_FLOAT;
			case ShaderDataType::Mat3:     return GL_FLOAT;
			case ShaderDataType::Mat4:     return GL_FLOAT;
			case ShaderDataType::Int:      return GL_INT;
			case ShaderDataType::Int2:     return GL_INT;
			case ShaderDataType::Int3:     return GL_INT;
			case ShaderDataType::Int4:     return GL_INT;
			case ShaderDataType::Bool:     return GL_BOOL;
		}
		PR_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	Application::Application()
	{
		PR_CORE_ASSERT(!s_Instance, "Application already exists! 应用已经存在");
		s_Instance = this;

		// 初始化窗口 Initialize Window
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
		
		// 下面做一些小测试 Just some small tests below
		// Vertex Array
		// Vertex Buffer
		// Index Buffer
		// Shader

		glGenVertexArrays(1, &m_VertexArray);
		glBindVertexArray(m_VertexArray);

		float vertices[] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			0.0f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};
		m_VertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
		m_VertexBuffer->Bind();
		{
			BufferLayout layout = {
				{ShaderDataType::Float3, "aPos"},
				{ShaderDataType::Float4, "aColor"}
			};
			m_VertexBuffer->SetLayout(layout);
		}
		uint32_t index = 0;
		const auto& layout = m_VertexBuffer->GetLayout();
		for (auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(
				index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				reinterpret_cast<const void*> (element.Offset)
			);
			index++;
		}

		unsigned int indeces[] = { 0, 1, 2 };
		m_IndexBuffer.reset(IndexBuffer::Create(indeces, 3));

		// Shader
		std::string vertexSrc = R"(
			#version 330 core
			layout (location = 0) in vec3 aPos;
			layout (location = 1) in vec4 aColor;
			out vec4 vColor;
			out vec3 vPos;
			void main()
			{
				vPos = aPos;
				vColor = aColor;
				gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
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
		m_Shader.reset(new Shader(vertexSrc, fragmentSrc));
	}

	Application::~Application()
	{
	}
	

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
		//PR_CORE_TRACE("{0}", e);
	}

	void Application::Run()
	{
		while (m_Running)
		{
			glClearColor(PR_ERROR_COLOR);
			glClear(GL_COLOR_BUFFER_BIT);

			m_Shader->Bind();
			glBindVertexArray(m_VertexArray);
			m_IndexBuffer->Bind();
			glDrawElements(GL_TRIANGLES, m_IndexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);

			OnUpdate();
		}
	}

	void Application::OnUpdate()
	{
		for (Layer* layer : m_LayerStack)
			layer->OnUpdate();

		m_ImGuiLayer->Begin();
		for (Layer* layer : m_LayerStack)
			layer->OnImGuiRender();
		m_ImGuiLayer->End();

		m_Window->OnUpdate();
	}
	
#pragma region Event Handling 事件处理
	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}
#pragma endregion

#pragma region LayerStack 层栈
	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		//layer->OnAttach(); 
		// 这里根据我的想法是有区别的 OnAttach() 应该在LayerStack 中被调用
	}
	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		//layer->OnAttach();
		// 这里根据我的想法是有区别的 OnAttach() 应该在LayerStack 中被调用
	}
#pragma endregion
}

