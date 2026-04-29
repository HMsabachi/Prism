
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Prism
{
    public class Material
    {
        public void Set(string uniform, float value)
        {
            unsafe { InternalCalls.Prism_Material_SetFloat(m_UnmanagedInstance, uniform, value);}
        }

        public void Set(string uniform, Texture2D texture)
        {
            unsafe { InternalCalls.Prism_Material_SetTexture(m_UnmanagedInstance, uniform, texture.m_UnmanagedInstance);}
        }

        public void SetTexture(string uniform, Texture2D texture)
        {
            unsafe { InternalCalls.Prism_Material_SetTexture(m_UnmanagedInstance, uniform, texture.m_UnmanagedInstance); }
        }

        internal Material(IntPtr unmanagedInstance)
        {
            m_UnmanagedInstance = unmanagedInstance;
        }

        ~Material()
        {
            unsafe { InternalCalls.Prism_Material_Destructor(m_UnmanagedInstance);}
        }

        internal IntPtr m_UnmanagedInstance;

        
    }

    public class MaterialInstance
    {
        public void Set(string uniform, float value)
        {
            unsafe {  InternalCalls.Prism_MaterialInstance_SetFloat(m_UnmanagedInstance, uniform, value);}
        }

        public void Set(string uniform, Texture2D texture)
        {
            unsafe { InternalCalls.Prism_MaterialInstance_SetTexture(m_UnmanagedInstance, uniform, texture.m_UnmanagedInstance);}
        }

        public void Set(string uniform, Vector3 value)
        {
            unsafe
            {
                IntPtr valuePtr = Marshal.AllocHGlobal(Marshal.SizeOf<Vector3>());
                try
                {
                    Marshal.StructureToPtr(value, valuePtr, false);
                    InternalCalls.Prism_MaterialInstance_SetVector3(m_UnmanagedInstance, uniform, valuePtr);
                }
                finally
                {
                   Marshal.FreeHGlobal(valuePtr);
                }
            }
        }


        public void SetTexture(string uniform, Texture2D texture)
        {
            unsafe { InternalCalls.Prism_MaterialInstance_SetTexture(m_UnmanagedInstance, uniform, texture.m_UnmanagedInstance);}
        }

        internal MaterialInstance(IntPtr unmanagedInstance)
        {
            m_UnmanagedInstance = unmanagedInstance;
        }

        ~MaterialInstance()
        {
            unsafe { InternalCalls.Prism_MaterialInstance_Destructor(m_UnmanagedInstance);}
        }

        internal IntPtr m_UnmanagedInstance;

       
    }
}
