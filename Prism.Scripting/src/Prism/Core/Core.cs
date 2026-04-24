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
            Log.Trace($"程序集限定名: {typeof(Prism.Core).AssemblyQualifiedName}");
        }

        [UnmanagedCallersOnly]
        internal unsafe static void PushFunctionTable(FunctionTable* table)
        {
            InternalCalls.Funcs = *table;
        }

        #region 脚本引擎调用
        [UnmanagedCallersOnly]
        internal unsafe static void CallScriptEngineInit(NativeString nativeString)
        {
            ScriptEngine.Init(nativeString);
            nativeString.Dispose();
        }
        [UnmanagedCallersOnly]
        internal unsafe static void CallScriptEngineOnCreateEntity(NativeString className, UInt32 entityID, UInt32 sceneID)
        {
            ScriptEngine.OnCreateEntity(className, entityID, sceneID);
            className.Dispose();
        }
        [UnmanagedCallersOnly]
        internal unsafe static void CallScriptEngineOnUpdateEntity(UInt32 entityID)
        {
            ScriptEngine.OnUpdateEntity(entityID);
        }
        [UnmanagedCallersOnly]
        internal unsafe static void CallScriptEngineOnDestroyEntity(UInt32 entityID)
        {
            ScriptEngine.OnDestroyEntity(entityID);
        }
        #endregion


    }
}
