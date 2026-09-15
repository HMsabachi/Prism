using System;
using System.Runtime.CompilerServices;

using Rolky.Managed.Interop;
namespace Prism
{
    public class Texture : Asset
    {
        internal Texture(IntPtr nativePtr) : base(nativePtr) { }
        public UInt32 Width => GetWidth();
        public UInt32 Height => GetHeight();
        public ImageFormat Format => GetFormat();
        public UInt32 GetWidth() { unsafe { return InternalCalls.Prism_Texture_GetWidth(m_NativePtr); } }
        public UInt32 GetHeight() { unsafe { return InternalCalls.Prism_Texture_GetHeight(m_NativePtr); } }
        public ImageFormat GetFormat() { unsafe { return InternalCalls.Prism_Texture_GetFormat(m_NativePtr); } }
    }
    public class Texture2D : Texture
    {
        internal Texture2D(IntPtr nativePtr) : base(nativePtr) { }
        public static Texture2D Create(uint width, uint height)
        {
            IntPtr nativePtr = IntPtr.Zero;
            unsafe { nativePtr = InternalCalls.Prism_Texture2D_Constructor(width, height); }
            return new Texture2D(nativePtr);
        }

        public void SetData(Vector4[] data)
        {
            NativeArray<Vector4> d = new(data);
            unsafe { InternalCalls.Prism_Texture2D_SetData(m_NativePtr, d, d.Length);}
        }
        public Image2D GetImage()
        {
            IntPtr nativePtr = IntPtr.Zero;
            unsafe { nativePtr = InternalCalls.Prism_Texture2D_GetImage(m_NativePtr); }
            return new Image2D(nativePtr);
        }
    }
    public class TextureCube : Texture
    {
        internal TextureCube(IntPtr nativePtr) : base(nativePtr) { }
        public static unsafe TextureCube Create<T>(ImageFormat format, UInt32 width, UInt32 height, in T[] data)
            where T : unmanaged
        {
            IntPtr nativePtr = IntPtr.Zero;
            fixed (T* ptr = data)
            {
                nativePtr = InternalCalls.Prism_TextureCube_Constructor(format, width, height, (IntPtr)ptr);
            }
            return new TextureCube(nativePtr);
        }
        public ImageCube GetImage()
        {
            IntPtr nativePtr = IntPtr.Zero;
            unsafe { nativePtr = InternalCalls.Prism_TextureCube_GetImage(m_NativePtr); }
            return new ImageCube(nativePtr);
        }
    }
}
