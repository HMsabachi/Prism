using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;

namespace Rolky.Managed.Build;

public static class DotNetLocator
{
    private static string? s_CachedDotNetExe;

    public static string? FindDotNetExe()
    {
        if (s_CachedDotNetExe is not null)
            return s_CachedDotNetExe;

        if (!RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            return null;

        var knownPaths = new[]
        {
            Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles), "dotnet", "dotnet.exe"),
            Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86), "dotnet", "dotnet.exe"),
        };

        foreach (var p in knownPaths)
        {
            if (File.Exists(p))
            {
                s_CachedDotNetExe = p;
                return p;
            }
        }

        var pathDirs = (Environment.GetEnvironmentVariable("PATH") ?? "").Split(Path.PathSeparator);
        foreach (var dir in pathDirs)
        {
            var candidate = Path.Combine(dir, "dotnet.exe");
            if (File.Exists(candidate))
            {
                s_CachedDotNetExe = candidate;
                return candidate;
            }
        }

        return null;
    }

    public static Version? FindSdk(int majorVersion)
    {
        var dotnetExe = FindDotNetExe();
        if (dotnetExe is null) return null;

        try
        {
            var startInfo = new ProcessStartInfo
            {
                FileName = dotnetExe,
                Arguments = "--list-sdks",
                RedirectStandardOutput = true,
                UseShellExecute = false,
                CreateNoWindow = true,
            };
            startInfo.Environment["DOTNET_CLI_UI_LANGUAGE"] = "en-US";

            using var process = Process.Start(startInfo);
            if (process is null) return null;

            var output = process.StandardOutput.ReadToEnd();
            process.WaitForExit();

            Version? best = null;
            foreach (var line in output.Split('\n'))
            {
                var trimmed = line.Trim();
                var spaceIdx = trimmed.IndexOf(' ');
                if (spaceIdx <= 0) continue;

                if (Version.TryParse(trimmed[..spaceIdx], out var ver) && ver.Major == majorVersion)
                {
                    if (best is null || ver > best) best = ver;
                }
            }

            return best;
        }
        catch
        {
            return null;
        }
    }
}
