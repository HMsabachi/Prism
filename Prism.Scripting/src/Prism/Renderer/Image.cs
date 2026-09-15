
using System;

namespace Prism
{
    public enum ImageFormat
    {
        None = 0,

        // 8-bit
        R8,
        RG8,
        RGB8,
        RGBA8,

        R8_SRGB,
        RG8_SRGB,
        RGB8_SRGB,
        RGBA8_SRGB,

        R8_SNORM,
        RG8_SNORM,
        RGB8_SNORM,
        RGBA8_SNORM,

        // 16-bit Float
        R16F,
        RG16F,
        RGB16F,
        RGBA16F,

        // 32-bit Float
        R32F,
        RG32F,
        RGB32F,
        RGBA32F,

        // Integer
        R16_UINT,
        RG16_UINT,
        RGBA16_UINT,

        R32_UINT,
        RG32_UINT,
        RGBA32_UINT,

        R16_SINT,
        RG16_SINT,
        RGBA16_SINT,

        R32_SINT,
        RG32_SINT,
        RGBA32_SINT,

        // Packed
        RGB565,
        RGBA4,
        RGB5A1,
        RGB10A2,
        RG11B10F,
        RGB9E5,

        // Depth / Stencil
        DEPTH16,
        DEPTH24STENCIL8,
        DEPTH32F,
        DEPTH32FSTENCIL8,

        // BC
        BC1,
        BC1_SRGB,
        BC2,
        BC2_SRGB,
        BC3,
        BC3_SRGB,
        BC4,
        BC5,
        BC6H_UF16,
        BC6H_SF16,
        BC7,
        BC7_SRGB,

        // ETC2
        ETC2_RGB8,
        ETC2_RGB8_SRGB,
        ETC2_RGBA8,
        ETC2_RGBA8_SRGB,

        // ASTC
        ASTC_4x4,
        ASTC_5x5,
        ASTC_6x6,
        ASTC_8x8,

        // Defaults
        Depth = DEPTH24STENCIL8
    };
    public class Image : RefCounted
    {
        internal Image(nint nativePtr) : base(nativePtr) { }
        public UInt32 Width => GetWidth();
        public UInt32 Height => GetHeight();
        public UInt32 Samples => GetSamples();
        public ImageFormat Format => GetFormat();
        public unsafe UInt32 GetWidth()
        {
            return InternalCalls.Prism_Image_GetWidth(m_NativePtr);
        }
        public unsafe UInt32 GetHeight()
        {
            return InternalCalls.Prism_Image_GetHeight(m_NativePtr);
        }
        public unsafe UInt32 GetSamples()
        {
            return InternalCalls.Prism_Image_GetSamples(m_NativePtr);
        }
        public unsafe ImageFormat GetFormat()
        {
            return InternalCalls.Prism_Image_GetFormat(m_NativePtr);
        }
    }
    public class Image2D : Image
    {
        internal Image2D(nint nativePtr) : base(nativePtr) { }
        public static unsafe Image2D Create<T>(ImageFormat format, UInt32 width, UInt32 height, in T[] data, UInt32 samples = 1)
            where T : unmanaged
        {
            IntPtr nativePtr = IntPtr.Zero;
            fixed (T* ptr = data)
            {
                nativePtr = InternalCalls.Prism_Image2D_Constructor(format, width, height, (IntPtr)ptr, samples);
            }
            return new Image2D(nativePtr);
        }
    }

    public class ImageCube : Image
    {
        internal ImageCube(nint nativePtr) : base(nativePtr) { }
        public static unsafe ImageCube Create<T>(ImageFormat format, UInt32 width, UInt32 height, in T[] data)
            where T : unmanaged
        {
            IntPtr nativePtr = IntPtr.Zero;
            fixed (T* ptr = data)
            {
                nativePtr = InternalCalls.Prism_ImageCube_Constructor(format, width, height, (IntPtr)ptr);
            }
            return new ImageCube(nativePtr);
        }
        public void GenerateMipMap()
        {
            unsafe { InternalCalls.Prism_ImageCube_GenerateMipMap(m_NativePtr); }
        }
        public void CopyTo(ImageCube target)
        {
            unsafe { InternalCalls.Prism_ImageCube_CopyTo(m_NativePtr, target.GetNativePtr()); }
        }

    }
}
