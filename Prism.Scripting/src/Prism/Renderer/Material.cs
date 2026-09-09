using System;

namespace Prism
{
    public class Material : RefCounted
    {

        public Material(PrismShader shader) : base(IntPtr.Zero)
        {
            unsafe { m_NativePtr = InternalCalls.Prism_Material_Constructor(shader.GetNativePtr()); }
        }

        internal Material(IntPtr nativePtr) : base(nativePtr) { }

        public void SetFloat(string uniform, float value)
        {
            unsafe { InternalCalls.Prism_Material_SetFloat(m_NativePtr, uniform, value); }
        }

        public void SetInt(string uniform, int value)
        {
            unsafe { InternalCalls.Prism_Material_SetInt(m_NativePtr, uniform, value); }
        }

        public void SetBool(string uniform, bool value)
        {
            unsafe { InternalCalls.Prism_Material_SetBool(m_NativePtr, uniform, value); }
        }

        public void SetVec2(string uniform, Vector2 value)
        {
            unsafe { InternalCalls.Prism_Material_SetVector2(m_NativePtr, uniform, &value); }
        }

        public void SetColor(string uniform, Vector3 value)
        {
            unsafe { InternalCalls.Prism_Material_SetColor3(m_NativePtr, uniform, &value); }
        }

        public void SetColor(string uniform, Vector4 value)
        {
            unsafe { InternalCalls.Prism_Material_SetColor(m_NativePtr, uniform, &value); }
        }

        public void SetMatrix4(string uniform, Matrix4 value)
        {
            unsafe { InternalCalls.Prism_Material_SetMatrix4(m_NativePtr, uniform, &value); }
        }

        public void SetVector3(string uniform, Vector3 value)
        {
            unsafe { InternalCalls.Prism_Material_SetVector3(m_NativePtr, uniform, &value); }
        }

        public void SetVector4(string uniform, Vector4 value)
        {
            unsafe { InternalCalls.Prism_Material_SetVector4(m_NativePtr, uniform, &value); }
        }

        public void SetTexture2D(string uniform, Texture2D texture)
        {
            unsafe { InternalCalls.Prism_Material_SetTexture(m_NativePtr, uniform, texture.GetNativePtr()); }
        }

        public void SetKeyword(string name, bool enabled)
        {
            unsafe { InternalCalls.Prism_Material_SetKeyword(m_NativePtr, name, enabled); }
        }

        public bool IsKeywordEnabled(string name)
        {
            unsafe { return InternalCalls.Prism_Material_IsKeywordEnabled(m_NativePtr, name); }
        }
    }
}
