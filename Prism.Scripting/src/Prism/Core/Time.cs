using System;

namespace Prism
{
    public static class Time
    {
        public static float DeltaTime { get { return GetDeltaTime_Native(); } }
        public static float UnscaledDeltaTime { get { return GetUnscaledDeltaTime_Native(); } }
        public static float GameTime { get { return GetTime_Native(); } }
        public static float UnscaledTime { get { return GetUnscaledTime_Native(); } }
        public static float FixedDeltaTime { get { return GetFixedDeltaTime_Native(); } }
        public static UInt64 FrameCount { get { return GetFrameCount_Native(); } }
        public static float TimeScale { get { return GetTimeScale_Native(); }  set { Time_SetTimeScale_Native(value); } }

        private unsafe static float GetDeltaTime_Native()
        {
            return InternalCalls.Prism_Time_GetDeltaTime();
        }
        private unsafe static float GetUnscaledDeltaTime_Native()
        {
            return InternalCalls.Prism_Time_GetUnscaledDeltaTime();
        }
        private unsafe static float GetTime_Native()
        {
            return InternalCalls.Prism_Time_GetTime();
        }
        private unsafe static float GetUnscaledTime_Native()
        {
            return InternalCalls.Prism_Time_GetUnscaledTime();
        }
        private unsafe static float GetFixedDeltaTime_Native()
        {
            return InternalCalls.Prism_Time_GetFixedDeltaTime();
        }
        private unsafe static UInt64 GetFrameCount_Native()
        {
            return InternalCalls.Prism_Time_GetFrameCount();
        }
        private unsafe static void Time_SetTimeScale_Native(float scale)
        {
            InternalCalls.Prism_Time_SetTimeScale(scale);
        }
        private unsafe static float GetTimeScale_Native()
        {
            return InternalCalls.Prism_Time_GetTimeScale();
        }
    }
}
