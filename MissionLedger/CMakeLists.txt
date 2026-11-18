# MissionLedger: GUI application (wxWidgets).
# Every source under src/ is part of this project: add a file by creating it in the folder.

file(GLOB_RECURSE GUI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

set(GUI_ENTRY main.cpp)

add_executable(MissionLedger WIN32 ${GUI_ENTRY} ${GUI_SOURCES})
ml_set_target_defaults(MissionLedger)

target_include_directories(MissionLedger PRIVATE src)
target_link_libraries(MissionLedger PRIVATE MissionLedgerCore wx::core wx::base)

if(WIN32)
    # Icon and version resources. The version goes to the resource compiler only (not to .cpp files),
    # so changing VERSION does not recompile everything.
    set(GUI_RESOURCES MissionLedger.rc resource.h resources/app.ico)
    target_sources(MissionLedger PRIVATE ${GUI_RESOURCES})
    set_source_files_properties(MissionLedger.rc PROPERTIES COMPILE_DEFINITIONS
        "ML_VERSION_MAJOR=${ML_VERSION_MAJOR};ML_VERSION_MINOR=${ML_VERSION_MINOR};ML_VERSION_PATCH=${ML_VERSION_PATCH};ML_VERSION_BUILD=${ML_BUILD_NUMBER}")
    source_group("Resources" FILES ${GUI_RESOURCES})
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${GUI_SOURCES})
