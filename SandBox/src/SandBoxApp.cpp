#include <Prism.h>

class Sandbox : public Prism::Application
{
public:
	Sandbox()
	{

	}
	~Sandbox()
	{

	}
};

Prism::Application* Prism::CreateApplication()
{
	return new Sandbox();
}