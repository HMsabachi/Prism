#include <Prism.h>

class ExampleLayer : public Prism::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void OnUpdate() override
	{
		//PR_INFO("ExampleLayer::Update");
	}

	void OnEvent(Prism::Event& event) override
	{
		if (event.GetEventType() == Prism::EventType::WindowResize)
			PR_TRACE(event);
	}

};

class Sandbox : public Prism::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
		PushOverlay(new Prism::ImGuiLayer());
	}
	~Sandbox()
	{

	}
};

Prism::Application* Prism::CreateApplication()
{
	return new Sandbox();
}