#pragma once
#include "Prism/Core/Core.h"

#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>

#include <thread>

namespace Prism {
	struct ProfileResult
	{
		std::string Name;
		long long Start, End;
		uint32_t ThreadID;
	};

	struct InstrumentationSession
	{
		std::string Name;
	};

	class PRISM_API Instrumentor
	{
	private:
		InstrumentationSession* m_CurrentSession;
		std::ofstream m_OutputStream;
		int m_ProfileCount;
	public:
		Instrumentor();

		void BeginSession(const std::string& name, const std::string& filepath = "results.json");

		void EndSession();

		void WriteProfile(const ProfileResult& result);

		void WriteHeader();

		void WriteFooter();

		static Instrumentor& Get();
	};

	class PRISM_API InstrumentationTimer
	{
	public:
		InstrumentationTimer(const char* name);

		~InstrumentationTimer();

		void Stop();
	private:
		const char* m_Name;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
		bool m_Stopped;
	};
}

#define PR_PROFILE 0
#if PR_PROFILE
#define PR_PROFILE_BEGIN_SESSION(name, filepath) ::Prism::Instrumentor::Get().BeginSession(name, filepath)
#define PR_PROFILE_END_SESSION() ::Prism::Instrumentor::Get().EndSession()
#define PR_PROFILE_SCOPE(name) ::Prism::InstrumentationTimer timer##__LINE__(name);
#define PR_PROFILE_FUNCTION() PR_PROFILE_SCOPE(__FUNCSIG__)
#else
#define PR_PROFILE_BEGIN_SESSION(name, filepath)
#define PR_PROFILE_END_SESSION()
#define PR_PROFILE_SCOPE(name)
#define PR_PROFILE_FUNCTION()
#endif