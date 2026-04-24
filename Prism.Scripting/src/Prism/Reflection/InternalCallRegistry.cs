using System;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Prism
{
    public static class InternalCallRegistry
    {
        private static unsafe string BytePtrToString(byte* ptr)
        {
            if (ptr == null) return null;
            IntPtr intPtr = (IntPtr)ptr;
            string result = Marshal.PtrToStringAnsi(intPtr);
            return result?.Replace("\0", "");
        }
        [UnmanagedCallersOnly]
        public static unsafe void AddInternalCall(byte* className, byte* funcName, void* address)
        {
            string nsClass = BytePtrToString(className);
            string fieldName = BytePtrToString(funcName);
            Type? type = Type.GetType(nsClass);
            if (type == null)
            {
                Log.Error($"找不到类: {nsClass}");
                return;
            }
            FieldInfo? field = type.GetField(fieldName, BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic);
            if (field == null)
            {
                Log.Error($"在 {nsClass} 中找不到字段: {fieldName}");
                return;
            }
            field.SetValue(null, (IntPtr)address);
            Log.Trace($"InternalCall绑定成功: {nsClass}.{fieldName} -> 0x{(nint)address:X}");
        }
    }
}