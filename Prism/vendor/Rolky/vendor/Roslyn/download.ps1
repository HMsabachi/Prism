param(
    [string]$Version = "4.9.2"
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$tempDir = Join-Path $scriptDir "_temp_restore"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Rolky Roslyn Vendor Download" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Target version: $Version"
Write-Host "Output dir:     $scriptDir"

# --- Cleanup old temp ---
if (Test-Path $tempDir) {
    Write-Host "Cleaning up previous temp directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $tempDir
}
New-Item -ItemType Directory -Path $tempDir -Force | Out-Null

# --- Create minimal restore project ---
$csproj = @"
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net9.0</TargetFramework>
  </PropertyGroup>
  <ItemGroup>
    <PackageReference Include="Microsoft.CodeAnalysis.CSharp" Version="$Version" />
  </ItemGroup>
</Project>
"@

$csprojPath = Join-Path $tempDir "restore.csproj"
$csproj | Out-File -Encoding utf8 $csprojPath

# --- Restore ---
Write-Host "`n[1/3] Restoring NuGet packages..." -ForegroundColor Green
Push-Location $tempDir
try {
    dotnet restore --packages .\packages 2>&1 | ForEach-Object { Write-Host "  $_" }
    Write-Host "  Restore complete." -ForegroundColor Green

    # --- Extract ---
    Write-Host "`n[2/3] Extracting Roslyn DLLs..." -ForegroundColor Green
    $packagesDir = Join-Path $tempDir "packages"
    $copied = @()
    $skipped = @()

    # Priority: prefer highest TFM available
    $tfmPriority = @("net8.0", "net7.0", "net6.0", "netstandard2.0")

    # Get all top-level package directories
    $pkgRoots = Get-ChildItem -Path $packagesDir -Directory

    foreach ($pkgRoot in $pkgRoots) {
        # Each package has version subdirectories, pick the best one
        $pkgVersions = Get-ChildItem -Path $pkgRoot.FullName -Directory | Sort-Object Name -Descending
        $bestVersion = $pkgVersions | Select-Object -First 1

        if (-not $bestVersion) {
            Write-Host "  WARN: No version dir in $($pkgRoot.Name)" -ForegroundColor Yellow
            continue
        }

        $found = $false
        foreach ($tfm in $tfmPriority) {
            $libDir = Join-Path $bestVersion.FullName "lib\$tfm"
            if (Test-Path $libDir) {
                $dlls = Get-ChildItem -Path $libDir -Filter "*.dll" -ErrorAction SilentlyContinue
                foreach ($dll in $dlls) {
                    # Skip resource/satellite assemblies
                    if ($dll.Name -match "\.resources\.dll$") { continue }

                    $dest = Join-Path $scriptDir $dll.Name

                    if ((Test-Path $dest) -and ((Get-Item $dest).Length -eq $dll.Length)) {
                        $skipped += $dll.Name
                    } else {
                        Copy-Item $dll.FullName $scriptDir -Force
                        $copied += $dll.Name
                        Write-Host "  COPY: $($dll.Name) ($('{0:N0}' -f $dll.Length) bytes) [$tfm]" -ForegroundColor Gray
                    }
                    $found = $true
                }
                if ($found) { break }
            }
        }

        # Also check "analyzers/dotnet/cs" for analyzer DLLs
        if (-not $found) {
            $analyzerDir = Join-Path $bestVersion.FullName "analyzers\dotnet\cs"
            if (Test-Path $analyzerDir) {
                $dlls = Get-ChildItem -Path $analyzerDir -Filter "*.dll" -ErrorAction SilentlyContinue
                foreach ($dll in $dlls) {
                    if ($dll.Name -match "\.resources\.dll$") { continue }
                    $dest = Join-Path $scriptDir $dll.Name
                    if (-not (Test-Path $dest)) {
                        Copy-Item $dll.FullName $scriptDir -Force
                        $copied += $dll.Name
                        Write-Host "  COPY: $($dll.Name) ($('{0:N0}' -f $dll.Length) bytes) [analyzer]" -ForegroundColor DarkGray
                        $found = $true
                    }
                }
            }
        }

        if (-not $found) {
            Write-Host "  WARN: No compatible lib found for $($pkgRoot.Name) (version $($bestVersion.Name))" -ForegroundColor Yellow
            # Show what's actually inside
            $libDir = Join-Path $bestVersion.FullName "lib"
            if (Test-Path $libDir) {
                $available = (Get-ChildItem -Path $libDir -Directory | Select-Object -ExpandProperty Name) -join ", "
                Write-Host "         Available TFMs: $available" -ForegroundColor DarkYellow
            }
        }
    }

    if ($skipped.Count -gt 0) {
        Write-Host "`n  Skipped (unchanged): $($skipped -join ', ')" -ForegroundColor DarkGray
    }

    Write-Host "`n[3/3] Cleaning up..." -ForegroundColor Green
}
finally {
    Pop-Location
}

Remove-Item -Recurse -Force $tempDir -ErrorAction SilentlyContinue

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host " Done! $($copied.Count) DLL(s) copied/updated" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# --- Summary ---
Write-Host "`nVendored DLLs:" -ForegroundColor Green
$totalBytes = 0
Get-ChildItem -Path $scriptDir -Filter "*.dll" | Sort-Object Name | ForEach-Object {
    $sizeKB = [math]::Round($_.Length / 1KB, 1)
    $totalBytes += $_.Length
    Write-Host "  $($_.Name.PadRight(50)) $sizeKB KB"
}

$totalKB = [math]::Round($totalBytes / 1KB, 1)
Write-Host ("  {0}Total: $totalKB KB" -f ('-' * 50)) -ForegroundColor Cyan
