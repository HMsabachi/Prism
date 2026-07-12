#include "prpch.h"
#include "Time.h"

namespace Prism
{
    // Static variables initialize 静态成员初始化
    float Time::s_DeltaTime = 0.0f;
    float Time::s_UnscaledDeltaTime = 0.0f;
    float Time::s_Time = 0.0f;
    float Time::s_FixedDeltaTime = 1.0f / 60.0f;
    float Time::s_ActualFixedDeltaTime = 0.0f;
    float Time::s_FixedAccumulator = 0.0f;
    float Time::s_InterpolationAlpha = 0.0f;
    float Time::s_UnscaledTime = 0.0f;
    float Time::s_TimeScale = 1.0f;
    long long Time::s_FrameCount = 0;

    std::chrono::steady_clock::time_point Time::s_StartTime;
    std::chrono::steady_clock::time_point Time::s_LastFrameTime;

    void Time::Init()
    {
        auto now = std::chrono::steady_clock::now();
        s_StartTime = now;
        s_LastFrameTime = now;

        s_DeltaTime = 0.0f;
        s_UnscaledDeltaTime = 0.0f;
        s_Time = 0.0f;
        s_UnscaledTime = 0.0f;
        s_FrameCount = 0;
        s_FixedAccumulator = 0.0f;
        s_InterpolationAlpha = 0.0f;
    }

    void Time::Update()
    {
        auto now = std::chrono::steady_clock::now();

        s_UnscaledDeltaTime = std::chrono::duration<float>(now - s_LastFrameTime).count();
        s_LastFrameTime = now;

        s_DeltaTime = s_UnscaledDeltaTime * s_TimeScale;

        s_UnscaledTime = std::chrono::duration<float>(now - s_StartTime).count();
        s_Time = s_UnscaledTime * s_TimeScale;

        s_FrameCount++;

        s_FixedAccumulator += s_UnscaledDeltaTime;
        s_InterpolationAlpha = s_FixedAccumulator / s_FixedDeltaTime;
    }

    int Time::ConsumeFixedUpdate()
    {
        int count = 0;
        while (s_FixedAccumulator >= s_FixedDeltaTime)
        {
            s_FixedAccumulator -= s_FixedDeltaTime;
            s_ActualFixedDeltaTime = s_FixedDeltaTime;
            count++;
        }
        return count;
    }

    void Time::Reset()
    {
        Init();
    }

    void Time::SetFixedDeltaTime(float fixedDeltaTime) noexcept
    {
        PR_CORE_ASSERT(fixedDeltaTime > 0.0f, "FixedDeltaTime must be positive!");
        s_FixedDeltaTime = fixedDeltaTime;
    }

    void Time::SetTimeScale(float scale) noexcept
    {
        if (scale < 0.0f)
        {
            PR_CORE_WARN("Time scale cannot be negative. Setting it to 0.0f.");
            s_TimeScale = 0.0f;
            return;
        }
        s_TimeScale = scale;
    }
}