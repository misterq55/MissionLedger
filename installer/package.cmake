# Builds the Windows installer with Inno Setup (called by the "installer" target, see the root CMakeLists.txt)
#
#   cmake -DCONFIG=<Release> -DISCC=<ISCC.exe> -DEXE_DIR=<dir with MissionLedger.exe> -DISS=<MissionLedger.iss> -P package.cmake
#
# Only Release is packaged: the .iss version check cannot tell a Debug exe from a Release one
# (both carry the same version), so a Debug build would otherwise ship silently.

if(NOT CONFIG STREQUAL "Release")
    message(FATAL_ERROR "The installer is built from Release only (current configuration: '${CONFIG}'). "
                        "Use: cmake --build --preset windows-installer")
endif()
if(NOT ISCC OR NOT EXISTS "${ISCC}")
    message(FATAL_ERROR "Inno Setup (ISCC.exe) not found. Install Inno Setup 6 from https://jrsoftware.org/isinfo.php "
                        "or configure with -DML_ISCC=<path to ISCC.exe>")
endif()

file(TO_NATIVE_PATH "${EXE_DIR}" exe_dir_native)
execute_process(
    COMMAND "${ISCC}" "/DExeDir=${exe_dir_native}" "${ISS}"
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "ISCC failed (exit code ${rc})")
endif()
