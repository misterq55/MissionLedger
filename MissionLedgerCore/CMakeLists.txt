# MissionLedgerCore: static library (model, controller, storage, export, interfaces). No wxWidgets.
# Every source under src/ is part of this project: add a file by creating it in the folder.

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

set(SQLITE_SOURCES
    "${PROJECT_SOURCE_DIR}/third_party/sqlite/sqlite3.c"
    "${PROJECT_SOURCE_DIR}/third_party/sqlite/sqlite3.h")

add_library(MissionLedgerCore STATIC ${CORE_SOURCES} ${SQLITE_SOURCES})
ml_set_target_defaults(MissionLedgerCore)

target_include_directories(MissionLedgerCore
    PUBLIC
        src                                         # GUI/CLI include Core headers from here
    PRIVATE
        # #include "sqlite3.h". Never add the repository root: on case-insensitive file systems (macOS)
        # the root VERSION file would be found for the C++ standard header <version>
        "${PROJECT_SOURCE_DIR}/third_party/sqlite"
        "${PROJECT_SOURCE_DIR}/third_party/pdfwriter")  # #include "PDFWriter/PDFWriter.h"

target_link_libraries(MissionLedgerCore PUBLIC PDFHummus::PDFWriter)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${CORE_SOURCES})
source_group("third_party/sqlite" FILES ${SQLITE_SOURCES})
