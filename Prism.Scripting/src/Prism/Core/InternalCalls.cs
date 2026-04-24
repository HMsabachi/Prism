using System;
using System.Runtime.InteropServices;

namespace Prism
{
    [StructLayout(LayoutKind.Sequential)]
    internal unsafe struct FunctionTable
    {
        // NativeString
        internal delegate* unmanaged[Cdecl]<byte*, NativeString> String_CreateNativeString_Native;
        internal delegate* unmanaged[Cdecl]<NativeString*, byte*> String_NativeStringToCString_Native;
        internal delegate* unmanaged[Cdecl]<NativeString*, void> String_FreeNativeString_Native;
        internal delegate* unmanaged[Cdecl]<NativeString, NativeString> String_CopyNativeString_Native;        
    };


    internal static unsafe class InternalCalls
    {
        internal static unsafe FunctionTable Funcs;
        internal static delegate* unmanaged[Cdecl]<Log.LogLevel, NativeString, void> Prism_Log_LogMessage;
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
    }
}
