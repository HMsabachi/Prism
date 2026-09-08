
using System;
using System.Runtime.CompilerServices;

namespace Prism
{
    [EditorAssignable]
    public class Mesh : Asset
    {
        public Mesh(string filepath) : base(IntPtr.Zero)
        {
            unsafe { m_NativePtr = InternalCalls.Prism_Mesh_Constructor(filepath); }
        }
        internal Mesh(IntPtr nativePtr) : base(nativePtr) { }

        public override string ToString() => $"Mesh({m_NativePtr})";
    }
}
