#include "prpch.h"
#include "ImGuiLayer.h"
#include "Prism/Core/Application.h"

#include "Prism/Core/Log.h"

#include "imgui.h"
#include "ImGuizmo.h"

#define IMGL3W_IMPL
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <filesystem>
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
		SetDarkThemeColors();
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
		ImGuizmo::BeginFrame();
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

		// 构建多语言字形范围
		static const ImWchar jpGlyphRanges[] =
		{
			0x0020, 0x00FF, // ASCII + Latin-1
			0x3000, 0x30FF, // CJK 符号和标点、平假名、片假名
			0x31F0, 0x31FF, // 片假名语音扩展
			0xFF00, 0xFFEF, // 全角 ASCII、半角片假名
			0x4E00, 0x9FFF, // CJK 统一表意文字
			0
		};

		ImFont* pFont = io.Fonts->AddFontFromFileTTF("Assets\\Fonts\\TTF\\LXGWWenKai-Medium.ttf", 25.0f, nullptr, jpGlyphRanges);
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

	void ImGuiLayer::SetDarkThemeColors()
	{
        ImGuiStyle& Style = ImGui::GetStyle();

        const float Hue = 0.0f; // [0,1] range.
        const float Saturation = 1.0f; // [0,6] range.
        const float SaturationAccent = 1.0f; // [0,6] range.
        const float Transparency = 0.95f;
        const float BorderSize = 0.0f;

        Style.FrameBorderSize = BorderSize;

        Style.TabBorderSize = BorderSize;

        Style.WindowRounding = 4.0f;
        Style.ChildRounding = 4.0f;
        Style.FrameRounding = 4.0f;
        Style.GrabRounding = 4.0f;
        Style.TabRounding = 4.0f;

        auto& colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.11f, 0.14f, 0.94f);
        colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.30f, 0.50f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.58f, 0.58f, 0.58f, 0.54f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.96f, 0.00f, 0.00f, 0.40f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(1.00f, 0.00f, 0.00f, 0.67f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.64f, 0.06f, 0.00f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.12f, 1.00f, 0.00f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 1.00f, 0.12f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(1.00f, 0.00f, 0.00f, 0.69f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.98f, 0.26f, 0.26f, 0.80f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.98f, 0.06f, 0.06f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.85f, 0.85f, 0.85f, 0.31f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(1.00f, 0.00f, 0.00f, 0.62f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
        colors[ImGuiCol_Separator] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.38f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.98f, 0.26f, 0.26f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.97f, 0.00f, 0.00f, 0.95f);
        colors[ImGuiCol_Tab] = ImVec4(0.80f, 0.00f, 0.00f, 0.86f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.98f, 0.26f, 0.26f, 0.80f);
        colors[ImGuiCol_TabActive] = ImVec4(1.00f, 0.07f, 0.07f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_DockingPreview] = ImVec4(1.00f, 0.36f, 0.36f, 0.70f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
        colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.10f, 0.15f, 0.60f);
	}
}
