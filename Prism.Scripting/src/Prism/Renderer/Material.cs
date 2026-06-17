using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Prism
{
    public class Material
    {
        public void Set(string uniform, float value)
        {
            unsafe { InternalCalls.Prism_Material_SetFloat(m_UnmanagedInstance, uniform, value); }
        }

        public void Set(string uniform, Vector3 value)
        {
            unsafe
            {
                IntPtr valuePtr = Marshal.AllocHGlobal(Marshal.SizeOf<Vector3>());
                try
                {
                    Marshal.StructureToPtr(value, valuePtr, false);
                    InternalCalls.Prism_Material_SetVector3(m_UnmanagedInstance, uniform, valuePtr);
                }
                finally
                {
                    Marshal.FreeHGlobal(valuePtr);
                }
            }
        }

        public void Set(string uniform, Vector4 value)
        {
            unsafe
            {
                IntPtr valuePtr = Marshal.AllocHGlobal(Marshal.SizeOf<Vector4>());
                try
                {
                    Marshal.StructureToPtr(value, valuePtr, false);
                    InternalCalls.Prism_Material_SetVector4(m_UnmanagedInstance, uniform, valuePtr);
                }
                finally
                {
                    Marshal.FreeHGlobal(valuePtr);
                }
            }
        }

        public void Set(string uniform, Texture2D texture)
        {
            unsafe { InternalCalls.Prism_Material_SetTexture(m_UnmanagedInstance, uniform, texture.m_UnmanagedInstance); }
        }

        public void SetTexture(string uniform, Texture2D texture)
        {
            unsafe { InternalCalls.Prism_Material_SetTexture(m_UnmanagedInstance, uniform, texture.m_UnmanagedInstance); }
        }

        public Material(string shaderName)
        {
            unsafe { m_UnmanagedInstance = InternalCalls.Prism_Material_Constructor(shaderName); }
        }

        internal Material(IntPtr unmanagedInstance)
        {
            m_UnmanagedInstance = unmanagedInstance;
        }

        ~Material()
        {
            unsafe { InternalCalls.Prism_Material_Destructor(m_UnmanagedInstance); }
        }

        internal IntPtr m_UnmanagedInstance;

        public void SetKeyword(string name, bool enabled)
        {
            unsafe { InternalCalls.Prism_Material_SetKeyword(m_UnmanagedInstance, name, enabled); }
        }

        public bool IsKeywordEnabled(string name)
        {
            unsafe { return InternalCalls.Prism_Material_IsKeywordEnabled(m_UnmanagedInstance, name); }
        }
    }
}
