using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;

namespace Rolky.Managed.Project;

public class ScriptSolution
{
    public string Name { get; set; } = "";
    public string Directory { get; set; } = "";
    public List<ScriptProject> Projects { get; } = new();

    public string SolutionPath => Path.Combine(Directory, $"{Name}.sln");

    public ScriptSolution(string name, string directory)
    {
        Name = name;
        Directory = directory;
    }

    public ScriptSolution() { }

    public void AddProject(ScriptProject project)
    {
        Projects.Add(project);
    }

    public ScriptProject? GetProject(string name)
    {
        return Projects.Find(p => p.Name == name);
    }

    public bool HasProject(string name)
    {
        return Projects.Exists(p => p.Name == name);
    }

    public void GenerateAll()
    {
        foreach (var project in Projects)
            project.Save();
        Save();
    }

    public void Save()
    {
        var projectDeclarations = "";
        var slnPlatforms = "";
        var projPlatforms = "";
        var configs = new[] { "Debug", "Release" };

        foreach (var project in Projects)
        {
            var guid = Guid.NewGuid().ToString("B").ToUpper();
            var relPath = Path.GetRelativePath(Directory, project.ProjectPath);

            projectDeclarations += string.Format(CultureInfo.InvariantCulture,
                "Project(\"{{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}}\") = \"{0}\", \"{1}\", \"{2}\"\nEndProject\n",
                project.Name, relPath, guid);

            foreach (var cfg in configs)
            {
                projPlatforms += string.Format(CultureInfo.InvariantCulture,
                    "\t\t{0}.{1}|Any CPU.ActiveCfg = {1}|Any CPU\n\t\t{0}.{1}|Any CPU.Build.0 = {1}|Any CPU\n",
                    guid, cfg);
            }
        }

        foreach (var cfg in configs)
        {
            slnPlatforms += string.Format(CultureInfo.InvariantCulture,
                "\t\t{0}|Any CPU = {0}|Any CPU\n", cfg);
        }

        var content = string.Format(CultureInfo.InvariantCulture,
            "Microsoft Visual Studio Solution File, Format Version 12.00\n" +
            "# Visual Studio Version 17\n" +
            "VisualStudioVersion = 17.0.31903.59\n" +
            "MinimumVisualStudioVersion = 10.0.40219.1\n" +
            "{0}" +
            "Global\n" +
            "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n" +
            "{1}" +
            "\tEndGlobalSection\n" +
            "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n" +
            "{2}" +
            "\tEndGlobalSection\n" +
            "\tGlobalSection(SolutionProperties) = preSolution\n" +
            "\t\tHideSolutionNode = FALSE\n" +
            "\tEndGlobalSection\n" +
            "EndGlobal\n",
            projectDeclarations, slnPlatforms, projPlatforms);

        System.IO.Directory.CreateDirectory(Directory);
        System.IO.File.WriteAllText(SolutionPath, content);
    }
}
