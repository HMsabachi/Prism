using System;
using System.Diagnostics;
using System.IO;
using System.Threading.Tasks;

namespace Rolky.Managed.Build;

public static class BuildSystem
{
    public static int Build(BuildConfig config, Action<string?>? stdOutHandler = null, Action<string?>? stdErrorHandler = null)
    {
        using var process = LaunchBuild(config, stdOutHandler, stdErrorHandler);
        if (process is null) return -1;
        process.WaitForExit();
        return process.ExitCode;
    }

    public static async Task<int> BuildAsync(BuildConfig config, Action<string?>? stdOutHandler = null, Action<string?>? stdErrorHandler = null)
    {
        using var process = LaunchBuild(config, stdOutHandler, stdErrorHandler);
        if (process is null) return -1;
        await process.WaitForExitAsync();
        return process.ExitCode;
    }

    private static Process? LaunchBuild(BuildConfig config, Action<string?>? stdOutHandler, Action<string?>? stdErrorHandler)
    {
        var dotnetExe = config.DotNetExePath ?? DotNetLocator.FindDotNetExe();
        if (string.IsNullOrEmpty(dotnetExe) || !File.Exists(dotnetExe))
            throw new FileNotFoundException("dotnet CLI not found. Install .NET SDK or set DotNetExePath.");

        var startInfo = new ProcessStartInfo
        {
            FileName = dotnetExe,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = true,
        };

        BuildArguments(config, startInfo);

        startInfo.Environment["DOTNET_CLI_UI_LANGUAGE"] = "en-US";

        stdOutHandler?.Invoke($"dotnet {string.Join(" ", startInfo.ArgumentList)}");

        var process = new Process { StartInfo = startInfo };

        process.OutputDataReceived += (_, e) => stdOutHandler?.Invoke(e.Data);
        process.ErrorDataReceived += (_, e) => stdErrorHandler?.Invoke(e.Data);

        process.Start();
        process.BeginOutputReadLine();
        process.BeginErrorReadLine();

        return process;
    }

    private static void BuildArguments(BuildConfig config, ProcessStartInfo startInfo)
    {
        var args = startInfo.ArgumentList;
        args.Add(config.OnlyClean ? "clean" : "build");
        args.Add(config.ProjectPath);
        args.Add("-c");
        args.Add(config.Configuration);

        if (!config.Restore) args.Add("--no-restore");
        if (config.Rebuild) args.Add("--no-incremental");

        args.Add("-v");
        args.Add("normal");

        foreach (var prop in config.CustomProperties)
            args.Add($"-p:{prop}");
    }
}
