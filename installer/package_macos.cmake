# Builds the macOS disk image (.dmg) (called by the "installer" target on macOS, see the root CMakeLists.txt)
#
#   cmake -DCONFIG=<Release> -DAPP=<path to MissionLedger.app> -DOUT=<.dmg path> -DWORK=<scratch dir>
#         [-DARCHS=arm64,x86_64] -P package_macos.cmake
#
# Before packaging, the app's executables (the app and mledger) are checked (docs/CI_PLAN.md §7) and the build fails if
#   - one is missing from the bundle or the signature does not match the bundle (codesign --verify), or
#   - it lacks one of ARCHS (lipo -archs; ARCHS = CMAKE_OSX_ARCHITECTURES, comma separated), or
#   - it links a library outside /usr/lib and /System (otool -L), e.g. from Homebrew, that other Macs lack.
#
# The image holds MissionLedger.app and a link to /Applications: the user drags the app onto the link.
# Only built-in tools are used (ditto, hdiutil, plutil). Unsigned for now (docs/CI_PLAN.md §0): on another
# Mac the first start needs System Settings > Privacy & Security > "Open Anyway".
# Release only, like the Windows installer (package.cmake).

# Script mode (-P) starts with old policy defaults: without this, if(... IN_LIST ...) is an error (CMP0057)
cmake_minimum_required(VERSION 3.25)

if(NOT CONFIG STREQUAL "Release")
    message(FATAL_ERROR "The disk image is built from Release only (current configuration: '${CONFIG}'). "
                        "Use: cmake --build --preset macos-installer")
endif()
foreach(var APP OUT WORK)
    if(NOT DEFINED ${var})
        message(FATAL_ERROR "package_macos.cmake: -D${var}=... is required")
    endif()
endforeach()
if(NOT IS_DIRECTORY "${APP}")
    message(FATAL_ERROR "App bundle not found: ${APP}")
endif()

# Show the bundle's Info.plist in the log (version, identifier, minimum macOS), and fail if it is malformed
execute_process(COMMAND plutil -lint "${APP}/Contents/Info.plist" RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "Info.plist is not valid: ${APP}/Contents/Info.plist")
endif()
execute_process(COMMAND plutil -p "${APP}/Contents/Info.plist")

# Executables in the bundle: the app and the CLI (Contents/MacOS/mledger, copied in by MissionLedger/CMakeLists.txt)
get_filename_component(exe_name "${APP}" NAME_WLE)
set(exes "${APP}/Contents/MacOS/${exe_name}" "${APP}/Contents/MacOS/mledger")
foreach(exe IN LISTS exes)
    if(NOT EXISTS "${exe}")
        message(FATAL_ERROR "Missing in the app bundle: ${exe}")
    endif()

    # Architectures (Universal build: arm64 = Apple Silicon, x86_64 = Intel)
    execute_process(COMMAND lipo -archs "${exe}" OUTPUT_VARIABLE archs OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "lipo -archs failed on ${exe}")
    endif()
    message(STATUS "Architectures of ${exe}: ${archs}")
    if(ARCHS)
        string(REPLACE "," ";" wanted "${ARCHS}")
        string(REPLACE " " ";" found "${archs}")
        foreach(a IN LISTS wanted)
            if(NOT a IN_LIST found)
                message(FATAL_ERROR "${exe} has no ${a} code (found: ${archs}, expected: ${ARCHS})")
            endif()
        endforeach()
    endif()

    # Linked libraries: only the system's (all of ours are static). -arch all: every half of a Universal file
    # (without it otool prints only the build Mac's architecture)
    execute_process(COMMAND otool -arch all -L "${exe}" OUTPUT_VARIABLE libs RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "otool -arch all -L failed on ${exe}")
    endif()
    message(STATUS "Linked libraries:\n${libs}")
    string(REPLACE "\n" ";" lib_lines "${libs}")
    set(foreign "")
    foreach(line IN LISTS lib_lines)
        # "<tab>/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version ...)"
        if(line MATCHES "^[ \t]+([^ \t].*) \\(compatibility version")
            set(lib "${CMAKE_MATCH_1}")
            if(NOT lib MATCHES "^/usr/lib/" AND NOT lib MATCHES "^/System/")
                list(APPEND foreign "${lib}")
            endif()
        endif()
    endforeach()
    if(foreign)
        list(REMOVE_DUPLICATES foreign)
        message(FATAL_ERROR "${exe} links libraries that other Macs may not have: ${foreign}")
    endif()
endforeach()

# Signature (ad-hoc): the seal must match the bundle's contents, including mledger copied in after the build.
# A broken seal makes macOS call a downloaded app "damaged" instead of offering "Open Anyway"
execute_process(COMMAND codesign --verify --deep --strict --verbose=2 "${APP}" RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "codesign --verify failed on ${APP}: the signature does not match the bundle")
endif()

# Staging folder: the app (ditto keeps permissions, symlinks and the signature) + Applications link
set(stage "${WORK}/dmg")
file(REMOVE_RECURSE "${stage}")
file(MAKE_DIRECTORY "${stage}")
get_filename_component(app_name "${APP}" NAME)
execute_process(COMMAND ditto "${APP}" "${stage}/${app_name}" RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "ditto failed (exit code ${rc})")
endif()
file(CREATE_LINK /Applications "${stage}/Applications" SYMBOLIC)

get_filename_component(out_dir "${OUT}" DIRECTORY)
file(MAKE_DIRECTORY "${out_dir}")
file(REMOVE "${OUT}")
get_filename_component(volume "${OUT}" NAME_WLE)   # NAME_WE would cut at the first dot of the version
execute_process(
    COMMAND hdiutil create -volname "${volume}" -srcfolder "${stage}" -ov -format UDZO "${OUT}"
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "hdiutil create failed (exit code ${rc})")
endif()
file(REMOVE_RECURSE "${stage}")
message(STATUS "Disk image: ${OUT}")
