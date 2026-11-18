# MissionLedger - root CMake build
#
# The source folders and these CMakeLists.txt files are the source of truth.
# IDE projects (Visual Studio .sln, Xcode) are generated into build/ and never committed.
#   Windows: GenerateProjectFiles.bat  (= cmake --preset windows)
# Details: docs/CI_PLAN.md §3

cmake_minimum_required(VERSION 3.25)   # workflow presets need 3.25; PDF-Writer needs 3.19

# ----- Version: the root VERSION file is the single source of truth (docs/VERSIONING.md) -----
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" ML_VERSION)
string(STRIP "${ML_VERSION}" ML_VERSION)
if(NOT ML_VERSION MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
    message(FATAL_ERROR "VERSION must be MAJOR.MINOR.PATCH (e.g. 1.0.0), got '${ML_VERSION}'")
endif()
set(ML_VERSION_MAJOR ${CMAKE_MATCH_1})
set(ML_VERSION_MINOR ${CMAKE_MATCH_2})
set(ML_VERSION_PATCH ${CMAKE_MATCH_3})
set(ML_BUILD_NUMBER 0 CACHE STRING "Build number (4th version field); CI passes -DML_BUILD_NUMBER=N")

project(MissionLedger VERSION ${ML_VERSION} LANGUAGES C CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Executables go to build/<preset>/bin/<Config>/ (multi-config generators append the config)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# ----- Third-party libraries (all from the repository, nothing installed on the machine) -----

# PDF-Writer (PDFHummus): bundled zlib/freetype/png/jpeg/tiff.
# OpenSSL OFF so the result does not depend on whether a machine has OpenSSL installed.
set(PDFHUMMUS_NO_OPENSSL ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/pdfwriter EXCLUDE_FROM_ALL)
foreach(t PDFWriter FreeType LibAesgm LibJpeg LibPng LibTiff Zlib)
    if(TARGET ${t})
        set_target_properties(${t} PROPERTIES FOLDER "ThirdParty/PDFWriter")
    endif()
endforeach()

# wxWidgets 3.2.8 (vendored, see third_party/wxWidgets/VERSION.txt). Static, base + core only.
set(wxBUILD_SHARED OFF CACHE BOOL "" FORCE)
set(wxBUILD_MONOLITHIC OFF CACHE BOOL "" FORCE)
set(wxBUILD_INSTALL OFF CACHE BOOL "" FORCE)       # install needs files that are not vendored
set(wxBUILD_LOCALES OFF CACHE STRING "" FORCE)     # locale/ is not vendored
# XRC must stay OFF (wxrc lives in utils/, not vendored). HTML OFF also needs WXHTML_HELP OFF.
foreach(opt XRC HTML WXHTML_HELP AUI STC WEBVIEW PROPGRID RIBBON RICHTEXT MEDIACTRL OPENGL)
    set(wxUSE_${opt} OFF CACHE BOOL "" FORCE)
endforeach()
add_subdirectory(third_party/wxWidgets EXCLUDE_FROM_ALL)

# ----- Common settings for our own targets -----
function(ml_set_target_defaults target)
    target_compile_features(${target} PRIVATE cxx_std_17)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    if(MSVC)
        # /utf-8: sources are BOM-less UTF-8 (Korean literals); without it MSVC on Korean Windows reads CP949
        target_compile_options(${target} PRIVATE /utf-8)
        target_compile_definitions(${target} PRIVATE UNICODE _UNICODE)
        # Release: keep PDBs (crash analysis) and the size/speed options of the former vcxproj files
        target_compile_options(${target} PRIVATE $<$<CONFIG:Release>:/Zi /Gy /Oi>)
        get_target_property(type ${target} TYPE)
        if(NOT type STREQUAL "STATIC_LIBRARY")
            target_link_options(${target} PRIVATE $<$<CONFIG:Release>:/DEBUG /OPT:REF /OPT:ICF>)
        endif()
    endif()
endfunction()

# ----- Projects (one project = one folder; each collects the sources under its own folder) -----
add_subdirectory(MissionLedgerCore)
add_subdirectory(MissionLedger)
add_subdirectory(MissionLedgerCLI)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MissionLedger)
