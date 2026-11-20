# Makes a release: VERSION, CHANGELOG.md, commit and tag in one step (docs/VERSIONING.md §3)
#
#   release.bat 1.1.0     (Windows)        ./release.sh 1.1.0     (macOS / Linux)
#   = cmake -DVERSION=1.1.0 [-DPUSH=ON] -P scripts/release.cmake
#
# The version number is the only input; everything else follows from it, so VERSION and the tag cannot disagree.
# 1. Checks, and stops without changing anything if one fails:
#    on main, no uncommitted changes to tracked files, not behind origin/main, VERSION format,
#    higher than the current VERSION, tag v<VERSION> not used yet (here or on origin),
#    CHANGELOG.md has entries under "## [미배포]"
# 2. VERSION = <VERSION>; CHANGELOG.md: "## [미배포]" becomes "## [<VERSION>] - <today>" under a new empty
#    "## [미배포]", and the link references at the bottom are updated
# 3. Commit "Release: v<VERSION>" (only VERSION and CHANGELOG.md) and annotated tag v<VERSION>
# 4. Stops before pushing and prints the push and undo commands. PUSH=ON pushes main and the tag too.
# Then run the Jenkins build: it sees the tag on this commit and uploads to Google Drive (docs/CI_PLAN.md §5).
# Messages are in English: cmake writes UTF-8, which a Korean Windows console (CP949) shows garbled.

cmake_minimum_required(VERSION 3.25)

get_filename_component(root "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(repo_url "https://github.com/misterq55/MissionLedger")

function(git out_var)
    execute_process(COMMAND git ${ARGN} WORKING_DIRECTORY "${root}"
        OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE rc OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT rc EQUAL 0)
        string(REPLACE ";" " " args "${ARGN}")
        message(FATAL_ERROR "git ${args} failed (exit code ${rc}):\n${err}")
    endif()
    set(${out_var} "${out}" PARENT_SCOPE)
endfunction()

# ----- 1. Checks -----
if(NOT DEFINED VERSION OR VERSION STREQUAL "")
    message(FATAL_ERROR "Give the new version: release.bat 1.1.0  (or cmake -DVERSION=1.1.0 -P scripts/release.cmake)")
endif()
if(NOT VERSION MATCHES "^(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)$")
    message(FATAL_ERROR "Version must be MAJOR.MINOR.PATCH, e.g. 1.1.0 (got '${VERSION}')")
endif()
set(tag "v${VERSION}")

file(READ "${root}/VERSION" current)
string(STRIP "${current}" current)
if(NOT VERSION VERSION_GREATER current)
    message(FATAL_ERROR "${VERSION} is not higher than the current version ${current}")
endif()

git(branch rev-parse --abbrev-ref HEAD)
if(NOT branch STREQUAL "main")
    message(FATAL_ERROR "Releases are made on main (current branch: ${branch})")
endif()
# Untracked files do not matter: only VERSION and CHANGELOG.md are committed
git(dirty status --porcelain --untracked-files=no)
if(NOT dirty STREQUAL "")
    message(FATAL_ERROR "Commit or stash these changes first:\n${dirty}")
endif()

message(STATUS "Fetching origin (to compare main and the tags)")
git(ignored fetch --quiet --tags origin main)
git(behind rev-list --count HEAD..origin/main)
if(NOT behind EQUAL 0)
    message(FATAL_ERROR "main is ${behind} commit(s) behind origin/main: pull first")
endif()
git(local_tag tag --list ${tag})
git(remote_tag ls-remote --tags origin refs/tags/${tag})
if(NOT local_tag STREQUAL "" OR NOT remote_tag STREQUAL "")
    message(FATAL_ERROR "Tag ${tag} already exists: choose another version")
endif()

# CHANGELOG.md (CRLF in a Windows checkout, LF on macOS: keep what the file has)
file(READ "${root}/CHANGELOG.md" changelog)
set(nl "\n")
if(changelog MATCHES "\r\n")
    set(nl "\r\n")
endif()
string(REPLACE "\r\n" "\n" text "${changelog}")
string(FIND "${text}" "\n## [미배포]\n" pos)
if(pos EQUAL -1)
    message(FATAL_ERROR "CHANGELOG.md has no '## [미배포]' section")
endif()
math(EXPR body_start "${pos} + 1")
string(SUBSTRING "${text}" ${body_start} -1 rest)
string(FIND "${rest}" "\n## [" next)
if(next EQUAL -1)
    set(unreleased "${rest}")
else()
    string(SUBSTRING "${rest}" 0 ${next} unreleased)
endif()
string(REGEX MATCHALL "\n- " entries "${unreleased}")
list(LENGTH entries entry_count)
if(entry_count EQUAL 0)
    message(FATAL_ERROR "CHANGELOG.md has no entries under '## [미배포]': write what changed first")
endif()

# ----- 2. Edit VERSION and CHANGELOG.md -----
string(TIMESTAMP today "%Y-%m-%d")
string(REPLACE "\n## [미배포]\n" "\n## [미배포]\n\n## [${VERSION}] - ${today}\n" text "${text}")
string(REGEX REPLACE "\n\\[미배포\\]: [^\n]*" "\n[미배포]: ${repo_url}/compare/${tag}...HEAD\n[${VERSION}]: ${repo_url}/compare/v${current}...${tag}" text "${text}")
if(NOT text MATCHES "\n\\[${VERSION}\\]: ")
    message(FATAL_ERROR "CHANGELOG.md: the '[미배포]: ...' link line at the bottom was not found (nothing was changed)")
endif()
string(REPLACE "\n" "${nl}" text "${text}")

file(READ "${root}/VERSION" version_file)
set(version_nl "\n")
if(version_file MATCHES "\r\n")
    set(version_nl "\r\n")
endif()
file(WRITE "${root}/VERSION" "${VERSION}${version_nl}")
file(WRITE "${root}/CHANGELOG.md" "${text}")
message(STATUS "VERSION: ${current} -> ${VERSION}")
message(STATUS "CHANGELOG.md: ${entry_count} entries now under [${VERSION}] - ${today}")

# ----- 3. Commit and tag -----
git(ignored add -- VERSION CHANGELOG.md)
git(ignored commit --quiet -m "Release: ${tag}")
git(ignored tag -a ${tag} -m "MissionLedger ${VERSION}")
git(commit_line log --oneline -1)
message(STATUS "Commit: ${commit_line}")
message(STATUS "Tag:    ${tag}")

# ----- 4. Push (only with PUSH=ON) -----
if(PUSH)
    git(ignored push --quiet origin main ${tag})
    message(STATUS "Pushed main and ${tag}. Next: run the Jenkins build (it uploads ${VERSION} to Google Drive)")
else()
    message(STATUS "")
    message(STATUS "Not pushed yet. Check the commit (git show), then:")
    message(STATUS "  git push origin main ${tag}")
    message(STATUS "and run the Jenkins build (it uploads ${VERSION} to Google Drive).")
    message(STATUS "To undo before pushing:")
    message(STATUS "  git tag -d ${tag}  and  git reset --hard HEAD~1")
endif()
