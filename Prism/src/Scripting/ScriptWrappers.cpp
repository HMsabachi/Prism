#include "prpch.h"
#include "ScriptWrappers.h"
#include "Native/NativeString.h"

namespace Prism::Script
{
#pragma region Legacy Functions

	void Prism_Log_Core_Trace(const char* mes)
	{
		PR_CORE_TRACE("[Script]: {0}", mes);
	}
	void Prism_Log_Core_Info(const char* mes)
	{
		PR_CORE_INFO("[Script]: {0}", mes);
	}
	void Prism_Log_Core_Warn(const char* mes)
	{
		PR_CORE_WARN("[Script]: {0}", mes);
	}
	void Prism_Log_Core_Error(const char* mes)
	{
		PR_CORE_ERROR("[Script]: {0}", mes);
	}
	void Prism_Log_Core_Fatal(const char* mes)
	{
		PR_CORE_FATAL("[Script]: {0}", mes);
	}
#pragma endregion

	void Log_LogMessage(LogLevel level, Native::NativeString inFormattedMessage)
	{
		std::string message = NativeStringToCString(&inFormattedMessage);
		message = "[Script]: " + message;
		switch (level)
		{
		case LogLevel::Trace:
			PR_CORE_TRACE(message);
			break;
		case LogLevel::Debug:
			PR_CORE_INFO(message);
			break;
		case LogLevel::Info:
			PR_CORE_INFO(message);
			break;
		case LogLevel::Warn:
			PR_CORE_WARN(message);
			break;
		case LogLevel::Error:
			PR_CORE_ERROR(message);
			break;
		case LogLevel::Critical:
			PR_CORE_FATAL(message);
			break;
		}
		Native::FreeNativeString(&inFormattedMessage);
	}

}