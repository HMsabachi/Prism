using System;

namespace Prism
{
    public class RefCounted
    {
        protected readonly IntPtr m_NativePtr = IntPtr.Zero;
        internal IntPtr GetNativePtr() => m_NativePtr;
        internal RefCounted(IntPtr nativePtr) => m_NativePtr = nativePtr;
        ~RefCounted()
        {
            if (m_NativePtr != IntPtr.Zero)
                unsafe { InternalCalls.Prism_RefCounted_Destructor(m_NativePtr); }
        }
        public bool IsValid() => m_NativePtr != IntPtr.Zero;
    }
}
