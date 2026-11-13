# Build MinecraftPE targeting Windows XP SP2/SP3.
# Requires: VS 2017 Build Tools with v141_xp toolset, CMake 3.21+, Ninja, Git.

param(
    [switch]$Clean,
    [string]$BuildDir = (Join-Path $PSScriptRoot "build-xp"),
    [string]$VsRoot = "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools",
    [string]$NinjaDir = "C:\Users\Jack\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe",
    [string]$CMake = "C:\Program Files\CMake\bin\cmake.exe"
)

$ErrorActionPreference = 'Stop'

if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "Cleaning $BuildDir"
    Remove-Item -Recurse -Force $BuildDir
}

$vcvars = Join-Path $VsRoot "VC\Auxiliary\Build\vcvars32.bat"
if (-not (Test-Path $vcvars)) { throw "vcvars32.bat not found at $vcvars" }

# Capture the v141 environment from vcvars32 into the current process.
$envLines = cmd.exe /c "`"$vcvars`" -vcvars_ver=14.16 1>nul 2>nul && set" 2>$null
$captured = @{}
foreach ($line in $envLines) { if ($line -match '^([^=]+)=(.*)$') { $captured[$matches[1]] = $matches[2] } }
$env:INCLUDE = $captured['INCLUDE']
$env:LIB     = $captured['LIB']
$env:LIBPATH = $captured['LIBPATH']
# Strip Watcom (or any other stray cl.exe) so MSVC wins on PATH.
$cleanPath = ($captured['PATH'] -split ';' | Where-Object { $_ -notmatch '(?i)WATCOM' }) -join ';'
$env:PATH = "$NinjaDir;$cleanPath"
# OpenAL 1.19.1's native-tools sub-project uses cmake_minimum_required < 3.5; CMake 4.x
# refuses to configure that without this hint.
$env:CMAKE_POLICY_VERSION_MINIMUM = "3.5"

Write-Host "cl.exe : $((Get-Command cl.exe).Source)"
Write-Host "ninja  : $((Get-Command ninja).Source)"
Write-Host "cmake  : $CMake"

& $CMake -S $PSScriptRoot -B $BuildDir -G Ninja `
    -DCMAKE_BUILD_TYPE=Release `
    -DXP_TARGET=ON `
    -DCMAKE_CXX_FLAGS_RELEASE="/O1 /Os /GS- /sdl- /MT /GL /Gy" `
    -DCMAKE_C_FLAGS_RELEASE="/O1 /Os /GS- /sdl- /MT /GL /Gy" `
    -DCMAKE_EXE_LINKER_FLAGS_RELEASE="/OPT:REF /OPT:ICF /LTCG" `
    -DCMAKE_SHARED_LINKER_FLAGS_RELEASE="/OPT:REF /OPT:ICF /LTCG"

if ($LASTEXITCODE -ne 0) { throw "Configure failed" }

& $CMake --build $BuildDir --config Release
if ($LASTEXITCODE -ne 0) { throw "Build failed" }

Write-Host ""
Write-Host "Output: $BuildDir\MinecraftPE.exe"