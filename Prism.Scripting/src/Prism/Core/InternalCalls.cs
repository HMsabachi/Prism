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
        internal delegate* unmanaged[Cdecl]<byte*, NativeString> CreateNativeString_Native;
        internal delegate* unmanaged[Cdecl]<NativeString*, byte*> NativeStringToCString_Native;
        internal delegate* unmanaged[Cdecl]<NativeString*, void> FreeNativeString_Native;
        internal delegate* unmanaged[Cdecl]<NativeString, NativeString> CopyNativeString_Native;

        // Log new
        internal delegate* unmanaged<Log.LogLevel, NativeString, void> Log_LogMessage;
    };


    internal static unsafe class InternalCall
    {
        internal static unsafe FunctionTable Funcs;
    }
}
