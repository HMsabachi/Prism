param(
    [int]$MajorVersion = 3,
    [int]$MinorVersion = 13,
    [switch]$InstallIfMissing = $true,
    [switch]$Quiet = $false
)

$ErrorActionPreference = "Stop"

function Write-Step { param([string]$Msg) Write-Host "[$((Get-Date).ToString('HH:mm:ss'))] $Msg" -ForegroundColor Gray }
function Write-Good { param([string]$Msg) Write-Host "[OK] $Msg" -ForegroundColor Green }
function Write-Bad  { param([string]$Msg) Write-Host "[XX] $Msg" -ForegroundColor Red }
function Write-Warn { param([string]$Msg) Write-Host "[!!] $Msg" -ForegroundColor Yellow }

# Step 1: Locate python executable. py launcher first, then python/python3 on PATH.
function Find-PythonExe {
    Write-Step "Searching for Python..."
    $candidates = @()

    $pyLauncher = Get-Command py.exe -ErrorAction SilentlyContinue
    if ($pyLauncher) { $candidates += $pyLauncher.Source }

    foreach ($name in @("python.exe", "python3.exe")) {
        $found = Get-Command $name -ErrorAction SilentlyContinue
        if ($found) { $candidates += $found.Source }
    }

    $pyDirTag = "{0}{1}" -f $MajorVersion, $MinorVersion
    $knownPaths = @(
        "$env:LOCALAPPDATA\Programs\Python\Python$pyDirTag\python.exe",
        "$env:ProgramFiles\Python$pyDirTag\python.exe",
        "${env:ProgramFiles(x86)}\Python$pyDirTag\python.exe"
    )
    foreach ($p in $knownPaths) {
        if (Test-Path $p) { $candidates += $p }
    }

    $candidates = $candidates | Select-Object -Unique
    foreach ($exe in $candidates) {
        Write-Step "Probing: $exe"
        $info = Test-PythonExe -ExePath $exe -MajorVersion $MajorVersion -MinorVersion $MinorVersion
        if ($info.Found) {
            Write-Good "Found Python $($info.Version) at: $exe"
            return @{ ExePath = $exe; Version = $info.Version; Found = $true }
        }
    }

    Write-Bad "No suitable Python $MajorVersion.$MinorVersion found."
    return @{ ExePath = $null; Version = $null; Found = $false }
}

# Step 2: Query a python executable for its version. Handles the py launcher
# (which needs -<major>.<minor> to pick a specific interpreter).
function Test-PythonExe {
    param([string]$ExePath, [int]$MajorVersion, [int]$MinorVersion)
    if (-not $ExePath) {
        return @{ Version = $null; Found = $false }
    }

    $isLauncher = (Split-Path $ExePath -Leaf) -ieq "py.exe"
    $versionArg = if ($isLauncher) { @("-$MajorVersion.$MinorVersion") } else { @() }

    try {
        $output = & $ExePath @versionArg -c "import sys; print(sys.version)" 2>&1
        if ($LASTEXITCODE -ne 0) {
            return @{ Version = $null; Found = $false }
        }
    }
    catch {
        return @{ Version = $null; Found = $false }
    }

    $ver = [Version]$output.ToString().Split(' ')[0]
    if ($ver.Major -ne $MajorVersion -or $ver.Minor -ne $MinorVersion) {
        Write-Step "  -> $ver (need $MajorVersion.$MinorVersion)"
        return @{ Version = $ver; Found = $false }
    }
    return @{ Version = $ver; Found = $true }
}

