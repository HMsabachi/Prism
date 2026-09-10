using System;

namespace Prism
{
    public class UniformBuffer : RefCounted
    {
        protected UniformBuffer(nint nativePtr) : base(nativePtr) { }
        public static UniformBuffer Create(UInt32 size)
        {
            IntPtr nativePtr = IntPtr.Zero;
            unsafe { nativePtr = InternalCalls.Prism_UniformBuffer_Constructor(size); }
            return new UniformBuffer(nativePtr);
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
