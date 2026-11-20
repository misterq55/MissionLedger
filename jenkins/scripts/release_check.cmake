# Is this build a release? (called by jenkins/Build.Jenkinsfile in Preflight and Deploy)
#
#   cmake -DOUT=<result file> -P jenkins/scripts/release_check.cmake
#
# A build is a release when the remote has the tag v<VERSION> and it points to the commit being built.
# The tag is made by release.bat / release.sh (scripts/release.cmake) together with VERSION, so the two
# only disagree by mistake. Writes "yes" or "no" to OUT and prints why.
#   - no tag v<VERSION> yet, or it points to an older commit (work after a release): "no"
#   - another v* tag points to this commit (tag and VERSION disagree): error
# Only reads: git rev-parse, git ls-remote (the job is not Multibranch and does not fetch tags itself).

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED OUT)
    message(FATAL_ERROR "release_check.cmake: -DOUT=<result file> is required")
endif()
get_filename_component(root "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

file(READ "${root}/VERSION" version)
string(STRIP "${version}" version)
set(tag "v${version}")

execute_process(COMMAND git rev-parse HEAD WORKING_DIRECTORY "${root}"
    OUTPUT_VARIABLE head OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "git rev-parse HEAD failed")
endif()

# Every tag on the remote with the commit it points to ("^{}" lines: annotated tags, peeled)
execute_process(COMMAND git ls-remote --tags origin WORKING_DIRECTORY "${root}"
    OUTPUT_VARIABLE remote_tags RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "git ls-remote --tags origin failed")
endif()
# Lightweight tags have one line (the commit); annotated tags two (tag object, then "^{}" = the commit)
string(REPLACE "\n" ";" lines "${remote_tags}")
set(names "")
foreach(line IN LISTS lines)
    if(line MATCHES "^([0-9a-f]+)[ \t]+refs/tags/(v[0-9]+\\.[0-9]+\\.[0-9]+)(\\^\\{\\})?$")
        set(name "${CMAKE_MATCH_2}")
        if(CMAKE_MATCH_3 OR NOT DEFINED commit_${name})
            set(commit_${name} "${CMAKE_MATCH_1}")
        endif()
        list(APPEND names "${name}")
    endif()
endforeach()
list(REMOVE_DUPLICATES names)
set(tag_commit "${commit_${tag}}")
set(tags_here "")
foreach(name IN LISTS names)
    if(commit_${name} STREQUAL head)
        list(APPEND tags_here "${name}")
    endif()
endforeach()

foreach(t IN LISTS tags_here)
    if(NOT t STREQUAL tag)
        message(FATAL_ERROR "This commit has the tag ${t}, but VERSION says ${version}. "
                            "Make releases with release.bat / release.sh, which writes both")
    endif()
endforeach()

if(tag_commit STREQUAL head)
    set(result yes)
    message(STATUS "Release: tag ${tag} points to this commit (${head})")
elseif(tag_commit)
    set(result no)
    message(STATUS "Not a release: ${tag} is an earlier commit (${tag_commit}); this is ${head}")
else()
    set(result no)
    message(STATUS "Not a release: no tag ${tag} on the remote yet")
endif()
file(WRITE "${OUT}" "${result}")
