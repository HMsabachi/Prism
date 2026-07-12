using System;

namespace Prism
{
    public static class Time
    {
        public static float DeltaTime { get { unsafe { return InternalCalls.Prism_Time_GetDeltaTime(); } } }
        public static float UnscaledDeltaTime { get { unsafe { return InternalCalls.Prism_Time_GetUnscaledDeltaTime(); } } }
        public static float GameTime { get { unsafe { return InternalCalls.Prism_Time_GetTime(); } } }
        public static float UnscaledTime { get { unsafe { return InternalCalls.Prism_Time_GetUnscaledTime(); } } }
        public static float FixedDeltaTime { get { unsafe { return InternalCalls.Prism_Time_GetFixedDeltaTime(); } } set { unsafe { InternalCalls.Prism_Time_SetFixedDeltaTime(value); } } }
        public static UInt64 FrameCount { get { unsafe { return InternalCalls.Prism_Time_GetFrameCount(); } } }
        public static float TimeScale { get { unsafe { return InternalCalls.Prism_Time_GetTimeScale(); } } set { unsafe { InternalCalls.Prism_Time_SetTimeScale(value); } } }
    }
}
