#pragma once

#include "Core.h"

#include "LayerStack.h"
#include "Prism/Events/ApplicationEvent.h"
#include "Prism/Events/Event.h"
#include "Window.h"

#include "Prism/ImGui/ImGuiLayer.h"



namespace Prism
{
	class PRISM_API Application
	{
	public:
		Application();
		virtual ~Application();

		// The main loop of the application 启动应用主循环
		void Run();

		// Event handling function 事件处理函数
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		std::string OpenFile(const std::string& filter) const;

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() const { return *m_Window; }
	private:
		void OnInit() {}
		void OnShutdown() {}
		void RenderImGui();


		void ImGuiRenderer();

	private:
		void Initialize();
		// Update function for the application(frame update) 应用更新函数(帧更新)
		void OnUpdate(); 
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		bool m_Minimized = false;
		ImGuiLayer* m_ImGuiLayer;
		LayerStack m_LayerStack;
	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT 需要在客户端定义
	Application* CreateApplication();
}


