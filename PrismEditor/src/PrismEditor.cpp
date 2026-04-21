#include <Prism.h>
#include "EditorLayer.h"

#include "Prism/Core/EntryPoint.h"

class PrismEditorApplication : public Prism::Application
{
public:
	PrismEditorApplication(const Prism::ApplicationProps& props)
		: Application(props)
	{
	}

	virtual void OnInit() override
	{
		Application::OnInit();
		PushLayer(new Prism::EditorLayer());
	}
};

Prism::Application* Prism::CreateApplication()
{
	return new PrismEditorApplication({ "PrismEditor", 1920, 1080 , true});
}