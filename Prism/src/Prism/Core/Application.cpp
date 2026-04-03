#include "prpch.h"
#include "Application.h"

#include "Log.h"
#include "Input.h"
#include "Time.h"
#include "Prism/Renderer/Renderer.h"

#include "Prism/Renderer/Legacy/Renderer_Legacy.h"

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

		m_ImGuiLayer = new ImGuiLayer("ImGui");
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

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			(*it)->OnEvent(e);
			if (e.Handled)
				break;
		}
		//PR_CORE_TRACE("{0}", e);
	}

	void Application::Run()
	{
		PR_PROFILE_FUNCTION();
		OnInit();
		while (m_Running)
		{
			OnUpdate();
		}
		OnShutdown();
	}

	void Application::OnUpdate()
	{
		PR_PROFILE_FUNCTION();
		Time::Update();
		//Renderer::SetClearColor( 0.1f, 0.1f, 0.1f, 0.1f );
		//Renderer::Clear(0.1f, 0.1f, 0.1f, 0.1f);
		
		if (!m_Minimized)
		{
			PR_PROFILE_SCOPE("Update LayerStack")
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();
		}
		Application* app = this;
		PR_RENDER_1(app, { app->RenderImGui(); });

		Renderer::WaitAndRender();

		m_Window->OnUpdate();
	}
#pragma region Private Methods 私有方法

	void Application::RenderImGui()
	{
		PR_PROFILE_FUNCTION();
		m_ImGuiLayer->Begin();
		for (Layer* layer : m_LayerStack)
			layer->OnImGuiRender();
		m_ImGuiLayer->End();
	}

	void Application::Initialize()
	{
		// 初始化窗口 Initialize Window
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		// 初始化渲染器 Initialize Renderer
		Renderer_Legacy::Init();
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
		Renderer_Legacy::OnWindowResize(e.GetWidth(), e.GetHeight());
		return false;
	}
#pragma endregion

#pragma region LayerStack 层栈
	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}
	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
	}
#pragma endregion

#pragma endregion
}

