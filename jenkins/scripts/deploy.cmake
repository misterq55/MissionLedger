# Uploads a release to Google Drive with rclone (called by the Deploy stage of jenkins/Build.Jenkinsfile)
#
#   cmake -DREMOTE=<rclone remote>:<folder> -DFILES=<dir with the installers> -DWORK=<scratch dir>
#         [-DDRY_RUN=ON] [-DRCLONE=<rclone>] -P jenkins/scripts/deploy.cmake
#
# Result on the remote (REMOTE = e.g. gdrive:MissionLedger):
#   <REMOTE>/v<VERSION>/   MissionLedger-<VERSION>-Setup.exe, MissionLedger-<VERSION>.dmg, 릴리스노트.txt
#   <REMOTE>/latest/       the same files, replaced on every release (share this folder: its link stays the same)
#
# Safety:
#   - both installers of VERSION must be in FILES, and CHANGELOG.md must have a "## [<VERSION>]" section
#   - <REMOTE>/v<VERSION>/ must not exist yet: a release is never overwritten
#   - latest/ is updated only after v<VERSION>/ is complete, and only if VERSION is the highest version
#     on the remote (a later patch of an older line does not move latest/ back)
#   - latest/ is synced, not deleted and recreated: the folder keeps its Drive ID, so its share link keeps working
# DRY_RUN: everything is read and checked, the writes run with --dry-run (rclone reports what it would do).
# rclone reads its configuration from RCLONE_CONFIG (the Jenkins secret file); nothing is stored here.

cmake_minimum_required(VERSION 3.25)

foreach(var REMOTE FILES WORK)
    if(NOT DEFINED ${var})
        message(FATAL_ERROR "deploy.cmake: -D${var}=... is required")
    endif()
endforeach()
if(NOT REMOTE MATCHES "^([^:]+):(.+)$")
    message(FATAL_ERROR "deploy.cmake: REMOTE must be <remote>:<folder>, got '${REMOTE}'")
