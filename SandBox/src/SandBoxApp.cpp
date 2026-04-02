#include <Prism.h>
#include "Prism/Core/EntryPoint.h"
#include "../../Prism/src/Platform/OpenGL/OpenGLTexture.h"

#include "Sandbox2D.h"
#include "ExampleLayer.h"

class GameLayer : public Prism::Layer
{
public:
	GameLayer()
	{
	}

	virtual ~GameLayer()
	{
	}

	virtual void OnAttach() override
	{
	}

	virtual void OnDetach() override
	{
	}

	virtual void OnUpdate() override
	{
	}

	virtual void OnImGuiRender() override
	{
		//ImGui::Begin("Example Window");
		//ImGui::Text("Hello World!");
		//ImGui::End();
	}

	virtual void OnEvent(Prism::Event& event) override
	{
	}
};

class Sandbox : public Prism::Application
{
public:
	Sandbox()
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
		PushLayer(new GameLayer());
		
	}
	~Sandbox()
	{

	}
};

Prism::Application* Prism::CreateApplication()
{
	return new Sandbox();
}