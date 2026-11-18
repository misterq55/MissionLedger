@echo off
REM MissionLedger 설치 프로그램 빌드 스크립트
REM
REM 사용법: build_installer.bat   (저장소 루트에서 실행하거나 더블클릭)
REM
REM 요구사항:
REM - CMake 3.25 이상, Visual Studio 2022 (C++ 워크로드)
REM - Inno Setup 6.7.0 이상
REM
REM CMake 프리셋으로 Release 빌드 후 build\windows\bin\Release 의 exe를 포장합니다.

setlocal enabledelayedexpansion
cd /d "%~dp0"

REM ===== 0. 버전 읽기 (VERSION 파일, docs/VERSIONING.md 참고) =====
if not exist "VERSION" (
    echo ERROR: VERSION file not found.
    exit /b 1
)
set /p ML_VERSION=<VERSION
set "EXE_DIR=%CD%\build\windows\bin\Release"
set "SETUP_EXE=installer\output\MissionLedger-%ML_VERSION%-Setup.exe"

echo ====================================
echo MissionLedger Installer Build Script
echo Version: %ML_VERSION%
echo ====================================
echo.

REM ===== 1. CMake 구성 (build\windows) =====
echo [1/4] Configuring (cmake --preset windows)...
cmake --preset windows >nul
if errorlevel 1 (
    echo ERROR: CMake configure failed! Run "cmake --preset windows" to see the details.
    exit /b 1
)
echo [OK] Configured.
echo.

REM ===== 2. Release 빌드 =====
echo [2/4] Building Release configuration...
cmake --build --preset windows-release --parallel
if errorlevel 1 (
    echo ERROR: Release build failed!
    exit /b 1
)
if not exist "%EXE_DIR%\MissionLedger.exe" (
    echo ERROR: MissionLedger.exe not found in %EXE_DIR%
    exit /b 1
)
echo.
echo [OK] Release build succeeded.
echo.

REM ===== 3. Inno Setup 경로 확인 =====
echo [3/4] Checking Inno Setup...

set ISCC="C:\Program Files (x86)\Inno Setup 6\ISCC.exe"

if not exist %ISCC% (
    echo ERROR: Inno Setup Compiler ^(ISCC.exe^) not found!
    echo Please install Inno Setup from: https://jrsoftware.org/isinfo.php
    echo.
    echo Expected path: %ISCC%
    exit /b 1
)

echo [OK] Inno Setup found.
echo.

REM ===== 4. 설치 프로그램 빌드 =====
echo [4/4] Compiling installer...

%ISCC% /DExeDir="%EXE_DIR%" installer\MissionLedger.iss

if errorlevel 1 (
    echo ERROR: Installer compilation failed!
    exit /b 1
)

echo.
echo ====================================
echo Build completed successfully!
echo ====================================
echo.
echo Output: %SETUP_EXE%
echo.

REM 설치 파일 크기 표시
for %%I in ("%SETUP_EXE%") do (
    set size=%%~zI
    set /a sizeMB=!size! / 1048576
    echo File size: !sizeMB! MB
)

echo.
pause
