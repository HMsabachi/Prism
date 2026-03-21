#pragma once

#include <memory>
#include "Core.h"
#include "spdlog/spdlog.h"

#ifdef PR_PLATFORM_WINDOWS
	#include <windows.h>
#endif

namespace Prism {
	void InitConsoleUTF8();

	class PRISM_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Core log macros 引擎核心日志
#define PR_CORE_TRACE(...)   ::Prism::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define PR_CORE_INFO(...)    ::Prism::Log::GetCoreLogger()->info(__VA_ARGS__)
#define PR_CORE_WARN(...)    ::Prism::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define PR_CORE_ERROR(...)   ::Prism::Log::GetCoreLogger()->error(__VA_ARGS__)
#define PR_CORE_FATAL(...)   ::Prism::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client log macros 客户端日志
#define PR_TRACE(...)        ::Prism::Log::GetClientLogger()->trace(__VA_ARGS__)
#define PR_INFO(...)         ::Prism::Log::GetClientLogger()->info(__VA_ARGS__)
#define PR_WARN(...)         ::Prism::Log::GetClientLogger()->warn(__VA_ARGS__)
#define PR_ERROR(...)        ::Prism::Log::GetClientLogger()->error(__VA_ARGS__)
#define PR_FATAL(...)        ::Prism::Log::GetClientLogger()->fatal(__VA_ARGS__)

// If dist build 如果是发布版本