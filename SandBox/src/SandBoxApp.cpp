#include <Prism.h>
#include "Prism/Core/EntryPoint.h"
#include "../../Prism/src/Platform/OpenGL/OpenGLTexture.h"

#include "Sandbox2D.h"
#include "ExampleLayer.h"



class Sandbox : public Prism::Application
{
public:
	Sandbox()
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
		
	}
	~Sandbox()
	{

	}
};

Prism::Application* Prism::CreateApplication()
{
	return new Sandbox();
}