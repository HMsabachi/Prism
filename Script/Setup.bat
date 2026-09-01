@echo off
REM Prism - one-shot environment bootstrap (Windows)
REM Chains: Python (build/tooling scripts) -> .NET SDK 9 -> Vulkan SDK -> shader compiler deps (glslang + SPIRV-Cross).
REM Does NOT touch vendor/Python (the engine's embedded CPython interpreter).
setlocal

echo ========================================
echo  Prism - Environment Setup
echo ========================================
echo.

echo [1/4] Python
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Setup-Python.ps1"
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [XX] Python setup failed. Aborting.
    pause
    exit /b 1
)
echo.

echo [2/4] .NET SDK
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Setup-DotNetSDK.ps1"
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [XX] .NET SDK setup failed. Aborting.
    pause
    exit /b 1
)
echo.

echo [3/4] Vulkan SDK
REM Vulkan script needs these pip packages; install up front (idempotent - pip skips what's already present).
python -m pip install --disable-pip-version-check requests colorama fake-useragent
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [XX] Failed to install Vulkan script dependencies: requests, colorama, fake-useragent.
    pause
    exit /b 1
)
echo.
REM Vulkan script drives its own install prompt and may sys.exit(0) after launching the installer.
REM Re-run this script after the installer finishes so VULKAN_SDK env var is re-read.
python "%~dp0Setup-VulkanSDK.py"
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [!!] Vulkan SDK not ready. Complete the installer / set VULKAN_SDK and re-run this script.
    echo.
    echo ========================================
    echo  Python and .NET SDK are ready.
    echo  Vulkan SDK still pending - see messages above.
    echo ========================================
    pause
    exit /b 1
)

echo.
echo [4/4] Shader compiler dependencies
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [XX] CMake not found in PATH. Install CMake and re-run this script.
    pause
    exit /b 1
)
call "%~dp0..\Prism\vendor\PrismShaderCompiler\BuildDeps.bat"
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [XX] Shader compiler dependencies failed. See messages above.
    pause
    exit /b 1
)

echo.
echo ========================================
echo  Prism environment ready.
echo  - Run Win-GenerateProjects.bat to generate the VS solution.
echo ========================================
pause
exit /b 0
