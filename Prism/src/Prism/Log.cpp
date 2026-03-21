#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#pragma region 为支持中文额外设置

#ifdef PR_PLATFORM_WINDOWS
	#include <windows.h>
#endif

void Prism::InitConsoleUTF8()
{
#ifdef PR_PLATFORM_WINDOWS
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif
}

#pragma endregion

namespace Prism
{

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init()
	{
		InitConsoleUTF8(); // 为支持中文额外设置

		// 输出示例 [2021-08-15 15:30:45.123] Prism: This is a log message
		spdlog::set_pattern("%^[%T] %n: %v%$"); 
		s_CoreLogger = spdlog::stdout_color_mt("PRISM");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);
	}
}