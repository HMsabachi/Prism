using System.Collections.Generic;

namespace Rolky.Managed.Build;

public class BuildConfig
{
    public string SolutionPath { get; init; } = "";
    public string ProjectPath { get; init; } = "";
    public string Configuration { get; init; } = "Debug";
    public string? Platform { get; init; }
    public bool Restore { get; init; } = true;
    public bool Rebuild { get; init; }
    public bool OnlyClean { get; init; }
    public List<string> CustomProperties { get; init; } = new();
    public string LogsDirPath { get; init; } = "";
    public string? DotNetExePath { get; init; }
}
