
using System;
using System.Runtime.CompilerServices;

namespace Prism
{
    public class Mesh : Asset
    {
        public unsafe Mesh(string filepath) : base(InternalCalls.Prism_Mesh_Constructor(filepath))
        {
        }
        internal Mesh(IntPtr nativePtr) : base(nativePtr) { }

        public override string ToString() => $"Mesh({m_NativePtr})";
    }
}
