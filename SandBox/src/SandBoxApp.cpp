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
		PR_INFO("ExampleLayer::Update");
	}

	void OnEvent(Prism::Event& event) override
	{
		PR_TRACE("{0}", event);
	}

};

class Sandbox : public Prism::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}
	~Sandbox()
	{

	}
};

Prism::Application* Prism::CreateApplication()
{
	return new Sandbox();
}