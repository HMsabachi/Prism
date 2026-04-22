#include "prpch.h"
#include "ScriptWrappers.h"

namespace Prism::Script
{

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

}