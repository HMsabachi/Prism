#include "prpch.h"
#include "Application.h"

#include "Log.h"
#include "Input.h"
#include "Time.h"

#include "Prism/Renderer/Renderer.h"
#include "Prism/Renderer/Framebuffer.h"
#include "PrismShaderParser.h"


#include <imgui.h>
#include <GLFW/glfw3.h>
#include "glad/glad.h"
#include <commdlg.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Windows.h>



namespace Prism
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)
#define PR_ERROR_COLOR 1, 0, 1, 1

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationProps& props)
		: m_Props(props)
	{
		PR_PROFILE_FUNCTION();
		PR_CORE_ASSERT(!s_Instance, "Application already exists! 应用已经存在");
		s_Instance = this;

		Initialize();

		
	}
	void Application::Initialize()
	{
		// 初始化窗口 Initialize Window
		m_Window = std::unique_ptr<Window>(Window::Create(WindowProps(m_Props.Name, m_Props.WindowWidth, m_Props.WindowHeight)));
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		m_Window->SetVSync(m_Props.VSync);
		// 初始化渲染器 Initialize Renderer

		// 初始化时间管理器 Initialize Time Manager
		Time::Init();
		// 初始化ImGui层 Initialize ImGui Layer
		m_ImGuiLayer = new ImGuiLayer("ImGui");
		PushOverlay(m_ImGuiLayer);
		// 初始化PrismShader解释器
		PrismShaderParser::Init();
		// 初始化渲染器 Initialize Renderer
		Renderer::Init();
		Renderer::WaitAndRender();
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
			OnUpdate();
		OnShutdown();
	}

	void Application::OnUpdate()
	{
		PR_PROFILE_FUNCTION();
		Time::Update();
		
		if (!m_Minimized)
		{
			PR_PROFILE_SCOPE("Update LayerStack")
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();

			Application* app = this;
			PR_RENDER_1(app, { app->RenderImGui(); });

			Renderer::WaitAndRender();
		}
		m_Window->OnUpdate();
	}
#pragma region Private Methods 私有方法

	void Application::RenderImGui()
	{
		PR_PROFILE_FUNCTION();
		ImGuiRenderer();

		for (Layer* layer : m_LayerStack)
			layer->OnImGuiRender();
		m_ImGuiLayer->End();
	}

	void Application::ImGuiRenderer()
	{
		m_ImGuiLayer->Begin();
		ImGui::Begin("Renderer");
		auto& caps = RendererAPI::GetCapabilities();
		ImGui::Text("Vendor: %s", caps.Vendor.c_str());
		ImGui::Text("Renderer: %s", caps.Renderer.c_str());
		ImGui::Text("Version: %s", caps.Version.c_str());
		ImGui::Text("Fps: %d", (int)(1.0f / Time::GetDeltaTime()));
		ImGui::End();
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
		int width = e.GetWidth(), height = e.GetHeight();

		//PR_RENDER_2(width, height, { RendererAPI::SetViewport(0, 0, width, height); });
		PR_RENDER_2(width, height, { glViewport(0, 0, width, height); });

		auto& fbs = FramebufferPool::GetGlobal()->GetAll();
		for (auto& fb : fbs)
			fb->Resize(width, height);

		m_Minimized = false;

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

#pragma region Public Methods 公共方法
	std::string Application::OpenFile(const std::string& filter) const
	{
		OPENFILENAMEA ofn;       // common dialog box structure
		CHAR szFile[260] = { 0 };       // if using TCHAR macros

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)m_Window->GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "All\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
	}

#pragma endregion

}

