using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

using Rolky.Managed.Interop;
namespace Prism
{
    [StructLayout(LayoutKind.Sequential)]
    public struct TextureSpecification
    {
        public ImageFormat Format;
        public UInt32 Width;
        public UInt32 Height;
        public TextureWrap SamplerWrap;
        public TextureFilter SamplerFilter;
        public Bool32 GenerateMips;

        public TextureSpecification()
        {
            Format = ImageFormat.RGBA8;
            Width = 1;
            Height = 1;
            SamplerWrap = TextureWrap.Repeat;
            SamplerFilter = TextureFilter.Linear;
            GenerateMips = true;
        }
    }
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
            TextureSpecification specification = new TextureSpecification();
            specification.Width = width;
            specification.Height = height;
            return Create(specification);
        }
        public static unsafe Texture2D Create(TextureSpecification specification)
        {
            IntPtr nativePtr = InternalCalls.Prism_Texture2D_Constructor(&specification);
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
        public static TextureCube Create<T>(ImageFormat format, UInt32 width, UInt32 height, in T[] data)
            where T : unmanaged
        {
            TextureSpecification specification = new TextureSpecification();
            specification.Format = format;
            specification.Width = width;
            specification.Height = height;
            return Create(specification, data);
        }
        public static unsafe TextureCube Create<T>(TextureSpecification specification, in T[] data)
            where T : unmanaged
        {
            IntPtr nativePtr = IntPtr.Zero;
            fixed (T* ptr = data)
            {
                nativePtr = InternalCalls.Prism_TextureCube_Constructor(&specification, (IntPtr)ptr);
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
