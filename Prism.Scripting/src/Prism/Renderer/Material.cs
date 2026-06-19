using System;

namespace Prism
{
    public class Material
    {
        public static Material DefaultMaterial
        {
            get
            {
                IntPtr ptr = IntPtr.Zero;
                unsafe { InternalCalls.Prism_Material_GetDefaultMaterial(&ptr); }
                return new Material(ptr);
            }
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

        public void SetFloat(string uniform, float value)
        {
            unsafe { InternalCalls.Prism_Material_SetFloat(m_UnmanagedInstance, uniform, value); }
        }

        public void SetInt(string uniform, int value)
        {
            unsafe { InternalCalls.Prism_Material_SetInt(m_UnmanagedInstance, uniform, value); }
        }

        public void SetBool(string uniform, bool value)
        {
            unsafe { InternalCalls.Prism_Material_SetBool(m_UnmanagedInstance, uniform, value); }
        }

        public void SetVec2(string uniform, Vector2 value)
        {
            unsafe { InternalCalls.Prism_Material_SetVector2(m_UnmanagedInstance, uniform, &value); }
        }

        public void SetColor(string uniform, Vector3 value)
        {
            unsafe { InternalCalls.Prism_Material_SetColor3(m_UnmanagedInstance, uniform, &value); }
        }

        public void SetColor(string uniform, Vector4 value)
        {
            unsafe { InternalCalls.Prism_Material_SetColor(m_UnmanagedInstance, uniform, &value); }
        }

        public void SetMatrix4(string uniform, Matrix4 value)
        {
            unsafe { InternalCalls.Prism_Material_SetMatrix4(m_UnmanagedInstance, uniform, &value); }
        }

        public void SetVector3(string uniform, Vector3 value)
        {
            unsafe { InternalCalls.Prism_Material_SetVector3(m_UnmanagedInstance, uniform, &value); }
        }

        public void SetVector4(string uniform, Vector4 value)
        {
            unsafe { InternalCalls.Prism_Material_SetVector4(m_UnmanagedInstance, uniform, &value); }
        }

        public void SetTexture(string uniform, Texture2D texture)
        {
            unsafe { InternalCalls.Prism_Material_SetTexture(m_UnmanagedInstance, uniform, texture.m_UnmanagedInstance); }
        }

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
