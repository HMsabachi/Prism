#pragma once

#include "Core.h" 
#include "Events/Event.h"
#include "Prism/Events/ApplicationEvent.h"
#include "Window.h"


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

	private:
		bool OnWindowClose(WindowCloseEvent& e);

	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};

	// To be defined in CLIENT 需要在客户端定义
	Application* CreateApplication();
}


