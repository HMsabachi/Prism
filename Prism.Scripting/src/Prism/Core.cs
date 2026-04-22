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
            public delegate* unmanaged[Cdecl]<byte*, void> CoreLog;
        }
        private static FunctionTable _functionTable;

        public static void Init()
        {
            Log($"真实类名: {typeof(Prism.Core).FullName}");
            Log($"真实程序集限定名: {typeof(Prism.Core).AssemblyQualifiedName}");
        }

        [UnmanagedCallersOnly]
        public static void PushFunctionTable(FunctionTable* table)
        {
            _functionTable = *table;
        }

        // TODO: 临时
        public static void Log(string message)
        {
            byte[] buffer = Encoding.UTF8.GetBytes(message + '\0');
            fixed (byte* ptr = buffer)
            {
                _functionTable.CoreLog(ptr);
                return;
            }
        }

    }
}
