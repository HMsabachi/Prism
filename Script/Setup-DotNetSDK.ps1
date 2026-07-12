param(
    [int]$RequiredMajorVersion = 9,
    [switch]$InstallIfMissing = $true,
    [switch]$Quiet = $false
)

$ErrorActionPreference = "Stop"

function Write-Step { param([string]$Msg) Write-Host "[$((Get-Date).ToString('HH:mm:ss'))] $Msg" -ForegroundColor Gray }
function Write-Good { param([string]$Msg) Write-Host "[OK] $Msg" -ForegroundColor Green }
function Write-Bad  { param([string]$Msg) Write-Host "[XX] $Msg" -ForegroundColor Red }
function Write-Warn { param([string]$Msg) Write-Host "[!!] $Msg" -ForegroundColor Yellow }

# Step 1: Find dotnet CLI
function Test-DotNetExe {
    Write-Step "Searching for dotnet CLI..."
    $knownPaths = @(
        "$env:ProgramFiles\dotnet\dotnet.exe",
        "${env:ProgramFiles(x86)}\dotnet\dotnet.exe"
    )
    foreach ($p in $knownPaths) {
        if (Test-Path $p) {
            Write-Good "Found at: $p"
            return $p
        }
    }
    $pathDirs = $env:PATH -split [IO.Path]::PathSeparator
    foreach ($dir in $pathDirs) {
        $candidate = Join-Path $dir "dotnet.exe"
        if (Test-Path $candidate) {
            Write-Good "Found on PATH: $candidate"
            return $candidate
        }
    }
    Write-Bad "dotnet CLI not found."
    return $null
}

