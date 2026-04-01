#pragma once

#ifdef PR_PLATFORM_WINDOWS

extern Prism::Application* Prism::CreateApplication();

int main(int argc, char** argv)
{
	// TODO: Log 初始化暂时写在这里
	Prism::Log::Init();
	PR_PROFILE_BEGIN_SESSION("Startup", "HazelProfile-Startup.json");
	auto app = Prism::CreateApplication();
	PR_PROFILE_END_SESSION();
	PR_PROFILE_BEGIN_SESSION("Runtime", "HazelProfile-Runtime.json");
	app->Run();
	PR_PROFILE_END_SESSION();
	PR_PROFILE_BEGIN_SESSION("Startup", "HazelProfile-Shutdown.json");
	delete app;
	PR_PROFILE_END_SESSION();
	return 0;
}

#endif