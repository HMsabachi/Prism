#include "prpch.h"
#include "Application.h"

#include "Log.h"
#include "Input.h"
#include "Time.h"

#include "Prism/Renderer/Renderer.h"

namespace Prism
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)
#define PR_ERROR_COLOR 1, 0, 1, 1

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		PR_PROFILE_FUNCTION();
		PR_CORE_ASSERT(!s_Instance, "Application already exists! 应用已经存在");
		s_Instance = this;

		Initialize();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		PR_PROFILE_FUNCTION();
	}
	

	void Application::OnEvent(Event& e)
	{
		PR_PROFILE_FUNCTION();
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

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
		PR_PROFILE_FUNCTION();
		while (m_Running)
		{
			OnUpdate();
		}
	}

	void Application::OnUpdate()
	{
		PR_PROFILE_FUNCTION();
		Time::Update();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 0.1f });
		RenderCommand::Clear();

		if (!m_Minimized)
		{
			PR_PROFILE_SCOPE("Update LayerStack")
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();
		}

		m_ImGuiLayer->Begin();
		for (Layer* layer : m_LayerStack)
			layer->OnImGuiRender();
		m_ImGuiLayer->End();

		m_Window->OnUpdate();
	}
#pragma region Private Methods 私有方法
	void Application::Initialize()
	{
		// 初始化窗口 Initialize Window
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		// 初始化渲染器 Initialize Renderer
		Renderer::Init();
		// 初始化时间管理器 Initialize Time Manager
		Time::Init();
	}


#pragma region Event Handling 事件处理
	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		PR_PROFILE_FUNCTION();
		m_Running = false;
		return true;
	}
	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		PR_PROFILE_FUNCTION();
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			PR_CORE_INFO("已暂停引擎 Engine Paused");
			return false;
		}
		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
		return false;
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

#pragma endregion
}

