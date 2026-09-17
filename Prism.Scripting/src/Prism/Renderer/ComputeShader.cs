using System;
using Rolky.Managed.Interop;

namespace Prism
{
    public class ComputeShader : Asset
    {
        internal ComputeShader(IntPtr nativePtr) : base(nativePtr) { }

        public static ComputeShader Create(string filePath)
        {
            unsafe
            {
                IntPtr nativePtr = InternalCalls.Prism_ComputeShader_Constructor(filePath);
                if (nativePtr == IntPtr.Zero) throw new NullReferenceException($"ComputeShader '{filePath}' load failed.");
                return new ComputeShader(nativePtr);
            }
        }

        public string Name { get { return GetName(); } }

        public string GetName()
        {
            using (var str = new NativeString())
            {
                unsafe { InternalCalls.Prism_ComputeShader_GetName(m_NativePtr, &str); }
                return str;
            }
        }

        public UInt32 GetKernelCount()
        {
            unsafe { return InternalCalls.Prism_ComputeShader_GetKernelCount(m_NativePtr); }
        }

        public int FindKernel(string name)
        {
            unsafe { return InternalCalls.Prism_ComputeShader_FindKernel(m_NativePtr, name); }
        }

        public bool HasKernel(string name)
        {
            unsafe { return InternalCalls.Prism_ComputeShader_HasKernel(m_NativePtr, name); }
        }

        public void GetKernelThreadGroupSizes(int kernel, out UInt32 x, out UInt32 y, out UInt32 z)
        {
            unsafe
            {
                UInt32 sizeX = 0, sizeY = 0, sizeZ = 0;
                InternalCalls.Prism_ComputeShader_GetKernelThreadGroupSizes(m_NativePtr, kernel, &sizeX, &sizeY, &sizeZ);
                x = sizeX;
                y = sizeY;
                z = sizeZ;
            }
        }

        public void SetUniformBuffer(int kernel, string name, UniformBuffer buffer)
        {
            unsafe { InternalCalls.Prism_ComputeShader_SetUniformBuffer(m_NativePtr, kernel, name, buffer.GetNativePtr()); }
        }

        public void SetBuffer(int kernel, string name, ShaderStorageBuffer buffer)
        {
            unsafe { InternalCalls.Prism_ComputeShader_SetBuffer(m_NativePtr, kernel, name, buffer.GetNativePtr()); }
        }

        public void SetTexture2D(int kernel, string name, Image2D image)
        {
            unsafe { InternalCalls.Prism_ComputeShader_SetTexture2D(m_NativePtr, kernel, name, image.GetNativePtr()); }
        }

        public void SetTextureCube(int kernel, string name, ImageCube image)
        {
            unsafe { InternalCalls.Prism_ComputeShader_SetTextureCube(m_NativePtr, kernel, name, image.GetNativePtr()); }
        }

        public void SetImage2D(int kernel, string name, Image2D image, UInt32 level = 0)
        {
            unsafe { InternalCalls.Prism_ComputeShader_SetImage2D(m_NativePtr, kernel, name, image.GetNativePtr(), level); }
        }

        public void SetImageCube(int kernel, string name, ImageCube image, UInt32 level = 0)
        {
            unsafe { InternalCalls.Prism_ComputeShader_SetImageCube(m_NativePtr, kernel, name, image.GetNativePtr(), level); }
        }

        public void Dispatch(int kernel, UInt32 groupsX, UInt32 groupsY, UInt32 groupsZ)
        {
            unsafe { InternalCalls.Prism_ComputeShader_Dispatch(m_NativePtr, kernel, groupsX, groupsY, groupsZ); }
        }
    }
}
