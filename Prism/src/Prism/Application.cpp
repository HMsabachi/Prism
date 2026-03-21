#include "Application.h"

#include "Prism/Events/ApplicationEvent.h"
#include "Prism/Log.h"

namespace Prism
{
	Application::Application()
	{
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		WindowResizeEvent e(1280, 720);
		PR_TRACE(e);
		while (true)
		{
		}
	}
}