endif()
set(remote_name "${CMAKE_MATCH_1}")
get_filename_component(root "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
# Relative FILES / WORK (as the Jenkinsfile passes them) are relative to the current directory = the workspace
get_filename_component(FILES "${FILES}" ABSOLUTE)
get_filename_component(WORK "${WORK}" ABSOLUTE)

file(READ "${root}/VERSION" version)
string(STRIP "${version}" version)
set(dry "")
if(DRY_RUN)
    set(dry --dry-run)
    message(STATUS "DRY RUN: nothing is written to the remote")
endif()

if(NOT RCLONE)
    find_program(RCLONE rclone)
endif()
if(NOT RCLONE)
    message(FATAL_ERROR "rclone not found in PATH on this node (https://rclone.org/install/). "
                        "Install it and add its folder to the node's PATH (Jenkins node settings: PATH+RCLONE=...)")
endif()

# rclone wrapper: fails the deploy on an error unless ALLOW_FAIL is given (then the exit code is returned and
# the caller decides; rclone's own "ERROR : directory not found" for an expected missing folder is not printed)
function(run_rclone out_var)
    cmake_parse_arguments(R "ALLOW_FAIL" "RESULT" "" ${ARGN})
    execute_process(COMMAND "${RCLONE}" ${R_UNPARSED_ARGUMENTS}
        OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0 AND NOT R_ALLOW_FAIL)
        string(REPLACE ";" " " args "${R_UNPARSED_ARGUMENTS}")
        message(FATAL_ERROR "rclone ${args} failed (exit code ${rc}):\n${err}${out}")
    endif()
    set(${out_var} "${out}" PARENT_SCOPE)
    if(R_RESULT)
        set(${R_RESULT} "${rc}" PARENT_SCOPE)
    endif()
    if(err AND rc EQUAL 0)
        message("${err}")   # rclone writes its progress and --dry-run notices to stderr
    endif()
endfunction()

run_rclone(ver version)
string(REGEX MATCH "^[^\n]*" ver "${ver}")
message(STATUS "${ver} (${RCLONE})")

# The remote must be in the configuration (RCLONE_CONFIG)
run_rclone(remotes listremotes)
string(REPLACE "\n" ";" remotes "${remotes}")
if(NOT "${remote_name}:" IN_LIST remotes)
    string(REPLACE ";" " " remotes "${remotes}")
    message(FATAL_ERROR "rclone has no remote '${remote_name}' (configured: ${remotes}). "
                        "Fix DRIVE_REMOTE in jenkins/Build.Jenkinsfile or the secret file")
endif()

# ----- Files to upload: both installers + release notes from CHANGELOG.md -----
set(stage "${WORK}/deploy")
file(REMOVE_RECURSE "${stage}")
file(MAKE_DIRECTORY "${stage}")
foreach(name "MissionLedger-${version}-Setup.exe" "MissionLedger-${version}.dmg")
    if(NOT EXISTS "${FILES}/${name}")
        message(FATAL_ERROR "Missing installer: ${FILES}/${name} (a release needs PLATFORM=both, Release)")
    endif()
    file(COPY "${FILES}/${name}" DESTINATION "${stage}")
endforeach()

file(READ "${root}/CHANGELOG.md" changelog)
string(REPLACE "\r\n" "\n" changelog "${changelog}")
string(FIND "${changelog}" "\n## [${version}]" start)
if(start EQUAL -1)
    message(FATAL_ERROR "CHANGELOG.md has no '## [${version}]' section (release.bat / release.sh writes it)")
endif()
math(EXPR start "${start} + 1")
string(SUBSTRING "${changelog}" ${start} -1 notes)
string(FIND "${notes}" "\n## [" end)            # next version section
string(FIND "${notes}" "\n[미배포]: " links)      # link references at the bottom
if(end EQUAL -1 OR (NOT links EQUAL -1 AND links LESS end))
    set(end ${links})
endif()
if(NOT end EQUAL -1)
    string(SUBSTRING "${notes}" 0 ${end} notes)
endif()
string(STRIP "${notes}" notes)
# Notepad-friendly: CRLF and a UTF-8 BOM (Windows Notepad before 2019 shows Korean without a BOM as garbage)
string(REPLACE "\n" "\r\n" notes "${notes}\n")
string(ASCII 239 187 191 bom)   # EF BB BF
file(WRITE "${stage}/릴리스노트.txt" "${bom}${notes}")

file(GLOB staged RELATIVE "${stage}" "${stage}/*")
message(STATUS "To upload: ${staged}")

# ----- v<VERSION>/ must not exist yet -----
set(dest "${REMOTE}/v${version}")
run_rclone(existing lsf "${dest}" ALLOW_FAIL RESULT rc)
if(rc EQUAL 0 AND NOT existing STREQUAL "")
    message(FATAL_ERROR "${dest} already exists on the remote: a release is never overwritten. "
                        "Make a new version (release.bat / release.sh), or delete the folder by hand if this upload was broken")
elseif(NOT rc EQUAL 0 AND NOT rc EQUAL 3)   # 3 = directory not found: fine, it is created by the copy
    message(FATAL_ERROR "rclone lsf ${dest} failed (exit code ${rc})")
endif()

# ----- Upload the version folder -----
message(STATUS "Uploading to ${dest}/")
run_rclone(out copy "${stage}" "${dest}" ${dry})
if(NOT DRY_RUN)
    run_rclone(listing lsl "${dest}")
    message(STATUS "${dest}:\n${listing}")
endif()

# ----- latest/: only if this is the highest version on the remote -----
run_rclone(dirs lsf --dirs-only "${REMOTE}" ALLOW_FAIL RESULT rc)
if(rc EQUAL 3)          # the folder does not exist yet (first release in a dry run)
    set(dirs "")
elseif(NOT rc EQUAL 0)
    message(FATAL_ERROR "rclone lsf --dirs-only ${REMOTE} failed (exit code ${rc})")
endif()
string(REPLACE "\n" ";" dirs "${dirs}")
set(highest "${version}")
foreach(d IN LISTS dirs)
    if(d MATCHES "^v([0-9]+\\.[0-9]+\\.[0-9]+)/$" AND CMAKE_MATCH_1 VERSION_GREATER highest)
        set(highest "${CMAKE_MATCH_1}")
    endif()
endforeach()
if(highest VERSION_GREATER version)
    message(STATUS "latest/ not changed: ${highest} on the remote is newer than ${version}")
else()
    message(STATUS "Updating ${REMOTE}/latest/ to ${version}")
    run_rclone(out sync "${stage}" "${REMOTE}/latest" ${dry})
    if(NOT DRY_RUN)
        run_rclone(listing lsl "${REMOTE}/latest")
        message(STATUS "${REMOTE}/latest:\n${listing}")
    endif()
endif()
file(REMOVE_RECURSE "${stage}")
if(DRY_RUN)
    message(STATUS "Deploy ${version}: dry run finished, nothing was written")
else()
    message(STATUS "Deploy ${version}: done")
endif()