# Step 3: Auto-install. Prefer winget; fall back to the python.org installer.
function Install-Python {
    param([int]$MajorVersion, [int]$MinorVersion)
    Write-Step "Attempting automatic installation of Python $MajorVersion.$MinorVersion..."

    $winget = Get-Command winget.exe -ErrorAction SilentlyContinue
    if ($winget) {
        Write-Step "Found winget, installing via package manager..."
        $pkgId = "Python.Python.$MajorVersion.$MinorVersion"
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

    Write-Step "Downloading python.org installer..."
    $installerDir = Join-Path $env:TEMP "PrismPythonInstall"
    New-Item -ItemType Directory -Force -Path $installerDir | Out-Null

    $release = (Invoke-RestMethod "https://www.python.org/ftp/python/").ToString()
    $versionMatches = [regex]::Matches($release, '\d+\.\d+\.\d+/')
    $candidates = $versionMatches | ForEach-Object { [Version]$_.Value.TrimEnd('/') } | Sort-Object -Descending
    $latest = $candidates | Where-Object { $_.Major -eq $MajorVersion -and $_.Minor -eq $MinorVersion } | Select-Object -First 1
    if (-not $latest) {
        Write-Bad "Could not find a Python $MajorVersion.$MinorVersion release on python.org."
        return $false
    }

    $verStr = $latest.ToString()
    $installerName = "python-$verStr-amd64.exe"
    $installerUrl = "https://www.python.org/ftp/python/$verStr/$installerName"
    $installerPath = Join-Path $installerDir $installerName

    Write-Step "Downloading $installerUrl"
    try {
        Invoke-WebRequest -Uri $installerUrl -OutFile $installerPath
    }
    catch {
        Write-Bad "Download failed: $_"
        return $false
    }

    Write-Step "Running installer (InstallAllUsers=0, PrependPath=1)..."
    try {
        $proc = Start-Process -FilePath $installerPath -ArgumentList "/quiet", "InstallAllUsers=0", "PrependPath=1", "Include_test=0" -Wait -PassThru
        if ($proc.ExitCode -eq 0) {
            Write-Good "Python $verStr installed."
            # python.org user install: %LOCALAPPDATA%\Programs\Python\Python<MM>
            $pyDirTag = "{0}{1}" -f $MajorVersion, $MinorVersion
            $pyDir = "{0}\Programs\Python\Python{1}" -f $env:LOCALAPPDATA, $pyDirTag
            $env:PATH = "$pyDir;$pyDir\Scripts;$env:PATH"
            return $true
        }
        Write-Bad "Installer exited with code $($proc.ExitCode)"
    }
    catch { Write-Bad "Installer failed: $_" }

    Write-Host ""
    Write-Host "  Please install manually:" -ForegroundColor Yellow
    Write-Host "  https://www.python.org/downloads/" -ForegroundColor Cyan
    Write-Host "  (Check 'Add Python to PATH' during install)" -ForegroundColor Yellow
    Write-Host ""
    return $false
}

# Main
function Main {
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host " Prism - Python Environment Check" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "Required: Python $MajorVersion.$MinorVersion (to run build/tooling scripts)" -ForegroundColor Gray
    if (-not $Quiet) { Write-Host "" }

    $py = Find-PythonExe
    if ($py.Found) {
        if (-not $Quiet) { Write-Host "" }
        Write-Host "========================================" -ForegroundColor Cyan
        Write-Good "Python environment is ready."
        Write-Host "  Version : $($py.Version)" -ForegroundColor Gray
        Write-Host "  Path    : $($py.ExePath)" -ForegroundColor Gray
        Write-Host "========================================" -ForegroundColor Cyan
        return @{ Success = $true; ExePath = $py.ExePath; Version = $py.Version; ActionTaken = "none" }
    }

    if ($InstallIfMissing) {
        Write-Host ""
        Write-Warn "Python $MajorVersion.$MinorVersion not found. Starting auto-install..."
        Write-Host ""

        if (Install-Python -MajorVersion $MajorVersion -MinorVersion $MinorVersion) {
            Write-Host ""
            Write-Step "Refreshing environment..."
            $py = Find-PythonExe
            if ($py.Found) {
                Write-Host ""
                Write-Host "========================================" -ForegroundColor Cyan
                Write-Good "Python installed and ready."
                Write-Host "  Version : $($py.Version)" -ForegroundColor Gray
                Write-Host "  Path    : $($py.ExePath)" -ForegroundColor Gray
                Write-Host "========================================" -ForegroundColor Cyan
                return @{ Success = $true; ExePath = $py.ExePath; Version = $py.Version; ActionTaken = "installed" }
            }
        }
    }

    Write-Host ""
    Write-Host "========================================" -ForegroundColor Red
    Write-Bad "Unable to set up Python environment."
    Write-Host ""
    Write-Host "  Install manually:" -ForegroundColor Yellow
    Write-Host "  https://www.python.org/downloads/" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Red
    return @{ Success = $false; ExePath = $null; Version = $null; ActionTaken = "failed" }
}

$result = Main
if ($result.Success) { exit 0 } else { exit 1 }
