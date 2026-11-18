# Run on a Windows build node (bypasses the default script policy for this run only):
#   powershell -NoProfile -ExecutionPolicy Bypass -File jenkins\scripts\node_check_windows.ps1
# Read-only. See docs/CI_PLAN.md S5, S8
# ===== MissionLedger build node check: Windows (read-only, PowerShell) =====
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
"`n--- CMake (needs 3.25+, stable release)"
Get-Command cmake -All -ErrorAction SilentlyContinue | ForEach-Object { "  PATH : $($_.Source)" }
@("$env:ProgramFiles\CMake\bin\cmake.exe") | Where-Object { Test-Path $_ } | ForEach-Object { "  found: $_  ($((& $_ --version)[0]))" }
if (Test-Path $vswhere) { & $vswhere -all -products * -property installationPath | ForEach-Object {
    Get-ChildItem "$_\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -ErrorAction SilentlyContinue | ForEach-Object { "  VS-bundled: $($_.FullName)" } } }
"`n--- Visual Studio 2022 (needs C++ x64 tools v143)"
if (Test-Path $vswhere) {
    & $vswhere -all -products * -format json | ConvertFrom-Json | ForEach-Object { "  {0} {1}  {2}" -f $_.displayName, $_.installationVersion, $_.installationPath }
    $cpp = & $vswhere -all -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($cpp) { $cpp | ForEach-Object { "  C++ x64 tools: OK  ($_)"
        $r = "$_\VC\Redist\MSVC\v143\vc_redist.x64.exe"; "  vc_redist.x64.exe: " + $(if (Test-Path $r) { "OK" } else { "MISSING ($r)" }) } }
    else { "  C++ x64 tools: MISSING (Desktop development with C++ workload)" }
} else { "  vswhere not found: Visual Studio / Build Tools not installed" }
"`n--- Inno Setup 6"
@("${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe", "$env:ProgramFiles\Inno Setup 6\ISCC.exe", "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe") | ForEach-Object { "  {0}: {1}" -f $(if (Test-Path $_) { "found  " } else { "missing" }), $_ }
Get-ItemProperty "HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Inno Setup 6_is1", "HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\Inno Setup 6_is1" -ErrorAction SilentlyContinue | ForEach-Object { "  installed version: $($_.DisplayVersion)  ($($_.InstallLocation))" }
"`n--- Git"
Get-Command git -All -ErrorAction SilentlyContinue | ForEach-Object { "  $($_.Source)  ($((& $_.Source --version)))" }
"`n--- Jenkins agent (service or process, and the account it runs as)"
Get-CimInstance Win32_Service | Where-Object { $_.Name -match 'jenkins' -or $_.PathName -match 'jenkins|agent\.jar' } | ForEach-Object { "  service: {0} [{1}] as {2}`n    {3}" -f $_.Name, $_.State, $_.StartName, $_.PathName }
Get-CimInstance Win32_Process -Filter "Name='java.exe' OR Name='javaw.exe'" | Where-Object { $_.CommandLine -match 'agent\.jar|remoting|slave\.jar' } | ForEach-Object { "  process {0}: {1}" -f $_.ProcessId, $_.CommandLine }
"`n--- PATH entries for these tools (an agent running as a service sees only the System PATH)"
foreach ($scope in 'Machine', 'User') {
    ([Environment]::GetEnvironmentVariable('Path', $scope) -split ';') | Where-Object { $_ -match 'cmake|git|inno' } | ForEach-Object { "  [$scope] $_" } }
