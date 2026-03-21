#pragma once

#ifdef PR_PLATFORM_WINDOWS

extern Prism::Application* Prism::CreateApplication();

int main(int argc, char** argv)
{
	/*printf("Prism Engine is by HMsabachi\n");
	printf("Engine is starting...\n");*/
	auto app = Prism::CreateApplication();
	app->Run();
	delete app;
	return 0;
}

#endif