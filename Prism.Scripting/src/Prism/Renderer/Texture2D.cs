using System;
using System.Runtime.CompilerServices;

using Rolky.Managed.Interop;
namespace Prism
{
    public class Texture2D
    {

        public Texture2D(uint width, uint height)
        {
            unsafe { m_UnmanagedInstance = InternalCalls.Prism_Texture2D_Constructor(width, height);}
        }

        ~Texture2D()
        {
            unsafe { InternalCalls.Prism_Texture2D_Destructor(m_UnmanagedInstance);}
        }

        public void SetData(Vector4[] data)
        {
            NativeArray<Vector4> d = new(data);
            unsafe { InternalCalls.Prism_Texture2D_SetData(m_UnmanagedInstance, d, d.Length);}
        }

        internal IntPtr m_UnmanagedInstance;
    }
}
