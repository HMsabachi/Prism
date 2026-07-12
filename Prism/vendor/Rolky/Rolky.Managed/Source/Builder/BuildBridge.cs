using System.Runtime.InteropServices;
using Rolky.Managed.Interop;

namespace Rolky.Managed.Build;

public static class BuildBridge
{
    [UnmanagedCallersOnly]
    public static Bool32 BuildProjectBlocking(NativeString configuration, NativeString platform, Bool32 rebuild)
    {
        return BuildManager.BuildProjectBlocking(
            configuration.ToString() ?? "Debug",
            platform.ToString(),
            rebuild);
    }

    [UnmanagedCallersOnly]
    public static Bool32 CleanProjectBlocking(NativeString configuration)
    {
        return BuildManager.CleanProjectBlocking(configuration.ToString() ?? "Debug");
    }

    [UnmanagedCallersOnly]
    public static void SetScriptBuildSettings(NativeString solutionPath, NativeString projectPath, NativeString logsDirectory)
    {
        ScriptBuildSettings.SolutionPath = solutionPath.ToString() ?? "";
        ScriptBuildSettings.ProjectPath = projectPath.ToString();
        ScriptBuildSettings.LogsDirectory = logsDirectory.ToString() ?? "";
    }
}
