using System.Collections.Generic;

namespace Rolky.Managed.Build;

public static class ScriptBuildSettings
{
    public static string SolutionPath { get; set; } = "";
    public static string? ProjectPath { get; set; }
    public static string LogsDirectory { get; set; } = "";
    public static List<string> AssemblySearchPaths { get; set; } = new();
}
