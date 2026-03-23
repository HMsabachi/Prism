#pragma once

#ifdef PR_PLATFORM_WINDOWS

extern Prism::Application* Prism::CreateApplication();

int main(int argc, char** argv)
{
	// TODO: Log 初始化暂时写在这里
	Prism::Log::Init();
	PR_CORE_WARN("初始化 Log!");

	auto app = Prism::CreateApplication();
	app->Run();
	delete app;
	return 0;
}

#endif