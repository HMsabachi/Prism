using System;
using System.Collections.Generic;
using System.IO;

namespace Rolky.Managed.Project;

public class ScriptProject
{
    public string Name { get; set; } = "";
    public string Directory { get; set; } = "";
    public string OutputDirectory { get; set; } = "";
    public List<string> SourceFiles { get; } = new();
    public List<string> References { get; } = new();
    public List<string> Defines { get; } = new();
    public List<ScriptProject> ProjectReferences { get; } = new();
    public bool AllowUnsafe { get; set; } = true;
    public string TargetFramework { get; set; } = "net9.0";

    public string ProjectPath => Path.Combine(Directory, $"{Name}.csproj");

    public ScriptProject(string name, string directory)
    {
        Name = name;
        Directory = directory;
    }

    public ScriptProject() { }

    public void Save()
    {
        var refsXml = "";
        foreach (var refPath in References)
        {
            var refName = Path.GetFileNameWithoutExtension(refPath);
            refsXml += $@"
    <Reference Include=""{refName}"">
      <HintPath>{refPath}</HintPath>
      <Private>false</Private>
    </Reference>";
        }

        foreach (var projRef in ProjectReferences)
        {
            refsXml += $@"
    <ProjectReference Include=""{projRef.ProjectPath}"">
      <Name>{projRef.Name}</Name>
    </ProjectReference>";
        }

        var definesStr = Defines.Count > 0
            ? $"<DefineConstants>{string.Join(";", Defines)}</DefineConstants>" : "";

        var sourcesXml = "";
        if (SourceFiles.Count > 0)
        {
            sourcesXml = "    <EnableDefaultCompileItems>false</EnableDefaultCompileItems>";
            foreach (var file in SourceFiles)
                sourcesXml += $"\n    <Compile Include=\"{file}\" />";
        }

        var csproj = $@"<Project Sdk=""Microsoft.NET.Sdk"">

  <PropertyGroup>
    <OutputType>Library</OutputType>
    <TargetFramework>{TargetFramework}</TargetFramework>
    <OutputPath>{OutputDirectory}</OutputPath>
    <AllowUnsafeBlocks>{(AllowUnsafe ? "true" : "false")}</AllowUnsafeBlocks>
    <Nullable>enable</Nullable>
    <EnableDynamicLoading>true</EnableDynamicLoading>
    <AppendTargetFrameworkToOutputPath>false</AppendTargetFrameworkToOutputPath>
    <CopyLocalLockFileAssemblies>true</CopyLocalLockFileAssemblies>
    {definesStr}
  </PropertyGroup>

  <ItemGroup>
    {sourcesXml}
    {refsXml}
  </ItemGroup>

</Project>";

        System.IO.Directory.CreateDirectory(Directory);
        System.IO.File.WriteAllText(ProjectPath, csproj.Trim());
    }
}
