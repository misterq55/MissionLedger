# Builds the macOS disk image (.dmg) (called by the "installer" target on macOS, see the root CMakeLists.txt)
#
#   cmake -DCONFIG=<Release> -DAPP=<path to MissionLedger.app> -DOUT=<.dmg path> -DWORK=<scratch dir> -P package_macos.cmake
#
# The image holds MissionLedger.app and a link to /Applications: the user drags the app onto the link.
# Only built-in tools are used (ditto, hdiutil, plutil). Unsigned for now (docs/CI_PLAN.md §0): on another
# Mac the first start needs System Settings > Privacy & Security > "Open Anyway".
# Release only, like the Windows installer (package.cmake).

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
