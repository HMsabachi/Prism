#pragma once
#include <chrono>
#include "Core.h"

namespace Prism
{
	class Application;
}

namespace Prism
{
	class PRISM_API Time
	{
		friend class Application;
	public:
		static float GetDeltaTime() noexcept { return s_DeltaTime; }
		static float GetUnscaledDeltaTime() noexcept { return s_UnscaledDeltaTime; }
		static float GetTime() noexcept { return s_Time; }
		static float GetUnscaledTime() noexcept { return s_UnscaledTime; }
		static float GetFixedDeltaTime() noexcept { return s_FixedDeltaTime; }
		static long long GetFrameCount() noexcept { return s_FrameCount; }
	public:
		static void SetTimeScale(float scale) noexcept;
		static float GetTimeScale() noexcept { return s_TimeScale; }
	private:
		static void Initialize();
		static void Update();
		static bool ShouldFixedUpdate();
		static void Reset();
	private:
		static float s_DeltaTime;
		static float s_UnscaledDeltaTime;
		static float s_Time;
		static float s_UnscaledTime;
		static float s_FixedDeltaTime;
		static float s_TimeScale;
		static long long s_FrameCount;

		static std::chrono::steady_clock::time_point s_StartTime;
		static std::chrono::steady_clock::time_point s_LastFrameTime;
		static std::chrono::steady_clock::time_point s_LastFixedTime;
	};
}