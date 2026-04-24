// NativeString.cs
using System;
using System.Runtime.InteropServices;
using System.Text;

namespace Prism
{
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct NativeString : IDisposable
    {
        public byte* data;        // 指向C++分配的字符串数据
        public uint length;       // 字符串长度
        public uint capacity;     // 分配容量
        // 从C#字符串创建NativeString
        public static unsafe NativeString FromString(string managedString)
        {
            var nativeStr = new NativeString();

            if (string.IsNullOrEmpty(managedString))
            {
                nativeStr.data = null;
                nativeStr.length = 0;
                nativeStr.capacity = 0;
                return nativeStr;
            }

            // 转换为UTF-8字节并创建fixed指针
            byte[] utf8Bytes = Encoding.UTF8.GetBytes(managedString);
            fixed (byte* ptr = utf8Bytes)
            {
                // 调用C++函数创建NativeString
                nativeStr = InternalCalls.Funcs.String_CreateNativeString_Native(ptr);
            }

            return nativeStr;
        }

        // 转换为C#字符串
        public new unsafe string ToString() 
        {
            if (data == null)
            {
                return string.Empty;
            }

            // 调用C++函数获取C风格字符串
            byte* cstrPtr;
            fixed (NativeString* self = &this)
            {
                cstrPtr = InternalCalls.Funcs.String_NativeStringToCString_Native(self);
            }
            if (cstrPtr == null)
            {
                return string.Empty;
            }

            // 计算字符串长度并转换为托管字符串
            int len = 0;
            while (cstrPtr[len] != 0) len++;

            return Encoding.UTF8.GetString(cstrPtr, len);
        }

        // 释放NativeString占用的内存
        public unsafe void Free()
        {
            fixed (NativeString* self = &this)
                InternalCalls.Funcs.String_FreeNativeString_Native(self);
        }

        // 拷贝NativeString
        public unsafe NativeString Copy()
        {
            return InternalCalls.Funcs.String_CopyNativeString_Native(this);
        }

        // 隐式转换：string -> NativeString
        public static implicit operator NativeString(string managedString)
        {
            return FromString(managedString);
        }

        // 隐式转换：NativeString -> string
        public static implicit operator string(NativeString nativeString)
        {
            return nativeString.ToString();
        }

        public void Dispose()
        {
            Free();
        }
    }
}