using System.Runtime.InteropServices;
using Rolky.Managed.Interop;

namespace Rolky.Managed.Project;

[StructLayout(LayoutKind.Sequential, Pack = 8)]
public struct ProjectDesc
{
    public NativeString Name;
    public NativeString Directory;
    public NativeString OutputDirectory;
    public NativeString SourceFiles;
    public NativeString ReferencePaths;
    public NativeString Defines;
    public Bool32 AllowUnsafe;
}
