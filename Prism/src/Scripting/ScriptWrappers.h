#pragma once

namespace Prism::Native
{
	struct NativeString;
}

namespace Prism::Script
{
	// Log
	void Prism_Log_Core_Trace(const char* mes);
	void Prism_Log_Core_Info(const char* mes);
	void Prism_Log_Core_Warn(const char* mes);
	void Prism_Log_Core_Error(const char* mes);
	void Prism_Log_Core_Fatal(const char* mes);

	enum class LogLevel : int32_t
	{
		Trace = BIT(0),
		Debug = BIT(1),
		Info = BIT(2),
		Warn = BIT(3),
		Error = BIT(4),
		Critical = BIT(5)
	};

	void Log_LogMessage(LogLevel level, Native::NativeString inFormattedMessage);
}