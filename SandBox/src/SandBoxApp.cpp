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
		if (Prism::Input::IsKeyPressed(PR_KEY_TAB))
			PR_TRACE("Tab key is pressed (poll)!");
	}

	void OnEvent(Prism::Event& event) override
	{
		if (event.GetEventType() == Prism::EventType::KeyPressed)
		{
			Prism::KeyPressedEvent& e = (Prism::KeyPressedEvent&)event;
			if (e.GetKeyCode() == PR_KEY_TAB)
				PR_TRACE("Tab key is pressed (event)!");
			PR_TRACE("{0}", (char)e.GetKeyCode());
		}
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