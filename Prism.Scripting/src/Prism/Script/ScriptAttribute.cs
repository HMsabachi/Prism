using System;

namespace Prism.ScriptAttribute
{
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
    public class ScriptEnterPoint : Attribute {}
}
