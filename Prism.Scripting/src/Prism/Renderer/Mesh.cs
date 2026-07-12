
using System;
using System.Runtime.CompilerServices;

namespace Prism
{
    [EditorAssignable]
    public class Mesh
    {
        public Mesh(string filepath)
        {
            unsafe { m_UnmanagedInstance = InternalCalls.Prism_Mesh_Constructor(filepath); }
        }
        internal Mesh(IntPtr unmanagedInstance)
        {
            m_UnmanagedInstance = unmanagedInstance;
        }

        ~Mesh()
        {
            unsafe { InternalCalls.Prism_Mesh_Destructor(m_UnmanagedInstance); }
        }

        public override string ToString() => $"Mesh({m_UnmanagedInstance})";

        internal IntPtr m_UnmanagedInstance;
    }
}
