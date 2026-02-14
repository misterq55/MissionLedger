@echo off
REM MissionLedger 설치 프로그램 빌드 스크립트
REM
REM 사용법: build_installer.bat
REM
REM 요구사항:
REM - Visual Studio 2022 (MSBuild)
REM - Inno Setup 6.7.0 이상

setlocal enabledelayedexpansion

echo ====================================
echo MissionLedger Installer Build Script
echo ====================================
echo.

REM ===== 1. Release 빌드 =====
echo [1/4] Building Release configuration...
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ^
    MissionLedger.sln ^
    /p:Configuration=Release ^
    /p:Platform=x64 ^
    /v:minimal ^
    /nologo

if errorlevel 1 (
    echo ERROR: Release build failed!
    exit /b 1
)

echo.
echo [OK] Release build succeeded.
echo.

REM ===== 2. 실행 파일 확인 =====
echo [2/4] Checking executable...

if not exist "x64\Release\MissionLedger.exe" (
    echo ERROR: MissionLedger.exe not found in x64\Release\
    exit /b 1
)

echo [OK] MissionLedger.exe found.
echo.

REM ===== 3. Inno Setup 경로 확인 =====
echo [3/4] Checking Inno Setup...

set ISCC="C:\Program Files (x86)\Inno Setup 6\ISCC.exe"

if not exist %ISCC% (
    echo ERROR: Inno Setup Compiler (ISCC.exe) not found!
    echo Please install Inno Setup from: https://jrsoftware.org/isinfo.php
    echo.
    echo Expected path: %ISCC%
    exit /b 1
)

echo [OK] Inno Setup found.
echo.

REM ===== 4. 설치 프로그램 빌드 =====
echo [4/4] Compiling installer...

%ISCC% installer\MissionLedger.iss

if errorlevel 1 (
    echo ERROR: Installer compilation failed!
    exit /b 1
)

echo.
echo ====================================
echo Build completed successfully!
echo ====================================
echo.
echo Output: installer\output\MissionLedger-1.0.0-Setup.exe
echo.

REM 설치 파일 크기 표시
for %%I in (installer\output\MissionLedger-1.0.0-Setup.exe) do (
    set size=%%~zI
    set /a sizeMB=!size! / 1048576
    echo File size: !sizeMB! MB
)

echo.
pause
