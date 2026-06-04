using System;
using System.Runtime.InteropServices;
using Rolky.Managed.Interop;

namespace Rolky.Managed.Project;

public static class ProjectBridge
{
    [UnmanagedCallersOnly]
    public static unsafe void GenerateProject(ProjectDesc* descPtr)
    {
        var desc = *descPtr;
        var project = new ScriptProject(
            desc.Name.ToString() ?? "Game",
            desc.Directory.ToString() ?? "."
        );
        project.OutputDirectory = desc.OutputDirectory.ToString() ?? "";
        project.AllowUnsafe = desc.AllowUnsafe;

        var sources = (desc.SourceFiles.ToString() ?? "").Split(';', StringSplitOptions.RemoveEmptyEntries);
        foreach (var s in sources) project.SourceFiles.Add(s.Trim());

        var refs = (desc.ReferencePaths.ToString() ?? "").Split(';', StringSplitOptions.RemoveEmptyEntries);
        foreach (var r in refs) project.References.Add(r.Trim());

        var defines = (desc.Defines.ToString() ?? "").Split(';', StringSplitOptions.RemoveEmptyEntries);
        foreach (var d in defines) project.Defines.Add(d.Trim());

        project.Save();
    }

    [UnmanagedCallersOnly]
    public static void GenerateSolution(NativeString outputDir, NativeString solutionName, NativeArray<ProjectDesc> projects)
    {
        var solution = new ScriptSolution(
            solutionName.ToString() ?? "Game",
            outputDir.ToString() ?? "."
        );

        foreach (var desc in projects)
        {
            var project = new ScriptProject(
                desc.Name.ToString() ?? "",
                desc.Directory.ToString() ?? outputDir.ToString() ?? "."
            );
            project.OutputDirectory = desc.OutputDirectory.ToString() ?? "";
            project.AllowUnsafe = desc.AllowUnsafe;

            var sources = (desc.SourceFiles.ToString() ?? "").Split(';', StringSplitOptions.RemoveEmptyEntries);
            foreach (var s in sources) project.SourceFiles.Add(s.Trim());

            var refs = (desc.ReferencePaths.ToString() ?? "").Split(';', StringSplitOptions.RemoveEmptyEntries);
            foreach (var r in refs) project.References.Add(r.Trim());

            var defines = (desc.Defines.ToString() ?? "").Split(';', StringSplitOptions.RemoveEmptyEntries);
            foreach (var d in defines) project.Defines.Add(d.Trim());

            project.Save();
            solution.Projects.Add(project);
        }

        solution.Save();
    }
}
