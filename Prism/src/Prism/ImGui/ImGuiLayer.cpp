#include "prpch.h"
#include "ImGuiLayer.h"
#include "Prism/Core/Application.h"

#include "Prism/Core/Log.h"

#include "imgui.h"

#define IMGL3W_IMPL
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

namespace Prism
{
	ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer")
	{
	}

	ImGuiLayer::ImGuiLayer(const std::string& name) : Layer(name)
	{

	}

	ImGuiLayer::~ImGuiLayer()
	{
	}
#pragma region Layer Interface  Layer 接口
	void ImGuiLayer::OnAttach()
	{
		PR_CORE_INFO("ImGuiLayer::OnAttach()");
		InitializeImGui();
	}
	void ImGuiLayer::OnDetach()
	{
		DestroyImGui();
	}
	void ImGuiLayer::OnUpdate()
	{
	}
	void ImGuiLayer::OnEvent(Event& event)
	{
		
	}
	void ImGuiLayer::Begin()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}
	void ImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2(static_cast<float>(app.GetWindow().GetWidth()), static_cast<float> (app.GetWindow().GetHeight()));

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}
	void ImGuiLayer::OnImGuiRender()
	{
		
	}
#pragma endregion

	
#pragma region Private Methods 私有方法
	void ImGuiLayer::InitializeImGui()
	{
		ImGui::CreateContext(); // Create the ImGui context 创建ImGui上下文
		ImGui::StyleColorsDark(); // Set the color scheme to dark mode  设置颜色模式为暗黑模式

		ImGuiIO& io = ImGui::GetIO(); // Get the IO structure 获取IO结构体
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // Enable mouse cursor  启用鼠标光标
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos; // Enable setting mouse position  启用设置鼠标位置

		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking  启用停靠
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable viewports  启用viewports
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard navigation  启用键盘导航

		// TODO: 调整字体
		ImFont* pFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\ARLRDBD.TTF", 23.0f);
		io.FontDefault = io.Fonts->Fonts.back();
		
		//SetKeyMap(io); // Set disable 暂时禁用
		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		// 当 viewports 启用时，我们调整 WindowRounding/WindowBg 以使平台窗口看起来与常规窗口相同。
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, style.Colors[ImGuiCol_WindowBg].w);

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		// Setup Platform/Renderer bindings Setup平台/渲染器绑定
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410"); // Initialize the OpenGL renderer  初始化OpenGL渲染器
	}
	void ImGuiLayer::DestroyImGui()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
#pragma endregion

	
}


