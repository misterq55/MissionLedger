# Build node check (Windows / macOS), run before every build by jenkins/Build.Jenkinsfile
#
#   cmake -DPLATFORM=<windows|macos> [-DWORKSPACE=<path>] [-DREQUIRE_RCLONE=ON] -P jenkins/scripts/preflight.cmake
#
# Checks only general tools (docs/CI_PLAN.md §8): project dependencies come from the repository.
# Every problem is listed, then the script fails once, so a new node can be fixed in one pass.
# Also usable on a developer PC.

if(NOT PLATFORM MATCHES "^(windows|macos)$")
    message(FATAL_ERROR "preflight: -DPLATFORM=windows or -DPLATFORM=macos is required")
endif()

# "ProgramFiles(x86)" cannot be written inside $ENV{} in script mode (-P has no policy CMP0010)
set(pf86_var "ProgramFiles(x86)")
set(PF86 "$ENV{${pf86_var}}")

set(problems "")
# Functions, not macros: macro arguments are re-parsed, which breaks Windows paths (backslashes)
function(problem text)
    message(STATUS "  [FAIL] ${text}")
    set(problems ${problems} "${text}" PARENT_SCOPE)
endfunction()
function(ok text)
    message(STATUS "  [ OK ] ${text}")
endfunction()

message(STATUS "MissionLedger preflight (${PLATFORM})")

# ----- CMake: 3.25+ (workflow presets); a release candidate works but should not be used on a node -----
if(CMAKE_VERSION VERSION_LESS 3.25)
    problem("CMake ${CMAKE_VERSION} is too old (3.25 or later required)")
else()
    ok("CMake ${CMAKE_VERSION}")
    if(CMAKE_VERSION MATCHES "rc")
        message(WARNING "CMake ${CMAKE_VERSION} is a release candidate; install a stable release on build nodes")
    endif()
endif()

# ----- Git -----
find_program(PF_GIT git)
if(PF_GIT)
    execute_process(COMMAND "${PF_GIT}" --version OUTPUT_VARIABLE out OUTPUT_STRIP_TRAILING_WHITESPACE)
    ok("${out}")
else()
    problem("git not found in PATH")
endif()

# ----- rclone (Google Drive upload): required only on the node that deploys (REQUIRE_RCLONE=ON, set by
#       the Jenkinsfile when this build uploads); elsewhere it is only reported -----
find_program(PF_RCLONE rclone)
if(PF_RCLONE)
    execute_process(COMMAND "${PF_RCLONE}" version OUTPUT_VARIABLE out OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX MATCH "^[^\n]*" out "${out}")
    ok("${out} (${PF_RCLONE})")
elseif(REQUIRE_RCLONE)
    problem("rclone not found in PATH (needed to upload the release: https://rclone.org/install/, then PATH+RCLONE=<its folder> in the node settings)")
else()
    message(STATUS "  [ -- ] rclone not found (only needed on the node that uploads releases)")
endif()

if(PLATFORM STREQUAL "windows")
    # ----- Visual Studio 2022 (IDE or Build Tools) with the C++ x64 tools -----
    set(vswhere "${PF86}/Microsoft Visual Studio/Installer/vswhere.exe")
    set(vs_path "")
    if(EXISTS "${vswhere}")
        execute_process(
            COMMAND "${vswhere}" -version "[17.0,18.0)" -products * -latest
                    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64
                    -property installationPath
            OUTPUT_VARIABLE vs_path OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
    if(vs_path)
        execute_process(
            COMMAND "${vswhere}" -version "[17.0,18.0)" -products * -latest
                    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64
                    -property displayName
            OUTPUT_VARIABLE vs_name OUTPUT_STRIP_TRAILING_WHITESPACE)
        ok("${vs_name} (${vs_path})")
        # The installer bundles vc_redist.x64.exe from the VS folder (installer/MissionLedger.iss)
        if(EXISTS "${vs_path}/VC/Redist/MSVC/v143/vc_redist.x64.exe")
            ok("vc_redist.x64.exe (VC/Redist/MSVC/v143)")
        else()
            problem("vc_redist.x64.exe not found in ${vs_path}/VC/Redist/MSVC/v143 (needed by the installer)")
        endif()
    else()
        problem("Visual Studio 2022 with \"Desktop development with C++\" (MSVC v143 x64) not found (checked with vswhere)")
    endif()

    # ----- Inno Setup (installer target) -----
    find_program(PF_ISCC ISCC HINTS "${PF86}/Inno Setup 6" "$ENV{ProgramFiles}/Inno Setup 6")
    if(PF_ISCC)
        ok("Inno Setup: ${PF_ISCC}")
    else()
        problem("Inno Setup 6 (ISCC.exe) not found (https://jrsoftware.org/isinfo.php)")
    endif()

    # ----- Workspace path length: the longest path under it is about 131 characters (measured 2026-11-18) -----
    if(WORKSPACE)
        string(LENGTH "${WORKSPACE}" len)
        if(len GREATER 120)
            message(WARNING "Workspace path is ${len} characters (${WORKSPACE}); paths may exceed the Windows 260-character limit. Use a shorter agent root or customWorkspace")
        else()
            ok("Workspace path length ${len} (${WORKSPACE})")
        endif()
    endif()
else()
    # ----- Xcode (command line tools alone are not enough for the app bundle) -----
    execute_process(COMMAND xcodebuild -version
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(rc EQUAL 0)
        string(REPLACE "\n" ", " out "${out}")
        ok("${out}")
    else()
        problem("xcodebuild failed (install Xcode and run: sudo xcode-select -s /Applications/Xcode.app): ${err}")
    endif()
endif()

if(problems)
    list(LENGTH problems n)
    list(JOIN problems "\n  - " text)
    # Installing is done by hand, once per node (the pipeline only checks)
    if(PLATFORM STREQUAL "windows")
        set(fix "In an administrator PowerShell on this node:\n    powershell -NoProfile -ExecutionPolicy Bypass -File jenkins\\scripts\\node_setup_windows.ps1\n  It installs CMake and Inno Setup if missing and says what else to install (Visual Studio, Git).")
    else()
        set(fix "On this node, as the agent user:\n    sh jenkins/scripts/node_setup_macos.sh\n  It installs CMake if missing and says what else to install (Xcode).")
    endif()
    message(FATAL_ERROR "preflight: ${n} problem(s) on this node:\n  - ${text}\n\nTo fix: ${fix}\nThen check the agent's PATH (docs/CI_PLAN.md S5) and run the build again.")
endif()
message(STATUS "preflight: OK")
