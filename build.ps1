# .-=#####=-.
# PSE SDK Lua
# |- Creator: ntcd_lol, opencode
# \- Comment: :3
# '-=#####=-'
# ^         ^
# Canonical Windows build (PowerShell 5.1+). Stages sources into an ASCII
# temp dir %TEMP%\pse_build (avoids spaces / non-ASCII project paths), then
# compiles bundled Lua 5.4.8 + the C++ host via MSVC (vcvars64) and copies
# the binary to <repo>\bin\pse_lua.exe.
# Run:  powershell -ExecutionPolicy Bypass -File build.ps1
# --==-==--

$ErrorActionPreference = "Stop"

$Root = $PSScriptRoot
$Stage = Join-Path $env:TEMP "pse_build"
$Out   = Join-Path $Root "bin"
New-Item -ItemType Directory -Force -Path $Out | Out-Null

# --- stage sources -----------------------------------------------------------
if (Test-Path -LiteralPath $Stage) { Remove-Item -LiteralPath $Stage -Recurse -Force }
New-Item -ItemType Directory -Force -Path $Stage | Out-Null
Copy-Item -LiteralPath (Join-Path $Root "src") -Destination $Stage -Recurse -Force
Copy-Item -LiteralPath (Join-Path $Root "third_party\lua-5.4.8") -Destination (Join-Path $Stage "lua") -Recurse -Force

$Src = Join-Path $Stage "src"
$Lua = Join-Path $Stage "lua"

# --- MSVC environment --------------------------------------------------------
$VCVARS = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path -LiteralPath $VCVARS)) {
    Write-Error "vcvars64.bat not found. Install MSVC Build Tools 2022 (Desktop development with C++)."
}
$envLines = & cmd /c "`"$VCVARS`" >nul 2>&1 && set"
if ($LASTEXITCODE -ne 0) { Write-Error "vcvars64.bat failed." }
foreach ($line in $envLines) {
    if ($line -match "^([^=]+)=(.*)$") {
        [Environment]::SetEnvironmentVariable($matches[1], $matches[2], "Process")
    }
}

$luaSources = Get-ChildItem -Path $Lua -Filter *.c -File |
    Where-Object { $_.BaseName -notin @("lua", "luac") }

$cmdArgs = @()
$cmdArgs += "/nologo", "/utf-8", "/W3", "/O2", "/EHsc", "/std:c++17", "/DNDEBUG"
$cmdArgs += "/I", $Lua
$cmdArgs += "/I", $Src
$cmdArgs += (Join-Path $Src "main.cpp")
$cmdArgs += (Join-Path $Src "lua_core.cpp")
$cmdArgs += (Join-Path $Src "pse_bridge.cpp")
foreach ($s in $luaSources) { $cmdArgs += $s.FullName }
$cmdArgs += "/Fe" + (Join-Path $Stage "pse_lua.exe")
$cmdArgs += "/link", "/SUBSYSTEM:CONSOLE"

Write-Host "[build] compiling in $Stage ($($luaSources.Count) lua files + 3 cpp files)..."
Push-Location -Path $Stage
try {
    & cl.exe @cmdArgs
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[build] FAILED (exit $LASTEXITCODE)" -ForegroundColor Red
        exit $LASTEXITCODE
    }
} finally {
    Pop-Location
}

Copy-Item -LiteralPath (Join-Path $Stage "pse_lua.exe") -Destination (Join-Path $Out "pse_lua.exe") -Force
Write-Host "[build] OK: $(Join-Path $Out 'pse_lua.exe')" -ForegroundColor Green
exit 0
