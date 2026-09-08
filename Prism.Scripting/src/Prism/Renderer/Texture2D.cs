using System;
using System.Runtime.CompilerServices;

using Rolky.Managed.Interop;
namespace Prism
{
    public class Texture2D : Asset
    {

        public Texture2D(uint width, uint height) : base(IntPtr.Zero)
        {
            unsafe { m_NativePtr = InternalCalls.Prism_Texture2D_Constructor(width, height);}
        }
        internal Texture2D(IntPtr nativePtr) : base(nativePtr) { }

        public void SetData(Vector4[] data)
        {
            NativeArray<Vector4> d = new(data);
            unsafe { InternalCalls.Prism_Texture2D_SetData(m_NativePtr, d, d.Length);}
        }
    }
}
