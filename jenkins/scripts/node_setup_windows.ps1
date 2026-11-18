# Installs the general tools a Windows build node needs, only the missing ones: CMake, Inno Setup.
# Run once by hand on the node, in an administrator PowerShell (the pipeline only checks, it never installs):
#   powershell -NoProfile -ExecutionPolicy Bypass -File jenkins\scripts\node_setup_windows.ps1
# Safe to run again: tools that are present are skipped. Visual Studio and Git are only checked
# (large installs with options): the script says what to install. See docs/CI_PLAN.md S5, S8

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'      # Invoke-WebRequest is very slow with the progress bar in PowerShell 5.1
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

# Pinned versions (update together with the hashes; SHA-256 from the release pages)
$CMakeVersion = '3.31.12'
$CMakeUrl     = "https://github.com/Kitware/CMake/releases/download/v$CMakeVersion/cmake-$CMakeVersion-windows-x86_64.msi"
$CMakeSha256  = '828b488fc9563d48d20768ddf818510bb07387a787197c6d10e683dda63b94d3'
$InnoVersion  = '6.7.4'
$InnoUrl      = "https://github.com/jrsoftware/issrc/releases/download/is-$($InnoVersion -replace '\.', '_')/innosetup-$InnoVersion.exe"
$InnoSha256   = 'a549929319f9120716677c685535ca4671e4b937b5fdc87e2585f32d8eb30e8c'

$CMakeBin = "$env:ProgramFiles\CMake\bin"
$IsccPath = "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe"
$vswhere  = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$work     = Join-Path $env:TEMP 'missionledger-node-setup'
$todo     = New-Object System.Collections.Generic.List[string]

$admin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $admin) {
    Write-Host 'ERROR: run this in an administrator PowerShell (installs for all users and changes the system PATH).' -ForegroundColor Red
    exit 1
}

function Get-Installer($url, $sha256) {
    New-Item -ItemType Directory -Force $work | Out-Null
    $file = Join-Path $work ([IO.Path]::GetFileName($url))
    Write-Host "  downloading $url"
    Invoke-WebRequest -Uri $url -OutFile $file -UseBasicParsing
    $actual = (Get-FileHash -Algorithm SHA256 $file).Hash.ToLower()
    if ($actual -ne $sha256) { throw "SHA-256 mismatch for $file (expected $sha256, got $actual)" }
    return $file
}

function Add-MachinePath($dir) {
    $path = [Environment]::GetEnvironmentVariable('Path', 'Machine')
    if (($path -split ';') -notcontains $dir) {
        [Environment]::SetEnvironmentVariable('Path', ($path.TrimEnd(';') + ';' + $dir), 'Machine')
        Write-Host "  added to the system PATH: $dir"
    }
}

# ----- CMake -----
Write-Host "`n--- CMake (pinned $CMakeVersion)"
if (Test-Path "$CMakeBin\cmake.exe") {
    Write-Host "  already installed: $((& "$CMakeBin\cmake.exe" --version)[0])  ($CMakeBin)"
} else {
    $other = Get-Command cmake -ErrorAction SilentlyContinue
    if ($other) { Write-Host "  note: another cmake is on the PATH ($($other.Source)); installing $CMakeVersion to $CMakeBin anyway" }
    $msi = Get-Installer $CMakeUrl $CMakeSha256
    Write-Host '  installing (all users, system PATH)...'
    $p = Start-Process msiexec.exe -Wait -PassThru -ArgumentList @('/i', "`"$msi`"", '/qn', '/norestart', 'ALLUSERS=1', 'ADD_CMAKE_TO_PATH=System')
    if ($p.ExitCode -ne 0 -and $p.ExitCode -ne 3010) { throw "CMake installer failed (msiexec exit code $($p.ExitCode))" }
    Write-Host "  installed: $((& "$CMakeBin\cmake.exe" --version)[0])"
}
Add-MachinePath $CMakeBin     # the MSI option normally does this; make sure

# ----- Inno Setup -----
Write-Host "`n--- Inno Setup (pinned $InnoVersion)"
if (Test-Path $IsccPath) {
    $v = (Get-ItemProperty 'HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Inno Setup 6_is1' -ErrorAction SilentlyContinue).DisplayVersion
    Write-Host "  already installed: $v  ($IsccPath)"
} else {
    if (Test-Path "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe") {
        Write-Host '  note: a per-user Inno Setup exists; the build only looks in Program Files, so installing for all users'
    }
    $exe = Get-Installer $InnoUrl $InnoSha256
    Write-Host '  installing (all users)...'
    $p = Start-Process $exe -Wait -PassThru -ArgumentList @('/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART', '/SP-', '/ALLUSERS')
    if ($p.ExitCode -ne 0) { throw "Inno Setup installer failed (exit code $($p.ExitCode))" }
    if (-not (Test-Path $IsccPath)) { throw "Inno Setup installed, but $IsccPath was not found" }
    Write-Host "  installed: $IsccPath"
}

# ----- Checked only -----
Write-Host "`n--- Visual Studio 2022 C++ tools, Git (checked only)"
$cpp = if (Test-Path $vswhere) { & $vswhere -version '[17.0,18.0)' -products * -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath }
if ($cpp) { Write-Host "  Visual Studio 2022 C++ tools: OK ($cpp)" }
else { $todo.Add('Install Visual Studio 2022 or Build Tools 2022 with the "Desktop development with C++" workload') }
if (Get-Command git -ErrorAction SilentlyContinue) { Write-Host "  Git: OK ($((git --version)))" }
else { $todo.Add('Install Git for Windows (https://git-scm.com) and keep "Git from the command line" (system PATH)') }

if (Test-Path $work) { Remove-Item -Recurse -Force $work }

# ----- What is left to do by hand -----
Write-Host "`n=== Next steps"
foreach ($t in $todo) { Write-Host "  - $t" -ForegroundColor Yellow }
Write-Host '  - Let the Jenkins agent see CMake. A Windows service keeps the PATH it started with, so either:'
Write-Host "      Jenkins: Manage Jenkins > Nodes > (this node) > Configure > Environment variables: PATH+CMAKE = $CMakeBin"
Write-Host '      or reboot this PC'
Write-Host '  - Then run the build again (Preflight checks everything).'
if ($PSScriptRoot -and (Test-Path "$PSScriptRoot\node_check_windows.ps1")) {
    Write-Host "`n=== Node check"
    & "$PSScriptRoot\node_check_windows.ps1"
}
