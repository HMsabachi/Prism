
using Rolky.Managed.Interop;
using System;
#pragma warning disable CS8603

namespace Prism
{
    public enum AssetType : UInt32
    {
        Scene, Mesh, Texture, EnvMap, Audio, Script, PhysicsMat, Shader, ComputeShader, Directory, Other, None, Missing
    };
    public class Asset : RefCounted
    {
        protected Asset(IntPtr nativePtr) : base(nativePtr) { }

        public AssetType Type
        {
            get { unsafe { return InternalCalls.Prism_Asset_GetType(m_NativePtr); } }
        }
        public UInt64 Handle
        {
            get { unsafe { return InternalCalls.Prism_Asset_GetHandle(m_NativePtr); } }
        }
        public string FilePath
        {
            get
            {
                using (var str = new NativeString())
                {
                    unsafe { InternalCalls.Prism_Asset_GetFilePath(m_NativePtr, &str); }
                    return str;
                }
            }
        }
        public string FileName
        {
            get
            {
                using (var str = new NativeString())
                {
                    unsafe { InternalCalls.Prism_Asset_GetFileName(m_NativePtr, &str); }
                    return str;
                }
            }
        }
        public string Extension
        {
            get
            {
                using (var str = new NativeString())
                {
                    unsafe { InternalCalls.Prism_Asset_GetExtension(m_NativePtr, &str); }
                    return str;
                }
            }
        }
    }
}
