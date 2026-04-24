using Prism.ScriptAttribute;
using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Runtime.Loader;

namespace Prism
{
    public static class ScriptEngine
    {
        private static Dictionary<string, ScriptClass> _classes = new();
        private static Dictionary<uint, ScriptInstance> _instances = new();
        private static MethodInfo _enterPoint = null;

        private static Assembly _assembly;


        private static void LoadClass(Type type)
        {
            if (type.IsSubclassOf(typeof(Entity)))
            {
                _classes[type.FullName] = new ScriptClass(type);
                Log.Trace($"加载脚本类 {type.FullName}");
            }
        }
        private static void LoadMethod(Type type)
        {
            foreach (MethodInfo method in type.GetMethods())
            {
                if (method.GetCustomAttribute<ScriptEnterPoint>() != null)
                    _enterPoint = method;
            }
        }
        private static void LoadTypes()
        {
            foreach (var type in _assembly.GetTypes())
            {
                LoadClass(type);
                LoadMethod(type);
            }
        }

        private static void InitAssembly()
        {
            _enterPoint?.Invoke(null, null);
        }
        public static void Init(string assemblyPath)
        {
            var myContext = AssemblyLoadContext.GetLoadContext(typeof(ScriptEngine).Assembly);
            Log.Trace($"当前加载上下文 {myContext.Name}");
            _assembly = myContext.LoadFromAssemblyPath(Path.GetFullPath(assemblyPath));
            Log.Trace($"加载脚本程序集 {_assembly.FullName}");
            LoadTypes();
            InitAssembly();
        }
        public static void OnCreateEntity(string className, uint entityID, uint sceneID)
        {
            if (!_classes.TryGetValue(className, out var scriptClass))
            {
                Log.Error($"不能找到: {className}");
                return;
            }
            var instance = new ScriptInstance(scriptClass, entityID, sceneID);
            _instances[entityID] = instance;
            instance.InvokeOnCreate();
        }
        public static void OnUpdateEntity(uint entityID)
        {
            if (_instances.TryGetValue(entityID, out var instance))
            {
                instance.InvokeOnUpdate();
            }
        }
        public static void OnDestroyEntity(uint entityID)
        {
            _instances.Remove(entityID);
        }
    }
}