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

        // 序数必须与 PrismShaderCompiler::GLSLType 一致（PrismShaderCore/include/PrismShaderCore/PSL/GLSLType.h）
        private enum ComputeUniformType
        {
            Bool = 2, Int = 3, UInt = 4, Float = 5,
            BVec2 = 7, BVec3 = 8, BVec4 = 9,
            IVec2 = 10, IVec3 = 11, IVec4 = 12,
            UVec2 = 13, UVec3 = 14, UVec4 = 15,
            Vec2 = 16, Vec3 = 17, Vec4 = 18,
        }

        public void SetBool(int kernel, string name, bool value)
        {
            unsafe
            {
                int data = value ? 1 : 0;
                InternalCalls.Prism_ComputeShader_SetUniformData(m_NativePtr, kernel, name, (int)ComputeUniformType.Bool, &data, 4);
            }
        }

        public void SetInt(int kernel, string name, int value)
        {
            unsafe { InternalCalls.Prism_ComputeShader_SetUniformData(m_NativePtr, kernel, name, (int)ComputeUniformType.Int, &value, 4); }
        }

        public void SetUInt(int kernel, string name, UInt32 value)
        {
            unsafe { InternalCalls.Prism_ComputeShader_SetUniformData(m_NativePtr, kernel, name, (int)ComputeUniformType.UInt, &value, 4); }
        }

        public void SetFloat(int kernel, string name, float value)
        {
            unsafe { InternalCalls.Prism_ComputeShader_SetUniformData(m_NativePtr, kernel, name, (int)ComputeUniformType.Float, &value, 4); }
        }

        public void SetVector2(int kernel, string name, Vector2 value)
        {
            unsafe { InternalCalls.Prism_ComputeShader_SetUniformData(m_NativePtr, kernel, name, (int)ComputeUniformType.Vec2, &value.X, 8); }
        }

        public void SetVector3(int kernel, string name, Vector3 value)
        {
            unsafe { InternalCalls.Prism_ComputeShader_SetUniformData(m_NativePtr, kernel, name, (int)ComputeUniformType.Vec3, &value.X, 12); }
        }

        public void SetVector4(int kernel, string name, Vector4 value)
        {
            unsafe { InternalCalls.Prism_ComputeShader_SetUniformData(m_NativePtr, kernel, name, (int)ComputeUniformType.Vec4, &value.X, 16); }
        }

        public void SetIntVector2(int kernel, string name, int x, int y)
        {
            unsafe
            {
                int* data = stackalloc int[2] { x, y };
                InternalCalls.Prism_ComputeShader_SetUniformData(m_NativePtr, kernel, name, (int)ComputeUniformType.IVec2, data, 8);
            }
        }

        public void SetIntVector3(int kernel, string name, int x, int y, int z)
        {
            unsafe
            {
                int* data = stackalloc int[3] { x, y, z };
                InternalCalls.Prism_ComputeShader_SetUniformData(m_NativePtr, kernel, name, (int)ComputeUniformType.IVec3, data, 12);
            }
        }

        public void SetIntVector4(int kernel, string name, int x, int y, int z, int w)
        {
            unsafe
            {
                int* data = stackalloc int[4] { x, y, z, w };
                InternalCalls.Prism_ComputeShader_SetUniformData(m_NativePtr, kernel, name, (int)ComputeUniformType.IVec4, data, 16);
            }
        }

        public void SetUIntVector2(int kernel, string name, UInt32 x, UInt32 y)
        {
            unsafe
            {
                UInt32* data = stackalloc UInt32[2] { x, y };
                InternalCalls.Prism_ComputeShader_SetUniformData(m_NativePtr, kernel, name, (int)ComputeUniformType.UVec2, data, 8);
            }
        }

        public void SetUIntVector3(int kernel, string name, UInt32 x, UInt32 y, UInt32 z)
        {
            unsafe
            {
                UInt32* data = stackalloc UInt32[3] { x, y, z };
                InternalCalls.Prism_ComputeShader_SetUniformData(m_NativePtr, kernel, name, (int)ComputeUniformType.UVec3, data, 12);
            }
        }

        public void SetUIntVector4(int kernel, string name, UInt32 x, UInt32 y, UInt32 z, UInt32 w)
        {
            unsafe
            {
                UInt32* data = stackalloc UInt32[4] { x, y, z, w };
                InternalCalls.Prism_ComputeShader_SetUniformData(m_NativePtr, kernel, name, (int)ComputeUniformType.UVec4, data, 16);
            }
        }

        public void SetBoolVector2(int kernel, string name, bool x, bool y)
        {
            unsafe
            {
                int* data = stackalloc int[2] { x ? 1 : 0, y ? 1 : 0 };
                InternalCalls.Prism_ComputeShader_SetUniformData(m_NativePtr, kernel, name, (int)ComputeUniformType.BVec2, data, 8);
            }
        }

        public void SetBoolVector3(int kernel, string name, bool x, bool y, bool z)
        {
            unsafe
            {
                int* data = stackalloc int[3] { x ? 1 : 0, y ? 1 : 0, z ? 1 : 0 };
                InternalCalls.Prism_ComputeShader_SetUniformData(m_NativePtr, kernel, name, (int)ComputeUniformType.BVec3, data, 12);
            }
        }

        public void SetBoolVector4(int kernel, string name, bool x, bool y, bool z, bool w)
        {
            unsafe
            {
                int* data = stackalloc int[4] { x ? 1 : 0, y ? 1 : 0, z ? 1 : 0, w ? 1 : 0 };
                InternalCalls.Prism_ComputeShader_SetUniformData(m_NativePtr, kernel, name, (int)ComputeUniformType.BVec4, data, 16);
            }
        }

        public void Dispatch(int kernel, UInt32 groupsX, UInt32 groupsY, UInt32 groupsZ, bool force = false)
        {
            unsafe { InternalCalls.Prism_ComputeShader_Dispatch(m_NativePtr, kernel, groupsX, groupsY, groupsZ, force); }
        }
    }
}
