
using System;

namespace Prism
{
    public enum AssetType : UInt32
    {
        Scene, Mesh, Texture, EnvMap, Audio, Script, PhysicsMat, Shader, Directory, Other, None, Missing
    };
    public class Asset : RefCounted
    {
        protected Asset(nint nativePtr) : base(nativePtr) { }

        public AssetType Type
        {
            get { unsafe { return InternalCalls.Prism_Asset_GetType(m_NativePtr); } }
        }
        public UInt64 Handle
        {
            get { unsafe { return InternalCalls.Prism_Asset_GetHandle(m_NativePtr); } }
        }
    }
}
