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

	Application::Application()
	{
		PR_CORE_ASSERT(!s_Instance, "Application already exists! 应用已经存在");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
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

