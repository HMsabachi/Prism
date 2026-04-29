
using System;
using System.Runtime.CompilerServices;

namespace Prism
{
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

        public Material BaseMaterial
        {
            get
            {
                unsafe { return new Material(InternalCalls.Prism_Mesh_GetMaterial(m_UnmanagedInstance)); }
            }
        }

        public MaterialInstance GetMaterial(int index)
        {
            unsafe 
            { 
                return new MaterialInstance(InternalCalls.Prism_Mesh_GetMaterialByIndex(m_UnmanagedInstance, index));
            }
        }

        public int GetMaterialCount()
        {
            unsafe { return InternalCalls.Prism_Mesh_GetMaterialCount(m_UnmanagedInstance); }
        }

        internal IntPtr m_UnmanagedInstance;

    }
}
