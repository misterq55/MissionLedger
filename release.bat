@echo off
REM MissionLedger release: VERSION, CHANGELOG.md, commit and tag in one step (docs/VERSIONING.md)
REM
REM   release.bat 1.1.0           check, edit, commit, tag (stops before pushing)
REM   release.bat 1.1.0 push      the same, then pushes main and the tag
REM   release.bat                 asks for the version (double-click)
REM
REM Same as: cmake -DVERSION=1.1.0 -P scripts\release.cmake   (macOS / Linux: ./release.sh)
REM After pushing, run the Jenkins build: it uploads the release to Google Drive.

setlocal
cd /d "%~dp0"

set "ML_VERSION=%~1"
set "ML_PUSH=OFF"
if /i "%~2"=="push" set "ML_PUSH=ON"
set "ML_ASKED="
if "%ML_VERSION%"=="" (
    set /p ML_CURRENT=<VERSION
    call echo Current version: %%ML_CURRENT%%
    set /p "ML_VERSION=New version (e.g. 1.0.1 / 1.1.0 / 2.0.0): "
    set "ML_ASKED=1"
)

cmake "-DVERSION=%ML_VERSION%" "-DPUSH=%ML_PUSH%" -P scripts\release.cmake
set "ML_RC=%errorlevel%"
if defined ML_ASKED pause
exit /b %ML_RC%
