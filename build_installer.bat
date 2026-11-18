@echo off
REM MissionLedger 설치 프로그램 빌드 (로컬용, 더블클릭 가능)
REM
REM 아래 명령 한 줄과 같습니다. CI(Jenkins)는 이 파일 대신 프리셋을 단계별로 부릅니다.
REM   cmake --workflow --preset windows-release
REM   = 구성 -> Release 빌드 -> 스모크 테스트 -> 설치 파일 (installer\output\MissionLedger-<버전>-Setup.exe)
REM 테스트가 실패하면 설치 파일을 만들지 않습니다.
REM
REM 요구사항: CMake 3.25 이상, Visual Studio 2022 (C++ 워크로드), Inno Setup 6.7.0 이상

cd /d "%~dp0"

cmake --workflow --preset windows-release
if errorlevel 1 (
    echo.
    echo ERROR: Installer build failed. See the messages above.
    pause
    exit /b 1
)

echo.
echo Build completed. Output: installer\output\
pause
