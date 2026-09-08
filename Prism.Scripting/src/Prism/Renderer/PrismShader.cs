using System;
using Rolky.Managed.Interop;
#pragma warning disable CS8603

namespace Prism
{

    public enum UniformType : UInt32
    {
        None,
        Bool,
        Color,
        Color3,
        Float,
        Int,
        Vector2,
        Vector3,
        Vector4,
        Range,
        Matrix3,
        Matrix4,
        Texture2D,
        Texture2DMS,
        TextureCube,
        Enum,
    };

    public class PrismShader : Asset
    {
        public static PrismShader GetShader(string shaderName)
        {
            unsafe
            {
                IntPtr nativePtr = InternalCalls.Prism_PrismShader_GetShader(shaderName);
                if (nativePtr == IntPtr.Zero) throw new NullReferenceException($"Shader '{shaderName}' not found.");
                return new PrismShader(nativePtr);
            }
        }


        protected PrismShader(IntPtr nativePtr) : base(nativePtr) { }
        public string Name { get { return GetName(); }}
        public string GetName()
        {
            using(var str = new NativeString())
            {
                unsafe { InternalCalls.Prism_PrismShader_GetName(m_NativePtr, &str); }
                return str;
            }
        }
        public UInt32 GetUniformCount()
        {
            unsafe { return InternalCalls.Prism_PrismShader_GetUniformCount(m_NativePtr); }
        }
        public UniformType GetUniformType(UInt32 index)
        {
            unsafe { return InternalCalls.Prism_PrismShader_GetUniformType(m_NativePtr, index); }
        }
        public string GetUniformName(UInt32 index)
        {
            using(var str = new NativeString())
            {
                unsafe { InternalCalls.Prism_PrismShader_GetUniformName(m_NativePtr, index, &str); }
                return str;
            }
        }
        public string GetUniformDisplayName(UInt32 index)
        {
            using (var str = new NativeString())
            {
                unsafe { InternalCalls.Prism_PrismShader_GetUniformDisplayName(m_NativePtr, index, &str); }
                return str;
            }
        }

        public T GetUniformDefaultValue<T>(UInt32 index) where T : struct
        {
            unsafe
            {
                T value = default;
                IntPtr valuePtr = (IntPtr)(&value);
                InternalCalls.Prism_PrismShader_GetUniformDefualtValue(m_NativePtr, index, valuePtr);
                return value;
            }
        }

        public override string ToString()
        {
            string result =
                $"PrismShader({m_NativePtr})\n" +
                $"Name: {Name}";
            return result;
        }
    }
}
