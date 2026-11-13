# Build MinecraftPE for modern 32-bit Windows (VS 2017+ default toolset, dynamic CRT).
# Requires: VS 2017/2019/2022 Build Tools (x86 toolset), CMake 3.21+, Ninja (on PATH), Git.

param(
    [switch]$Clean,
    [string]$BuildDir = (Join-Path $PSScriptRoot "build-win32"),
    [string]$VsRoot = "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools",
    [string]$CMake = "C:\Program Files\CMake\bin\cmake.exe"
)

$ErrorActionPreference = 'Stop'

if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "Cleaning $BuildDir"
    Remove-Item -Recurse -Force $BuildDir
}

$vcvars = Join-Path $VsRoot "VC\Auxiliary\Build\vcvars32.bat"
if (-not (Test-Path $vcvars)) { throw "vcvars32.bat not found at $vcvars" }

# Capture the default (modern, x86) toolset environment.
$envLines = cmd.exe /c "`"$vcvars`" 1>nul 2>nul && set" 2>$null
$captured = @{}
foreach ($line in $envLines) { if ($line -match '^([^=]+)=(.*)$') { $captured[$matches[1]] = $matches[2] } }
$env:INCLUDE = $captured['INCLUDE']
$env:LIB     = $captured['LIB']
$env:LIBPATH = $captured['LIBPATH']
$env:PATH = ($captured['PATH'] -split ';' | Where-Object { $_ -notmatch '(?i)WATCOM' }) -join ';'

Write-Host "cl.exe : $((Get-Command cl.exe).Source)"
Write-Host "ninja  : $((Get-Command ninja).Source)"
Write-Host "cmake  : $CMake"

& $CMake -S $PSScriptRoot -B $BuildDir -G Ninja `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_CXX_FLAGS_RELEASE="/O2 /GS- /sdl- /MD /GL /Gy" `
    -DCMAKE_C_FLAGS_RELEASE="/O2 /GS- /sdl- /MD /GL /Gy" `
    -DCMAKE_EXE_LINKER_FLAGS_RELEASE="/OPT:REF /OPT:ICF /LTCG" `
    -DCMAKE_SHARED_LINKER_FLAGS_RELEASE="/OPT:REF /OPT:ICF /LTCG"

if ($LASTEXITCODE -ne 0) { throw "Configure failed" }

& $CMake --build $BuildDir --config Release
if ($LASTEXITCODE -ne 0) { throw "Build failed" }

Write-Host ""
Write-Host "Output: $BuildDir\MinecraftPE.exe"