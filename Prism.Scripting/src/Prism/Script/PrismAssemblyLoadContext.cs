
using System.Reflection;
using System.Runtime.Loader;

namespace Prism
{
    public class PrismAssemblyLoadContext : AssemblyLoadContext
    {
        protected override Assembly Load(AssemblyName assemblyName)
        {
            if (assemblyName.Name == Assembly.GetExecutingAssembly().GetName().Name)
            {
                return null;
            }
            return null;
        }
    }
}
