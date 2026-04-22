using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace ExampleApp
{
    public class Test
    {
        [UnmanagedCallersOnly]
        public static void Init()
        {
            Prism.Core.Init();
            Prism.Core.Log("Hello World!");
        }
    }
}
