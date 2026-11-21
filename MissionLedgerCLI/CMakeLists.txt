# MissionLedgerCLI: command-line interface (no wxWidgets).
# Every source under src/ is part of this project: add a file by creating it in the folder.

file(GLOB_RECURSE CLI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

add_executable(MissionLedgerCLI main_cli.cpp ${CLI_SOURCES})
ml_set_target_defaults(MissionLedgerCLI)

target_include_directories(MissionLedgerCLI PRIVATE src)
# nlohmann/json (header only, third_party/nlohmann_json/VERSION.txt): --json output
target_include_directories(MissionLedgerCLI SYSTEM PRIVATE "${CMAKE_SOURCE_DIR}/third_party/nlohmann_json/include")
target_link_libraries(MissionLedgerCLI PRIVATE MissionLedgerCore)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${CLI_SOURCES})
