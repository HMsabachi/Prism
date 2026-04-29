using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using static Prism.Core;

using Rolky.Managed.Interop;

namespace Prism
{
    public class Core
    {
        public static void Init()
        {
            Log.Trace($"类名: {typeof(Prism.Core).FullName}");
            Log.Trace($"程序集限定名: {typeof(Prism.Core).AssemblyQualifiedName}");
        }

        #region 脚本引擎调用
        
        internal static void CallScriptEngineInit(NativeString nativeString)
        {
            ScriptEngine.Init(nativeString);
            nativeString.Dispose();
        }
        
        internal static void CallScriptEngineOnCreateEntity(NativeString className, UInt32 entityID, UInt32 sceneID)
        {
            if (className == null) return;
            //Console.WriteLine($"CallScriptEngineOnCreateEntity: {className} {entityID} {sceneID}");
            ScriptEngine.OnCreateEntity(className, entityID, sceneID);
            className.Dispose();
        }
        
        internal static void CallScriptEngineOnUpdateEntity(UInt32 entityID)
        {
            //Console.WriteLine($"CallScriptEngineOnUpdateEntity: {entityID}");
            ScriptEngine.OnUpdateEntity(entityID);
        }
        
        internal static void CallScriptEngineOnDestroyEntity(UInt32 entityID)
        {
            //Console.WriteLine($"CallScriptEngineOnDestroyEntity: {entityID}");
            ScriptEngine.OnDestroyEntity(entityID);
        }
        #endregion


    }
}
