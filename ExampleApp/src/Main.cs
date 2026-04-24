using Prism;
using Prism.ScriptAttribute;
namespace Example
{
    public class Test
    {
        [ScriptEnterPoint]
        public static void Init()
        {
            Prism.Core.Init();
            Prism.Log.Trace("Hello World!");
        }
    }
}
