using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Rolky.Managed.Build;

public static class BuildManager
{
    public const string IssuesFileName = "msbuild_issues.csv";
    public const string LogFileName = "msbuild_log.txt";

    private static BuildConfig? s_BuildInProgress;

    public static event Action<BuildConfig, string>? BuildLaunchFailed;
    public static event Action<BuildConfig>? BuildStarted;
    public static event Action<BuildResult>? BuildFinished;
    public static event Action<string?>? StdOutputReceived;
    public static event Action<string?>? StdErrorReceived;

    public static bool BuildProjectBlocking(string configuration = "Debug", string? platform = null, bool rebuild = false)
    {
        return Build(CreateConfig(configuration, platform, rebuild));
    }

    public static bool CleanProjectBlocking(string configuration = "Debug")
    {
        return Build(CreateConfig(configuration, onlyClean: true));
    }

    public static async Task<bool> BuildProjectAsync(string configuration = "Debug")
    {
        return await BuildAsync(CreateConfig(configuration));
    }

    public static List<BuildDiagnostic> ReadDiagnostics(BuildConfig config)
    {
        var list = new List<BuildDiagnostic>();
        var csvPath = Path.Combine(config.LogsDirPath, IssuesFileName);
        if (!File.Exists(csvPath)) return list;

        foreach (var line in File.ReadAllLines(csvPath).Skip(1))
        {
            var parts = ParseCsvLine(line);
            if (parts.Length < 7) continue;

            list.Add(new BuildDiagnostic
            {
                Type = Enum.TryParse<BuildDiagnostic.DiagnosticType>(parts[0], true, out var t) ? t : BuildDiagnostic.DiagnosticType.Error,
                File = string.IsNullOrEmpty(parts[1]) ? null : parts[1],
                Line = int.TryParse(parts[2], out var ln) ? ln : 0,
                Column = int.TryParse(parts[3], out var col) ? col : 0,
                Code = string.IsNullOrEmpty(parts[4]) ? null : parts[4],
                Message = parts[5],
                ProjectFile = string.IsNullOrEmpty(parts[6]) ? null : parts[6],
            });
        }

        return list;
    }

    public static List<BuildDiagnostic> ParseDiagnosticsFromOutput(string output)
    {
        var list = new List<BuildDiagnostic>();
        foreach (var line in output.Split('\n'))
        {
            var trimmed = line.Trim();
            var diag = ParseDiagnosticLine(trimmed);
            if (diag is not null) list.Add(diag);
        }
        return list;
    }

    private static BuildDiagnostic? ParseDiagnosticLine(string line)
    {
        var match = Regex.Match(
            line, @"^(.+?)\((\d+),(\d+)\):\s*(error|warning|info)\s+(\w+):\s+(.+?)\s*\[(.+?)\]$");

        if (!match.Success) return null;

        return new BuildDiagnostic
        {
            File = match.Groups[1].Value,
            Line = int.Parse(match.Groups[2].Value),
            Column = int.Parse(match.Groups[3].Value),
            Type = match.Groups[4].Value switch
            {
                "error" => BuildDiagnostic.DiagnosticType.Error,
                "warning" => BuildDiagnostic.DiagnosticType.Warning,
                _ => BuildDiagnostic.DiagnosticType.Info,
            },
            Code = match.Groups[5].Value,
            Message = match.Groups[6].Value,
            ProjectFile = match.Groups[7].Value,
        };
    }

    private static BuildConfig CreateConfig(string configuration, string? platform = null, bool rebuild = false, bool onlyClean = false)
    {
        return new BuildConfig
        {
            SolutionPath = ScriptBuildSettings.SolutionPath,
            ProjectPath = ScriptBuildSettings.ProjectPath ?? ScriptBuildSettings.SolutionPath,
            Configuration = configuration,
            Platform = platform,
            Restore = true,
            Rebuild = rebuild,
            OnlyClean = onlyClean,
            LogsDirPath = ScriptBuildSettings.LogsDirectory,
        };
    }

    private static bool Build(BuildConfig config)
    {
        if (s_BuildInProgress is not null)
            throw new InvalidOperationException("A build is already in progress.");

        s_BuildInProgress = config;
        try
        {
            RemoveOldIssuesFile(config);
            BuildStarted?.Invoke(config);

            var logPath = Path.Combine(config.LogsDirPath, LogFileName);
            Directory.CreateDirectory(config.LogsDirPath);

            var logWriter = new StreamWriter(logPath, false);
            Action<string?> logHandler = line =>
            {
                if (line is not null) logWriter.WriteLine(line);
                StdOutputReceived?.Invoke(line);
            };

            int exitCode = BuildSystem.Build(config, logHandler, StdErrorReceived);
            logWriter.Dispose();

            if (exitCode != 0)
            {
                BuildFinished?.Invoke(BuildResult.Error);
                return false;
            }

            BuildFinished?.Invoke(BuildResult.Success);
            return true;
        }
        catch (FileNotFoundException ex)
        {
            BuildLaunchFailed?.Invoke(config, ex.Message);
            BuildFinished?.Invoke(BuildResult.Error);
            return false;
        }
        catch (Exception ex)
        {
            BuildLaunchFailed?.Invoke(config, ex.ToString());
            BuildFinished?.Invoke(BuildResult.Error);
            return false;
        }
        finally
        {
            s_BuildInProgress = null;
        }
    }

    private static async Task<bool> BuildAsync(BuildConfig config)
    {
        if (s_BuildInProgress is not null)
            throw new InvalidOperationException("A build is already in progress.");

        s_BuildInProgress = config;
        try
        {
            RemoveOldIssuesFile(config);
            BuildStarted?.Invoke(config);

            var logPath = Path.Combine(config.LogsDirPath, LogFileName);
            Directory.CreateDirectory(config.LogsDirPath);

            var logWriter = new StreamWriter(logPath, false);
            Action<string?> logHandler = line =>
            {
                if (line is not null) logWriter.WriteLine(line);
                StdOutputReceived?.Invoke(line);
            };

            int exitCode = await BuildSystem.BuildAsync(config, logHandler, StdErrorReceived);
            logWriter.Dispose();

            BuildFinished?.Invoke(exitCode == 0 ? BuildResult.Success : BuildResult.Error);
            return exitCode == 0;
        }
        finally
        {
            s_BuildInProgress = null;
        }
    }

    private static void RemoveOldIssuesFile(BuildConfig config)
    {
        try
        {
            var path = Path.Combine(config.LogsDirPath, IssuesFileName);
            if (File.Exists(path)) File.Delete(path);
        }
        catch (IOException) { }
    }

    private static string[] ParseCsvLine(string line)
    {
        var result = new List<string>();
        bool inQuotes = false;
        int start = 0;

        for (int i = 0; i < line.Length; i++)
        {
            if (line[i] == '"') inQuotes = !inQuotes;
            else if (line[i] == ',' && !inQuotes)
            {
                result.Add(line[start..i].Trim('"'));
                start = i + 1;
            }
        }
        result.Add(line[start..].Trim('"'));
        return result.ToArray();
    }
}
