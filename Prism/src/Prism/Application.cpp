#include "Application.h"

#include "Prism/Log.h"
#include "Prism/Events/ApplicationEvent.h"

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
		if (e.IsInCategory(EventCategoryApplication))
		{
			PR_TRACE(e);
		}
		if (e.IsInCategory(EventCategoryInput))
		{
			PR_TRACE(e);
		}
		while (true)
		{
		}
	}
}

