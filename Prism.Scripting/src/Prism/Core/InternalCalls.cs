using System;
using System.Runtime.InteropServices;

using Rolky.Managed.Interop;

namespace Prism
{


    internal static unsafe class InternalCalls
    {
        internal static delegate* unmanaged[Cdecl]<Log.LogLevel, Rolky.Managed.Interop.NativeString, void> Prism_Log_LogMessage;
        // Time
        internal static delegate* unmanaged[Cdecl]<float> Prism_Time_GetDeltaTime;
        internal static delegate* unmanaged[Cdecl]<float> Prism_Time_GetUnscaledDeltaTime;
        internal static delegate* unmanaged[Cdecl]<float> Prism_Time_GetTime;
        internal static delegate* unmanaged[Cdecl]<float> Prism_Time_GetUnscaledTime;
        internal static delegate* unmanaged[Cdecl]<float> Prism_Time_GetFixedDeltaTime;
        internal static delegate* unmanaged[Cdecl]<UInt64> Prism_Time_GetFrameCount;
        internal static delegate* unmanaged[Cdecl]<float, void> Prism_Time_SetTimeScale;
        internal static delegate* unmanaged[Cdecl]<float> Prism_Time_GetTimeScale;
        // Math
        internal static delegate* unmanaged[Cdecl]<float, float, float> Prism_Noise_PerlinNoise;
        // Input
        internal static delegate* unmanaged[Cdecl]<KeyCode, bool> Prism_Input_IsKeyPressed;
        // Entity
        internal static delegate* unmanaged[Cdecl]<UInt32, UInt32, Matrix4*, void> Prism_Entity_GetTransform;
        internal static delegate* unmanaged[Cdecl]<UInt32, UInt32, Matrix4*, void> Prism_Entity_SetTransform;
        internal static delegate* unmanaged[Cdecl]<UInt32, UInt32, ReflectionType, void> Prism_Entity_CreateComponent;
        internal static delegate* unmanaged[Cdecl]<UInt32, UInt32, ReflectionType, bool> Prism_Entity_HasComponent;
    }
}
