using System;
using System.Runtime.InteropServices;

namespace Prism
{
    [StructLayout(LayoutKind.Sequential)]
    internal unsafe struct FunctionTable
    {
        // Log
        internal delegate* unmanaged[Cdecl]<byte*, void> CoreTrace_Native;
        internal delegate* unmanaged[Cdecl]<byte*, void> CoreInfo_Native;
        internal delegate* unmanaged[Cdecl]<byte*, void> CoreWarn_Native;
        internal delegate* unmanaged[Cdecl]<byte*, void> CoreError_Native;
        internal delegate* unmanaged[Cdecl]<byte*, void> CoreFatal_Native;

        // NativeString
        internal delegate* unmanaged[Cdecl]<byte*, NativeString> String_CreateNativeString_Native;
        internal delegate* unmanaged[Cdecl]<NativeString*, byte*> String_NativeStringToCString_Native;
        internal delegate* unmanaged[Cdecl]<NativeString*, void> String_FreeNativeString_Native;
        internal delegate* unmanaged[Cdecl]<NativeString, NativeString> String_CopyNativeString_Native;

        // Log new
        internal delegate* unmanaged[Cdecl]<Log.LogLevel, NativeString, void> Log_LogMessage_Native;

        // Entity
        internal delegate* unmanaged[Cdecl]<UInt32, UInt32, Matrix4*, void> Entity_GetTransform_Native;
        internal delegate* unmanaged[Cdecl]<UInt32, UInt32, Matrix4*, void> Entity_SetTransform_Native;
    };


    internal static unsafe class InternalCalls
    {
        internal static unsafe FunctionTable Funcs;
    }
}
