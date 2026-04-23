using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using static Prism.Core;

namespace Prism
{
    public class Core
    {
        public static void Init()
        {
            Log.Trace($"类名: {typeof(Prism.Core).FullName}");
            Log.Info($"程序集限定名: {typeof(Prism.Core).AssemblyQualifiedName}");
        }

        [UnmanagedCallersOnly]
        internal unsafe static void PushFunctionTable(FunctionTable* table)
        {
            InternalCall.Funcs = *table;
        }

    }
}
