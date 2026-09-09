using System;

namespace Prism
{
    public class UniformBuffer : RefCounted
    {
        protected UniformBuffer(nint nativePtr) : base(nativePtr) { }

        public UniformBuffer(UInt32 size) : base(IntPtr.Zero)
        {
            unsafe { m_NativePtr = InternalCalls.Prism_UniformBuffer_Constructor(size); }
        }

        public unsafe void SetData<T>(in T[] data) where T : unmanaged
        {
            fixed (T* ptr = data)
            {
                InternalCalls.Prism_UniformBuffer_SetData(m_NativePtr, (IntPtr)ptr, (UInt32)(sizeof(T) * data.Length));
            }
        }
        public unsafe void SetData<T>(in T data) where T : unmanaged
        {
            fixed (T* ptr = &data)
            {
                InternalCalls.Prism_UniformBuffer_SetData(m_NativePtr, (IntPtr)ptr, (UInt32)sizeof(T));
            }
            
        }

    }
}
