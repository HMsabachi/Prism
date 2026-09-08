

using System;

namespace Prism
{
    public class RefCounted
    {
        protected IntPtr m_NativePtr = IntPtr.Zero;
        internal IntPtr NativePtr => m_NativePtr;
        protected RefCounted(IntPtr nativePtr) => m_NativePtr = nativePtr;
        ~RefCounted()
        {
            if (m_NativePtr != IntPtr.Zero)
            {
                unsafe { InternalCalls.Prism_RefCounted_Destructor(m_NativePtr); }
                m_NativePtr = IntPtr.Zero;
            }
        }
    }
}
