using System;

namespace Prism
{
    public enum BufferUsage : UInt32
    {
        None = 0,
        Static = 1,
        Dynamic = 2,
    };

    public class ShaderStorageBufferReadback : RefCounted
    {
        internal ShaderStorageBufferReadback(IntPtr nativePtr) : base(nativePtr) { }
        public bool IsDone
        {
            get { unsafe { return InternalCalls.Prism_ShaderStorageBufferReadback_IsDone(m_NativePtr); } }
        }
        public UInt32 Size
        {
            get { unsafe { return InternalCalls.Prism_ShaderStorageBufferReadback_GetSize(m_NativePtr); } }
        }
        public unsafe void GetData<T>(in T[] data) where T : unmanaged
        {
            fixed (T* ptr = data)
            {
                InternalCalls.Prism_ShaderStorageBufferReadback_GetData(m_NativePtr, (IntPtr)ptr);
            }
        }
        public unsafe void GetData<T>(ref T data) where T : unmanaged
        {
            fixed (T* ptr = &data)
            {
                InternalCalls.Prism_ShaderStorageBufferReadback_GetData(m_NativePtr, (IntPtr)ptr);
            }
        }
    }

    public class ShaderStorageBuffer : RefCounted
    {
        internal ShaderStorageBuffer(IntPtr nativePtr) : base(nativePtr) { }

        public static ShaderStorageBuffer Create(UInt32 size, BufferUsage usage = BufferUsage.Dynamic)
        {
            IntPtr nativePtr = IntPtr.Zero;
            unsafe { nativePtr = InternalCalls.Prism_ShaderStorageBuffer_Constructor(size, (UInt32)usage); }
            return new ShaderStorageBuffer(nativePtr);
        }

        public UInt32 Size { get { unsafe { return InternalCalls.Prism_ShaderStorageBuffer_GetSize(m_NativePtr); } } }

        public unsafe void SetData<T>(in T[] data, UInt32 offset = 0) where T : unmanaged
        {
            fixed (T* ptr = data)
            {
                InternalCalls.Prism_ShaderStorageBuffer_SetData(m_NativePtr, (IntPtr)ptr, (UInt32)(sizeof(T) * data.Length), offset);
            }
        }

        public unsafe void SetData<T>(in T data, UInt32 offset = 0) where T : unmanaged
        {
            fixed (T* ptr = &data)
            {
                InternalCalls.Prism_ShaderStorageBuffer_SetData(m_NativePtr, (IntPtr)ptr, (UInt32)sizeof(T), offset);
            }
        }

        public ShaderStorageBufferReadback RequestReadback(UInt32 offset = 0, UInt32 size = 0)
        {
            IntPtr nativePtr = IntPtr.Zero;
            unsafe { nativePtr = InternalCalls.Prism_ShaderStorageBuffer_RequestReadback(m_NativePtr, offset, size); }
            return new ShaderStorageBufferReadback(nativePtr);
        }
    }
}
