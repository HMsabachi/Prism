using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using static Prism.Core;

namespace Prism
{
    public unsafe class Core
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct FunctionTable
        {
            // Log
            public delegate* unmanaged[Cdecl]<byte*, void> CoreTrace_Native;
            public delegate* unmanaged[Cdecl]<byte*, void> CoreInfo_Native;
            public delegate* unmanaged[Cdecl]<byte*, void> CoreWarn_Native;
            public delegate* unmanaged[Cdecl]<byte*, void> CoreError_Native;
            public delegate* unmanaged[Cdecl]<byte*, void> CoreFatal_Native;
        }
        public static FunctionTable EngineFuncs { get; private set; }

        public static void Init()
        {
            Log.PR_CORE_INFO($"类名: {typeof(Prism.Core).FullName}");
            Log.PR_CORE_INFO($"程序集限定名: {typeof(Prism.Core).AssemblyQualifiedName}");
        }

        [UnmanagedCallersOnly]
        public static void PushFunctionTable(FunctionTable* table)
        {
            EngineFuncs = *table;
        }

    }
}