function Test-DotNetSdk {
    param([string]$DotNetExe, [int]$MajorVersion)
    Write-Step "Checking .NET SDK (major version $MajorVersion)..."
    if (-not $DotNetExe) {
        return @{ Version = $null; Path = $null; Found = $false }
    }
    try {
        $env:DOTNET_CLI_UI_LANGUAGE = "en-US"
        $output = & $DotNetExe --list-sdks 2>&1
        $env:DOTNET_CLI_UI_LANGUAGE = $null
        if ($LASTEXITCODE -ne 0) {
            Write-Bad "'dotnet --list-sdks' failed (exit code $LASTEXITCODE)"
            return @{ Version = $null; Path = $null; Found = $false }
        }
    }
    catch {
        Write-Bad "Failed to run 'dotnet --list-sdks': $_"
        return @{ Version = $null; Path = $null; Found = $false }
    }

    $bestVersion = $null
    $bestPath = $null
    foreach ($line in $output) {
        if ($line -match '^(\d+\.\d+\.\d+)\s+\[(.+)\]') {
            $ver = [Version]$matches[1]
            $sdkPathBase = $matches[2].TrimEnd('\')
            $sdkFullPath = Join-Path $sdkPathBase $ver.ToString()
            if ($ver.Major -eq $MajorVersion) {
                if ((-not $bestVersion) -or ($ver -gt $bestVersion)) {
                    $bestVersion = $ver
                    $bestPath = $sdkFullPath
                }
            }
        }
    }

    $allSdks = ($output | Where-Object { $_ -match '^\d' }) -join ', '
    Write-Step "Installed SDKs: $allSdks"

    if ($bestVersion) {
        Write-Good "Found SDK $bestVersion at: $bestPath"
    }
    else {
        Write-Bad "No .NET $MajorVersion SDK found."
    }
    return @{ Version = $bestVersion; Path = $bestPath; Found = ($bestVersion -ne $null) }
}

function Install-DotNetSdk {
    param([int]$MajorVersion)
    Write-Step "Attempting automatic installation of .NET SDK $MajorVersion..."

    $winget = Get-Command winget.exe -ErrorAction SilentlyContinue
    if ($winget) {
        Write-Step "Found winget, installing via package manager..."
        $pkgId = "Microsoft.DotNet.SDK.$MajorVersion"
        try {
            $result = & winget install $pkgId --accept-package-agreements --accept-source-agreements --silent 2>&1
            foreach ($l in $result) { Write-Host "    $l" }
            if ($LASTEXITCODE -eq 0) {
                Write-Good "winget installation completed."
                $env:PATH = [Environment]::GetEnvironmentVariable("PATH", "Machine") + ";" + [Environment]::GetEnvironmentVariable("PATH", "User")
                return $true
            }
            Write-Warn "winget returned exit code $LASTEXITCODE, falling back..."
        }
        catch { Write-Warn "winget failed: $_" }
    }
    else {
        Write-Step "winget not found on this system."
    }

    Write-Step "Downloading dotnet-install.ps1..."
    $installScript = Join-Path $env:TEMP "dotnet-install.ps1"
    try {
        Invoke-WebRequest -Uri "https://dot.net/v1/dotnet-install.ps1" -OutFile $installScript
        Write-Step "Running dotnet-install.ps1 -Channel $MajorVersion.0 ..."
        & $installScript -Channel "$MajorVersion.0" -InstallDir "$env:ProgramFiles\dotnet"
        if ($LASTEXITCODE -eq 0) {
            Write-Good ".NET SDK $MajorVersion installed successfully."
            $env:PATH = "$env:ProgramFiles\dotnet;$env:PATH"
            return $true
        }
        Write-Bad "dotnet-install.ps1 returned exit code $LASTEXITCODE"
    }
    catch { Write-Bad "Download/install failed: $_" }
    finally {
        if (Test-Path $installScript) { Remove-Item $installScript -Force -ErrorAction SilentlyContinue }
    }

    Write-Host ""
    Write-Host "  Please install manually:" -ForegroundColor Yellow
    Write-Host "  https://dotnet.microsoft.com/en-us/download/dotnet/$MajorVersion.0" -ForegroundColor Cyan
    Write-Host ""
    return $false
}

# Main
function Main {
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host " Prism - .NET SDK Environment Check" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "Required: .NET SDK $RequiredMajorVersion.0" -ForegroundColor Gray
    if (-not $Quiet) { Write-Host "" }

    $dotnetExe = Test-DotNetExe
    if ($dotnetExe) {
        $sdkInfo = Test-DotNetSdk -DotNetExe $dotnetExe -MajorVersion $RequiredMajorVersion
        if ($sdkInfo.Found) {
            if (-not $Quiet) { Write-Host "" }
            Write-Host "========================================" -ForegroundColor Cyan
            Write-Good ".NET SDK environment is ready."
            Write-Host "  Version : $($sdkInfo.Version)" -ForegroundColor Gray
            Write-Host "  Path    : $($sdkInfo.Path)" -ForegroundColor Gray
            Write-Host "========================================" -ForegroundColor Cyan
            return @{ Success = $true; DotNetExe = $dotnetExe; SdkVersion = $sdkInfo.Version; SdkPath = $sdkInfo.Path; ActionTaken = "none" }
        }
    }

    if ($InstallIfMissing) {
        Write-Host ""
        Write-Warn ".NET SDK $RequiredMajorVersion not found. Starting auto-install..."
        Write-Host ""

        if (Install-DotNetSdk -MajorVersion $RequiredMajorVersion) {
            Write-Host ""
            Write-Step "Refreshing environment..."
            $dotnetExe = Test-DotNetExe
            if ($dotnetExe) {
                $sdkInfo = Test-DotNetSdk -DotNetExe $dotnetExe -MajorVersion $RequiredMajorVersion
                if ($sdkInfo.Found) {
                    Write-Host ""
                    Write-Host "========================================" -ForegroundColor Cyan
                    Write-Good ".NET SDK installed and ready."
                    Write-Host "  Version : $($sdkInfo.Version)" -ForegroundColor Gray
                    Write-Host "========================================" -ForegroundColor Cyan
                    return @{ Success = $true; DotNetExe = $dotnetExe; SdkVersion = $sdkInfo.Version; SdkPath = $sdkInfo.Path; ActionTaken = "installed" }
                }
            }
        }
    }

    Write-Host ""
    Write-Host "========================================" -ForegroundColor Red
    Write-Bad "Unable to set up .NET SDK environment."
    Write-Host ""
    Write-Host "  Install manually:" -ForegroundColor Yellow
    Write-Host "  https://dotnet.microsoft.com/en-us/download/dotnet/$RequiredMajorVersion.0" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Red
    return @{ Success = $false; DotNetExe = $null; SdkVersion = $null; SdkPath = $null; ActionTaken = "failed" }
}

$result = Main
if ($result.Success) { exit 0 } else { exit 1 }
