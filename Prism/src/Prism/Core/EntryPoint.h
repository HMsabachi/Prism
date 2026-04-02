#pragma once

#ifdef PR_PLATFORM_WINDOWS

extern Prism::Application* Prism::CreateApplication();

int main(int argc, char** argv)
{
	PR_PROFILE_BEGIN_SESSION("Startup", "PrismProfile-Startup.json");
	auto app = Prism::CreateApplication();
	PR_PROFILE_END_SESSION();
	PR_PROFILE_BEGIN_SESSION("Runtime", "PrismProfile-Runtime.json");
	app->Run();
	PR_PROFILE_END_SESSION();
	PR_PROFILE_BEGIN_SESSION("Startup", "PrismProfile-Shutdown.json");
	delete app;
	PR_PROFILE_END_SESSION();
	return 0;
}

#endif