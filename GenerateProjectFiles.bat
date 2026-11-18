@echo off
REM Generate the Visual Studio solution from CMakeLists.txt (like Unreal's GenerateProjectFiles.bat)
REM Output: build\windows\MissionLedger.sln
REM Run once after cloning, or after deleting build\. Later changes are picked up automatically on build.

cd /d "%~dp0"
cmake --preset windows
if errorlevel 1 (
    echo.
    echo ERROR: CMake configure failed. CMake 3.25 or newer and Visual Studio 2022 are required.
    pause
    exit /b 1
)
echo.
echo Done: build\windows\MissionLedger.sln
