
using System;
using System.Runtime.CompilerServices;

namespace Prism
{
    public static class MeshFactory
    {

        public static Mesh CreatePlane(float width, float height)
        {
            return new Mesh(CreatePlane_Native(width, height));
        }

        public unsafe static IntPtr CreatePlane_Native(float width, float height)
        {
            return InternalCalls.Prism_MeshFactory_CreatePlane(width, height);
        }

    }
}
