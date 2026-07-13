#include "prpch.h"
#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

#include <filesystem>
#pragma region 为支持中文额外设置

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

		// Create "logs" directory if doesn't exist
		std::string logsDirectory = "logs";
		if (!std::filesystem::exists(logsDirectory))
			std::filesystem::create_directories(logsDirectory);

		// 输出示例 [2021-08-15 15:30:45.123] Prism: This is a log message
		spdlog::set_pattern("%^[%T] %n: %v%$");

		std::vector<spdlog::sink_ptr> coreSinks =
		{
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
			std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/PRISM.log", true)
		};

		std::vector<spdlog::sink_ptr> appSinks =
		{
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
			std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/APP.log", true)
		};

		s_CoreLogger = std::make_shared<spdlog::logger>("PRISM", coreSinks.begin(), coreSinks.end());
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = std::make_shared<spdlog::logger>("APP", appSinks.begin(), appSinks.end());
		s_ClientLogger->set_level(spdlog::level::trace);
	}

	void Log::Shutdown()
	{
		s_ClientLogger.reset();
		s_CoreLogger.reset();
		spdlog::drop_all();
	}
}

