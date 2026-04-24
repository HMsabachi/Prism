using System;
using System.Reflection;

namespace Prism
{
    public class ScriptClass
    {
        public Type Type;
        public MethodInfo OnCreate;
        public MethodInfo OnUpdate;

        public ScriptClass(Type type)
        {
            Type = type;

            OnCreate = type.GetMethod("OnCreate", BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
            OnUpdate = type.GetMethod("OnUpdate", BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
        }

        public object Instantiate()
        {
            return Activator.CreateInstance(Type);
        }
    }
}