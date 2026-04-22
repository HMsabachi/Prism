#pragma once

namespace Prism::Script
{
	// Log
	void Prism_Log_Core_Trace(const char* mes);
	void Prism_Log_Core_Info(const char* mes);
	void Prism_Log_Core_Warn(const char* mes);
	void Prism_Log_Core_Error(const char* mes);
	void Prism_Log_Core_Fatal(const char* mes);
}