# MissionLedger: GUI application (wxWidgets).
# Every source under src/ is part of this project: add a file by creating it in the folder.

file(GLOB_RECURSE GUI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

set(GUI_ENTRY main.cpp)

# WIN32: no console window on Windows. MACOSX_BUNDLE: MissionLedger.app on macOS (each ignored elsewhere)
add_executable(MissionLedger WIN32 MACOSX_BUNDLE ${GUI_ENTRY} ${GUI_SOURCES})
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
elseif(APPLE)
    # App bundle: Info.plist from a template, icon and PDF fonts in Contents/Resources/
    # (MLPDFDocument.cpp looks in ../Resources/fonts/ next to Contents/MacOS/).
    # resources/app.icns holds the 32 and 256 px PNGs of resources/app.ico ("icp5" and "ic08" entries);
    # regenerate it from app.ico if the icon changes
    file(GLOB GUI_BUNDLE_FONTS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/third_party/fonts/NotoSansKR/*")
    set(GUI_RESOURCES resources/app.icns resources/Info.plist.in)
    target_sources(MissionLedger PRIVATE resources/app.icns ${GUI_BUNDLE_FONTS})
    set_source_files_properties(resources/app.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_source_files_properties(${GUI_BUNDLE_FONTS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/fonts)
    set_target_properties(MissionLedger PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME "MissionLedger"
        MACOSX_BUNDLE_GUI_IDENTIFIER "io.github.misterq55.MissionLedger"
        MACOSX_BUNDLE_ICON_FILE "app.icns"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${ML_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${ML_VERSION}.${ML_BUILD_NUMBER}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (C) 2025 MissionLedger Project"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "io.github.misterq55.MissionLedger"
        # Unsigned for now (docs/CI_PLAN.md §0): ad-hoc signature, which Apple Silicon needs to run at all
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-")
    source_group("Resources" FILES ${GUI_RESOURCES})

    # CLI inside the bundle: MissionLedger.app/Contents/MacOS/mledger (docs/CLI_DESIGN.md §2). It finds the
    # PDF fonts in ../Resources/fonts/ like the app. Copying it changes the bundle after Xcode may have signed it,
    # so sign inside-out again (ad-hoc "-", like the app): mledger first, then the whole app. With a real
    # certificate later, the same two commands take its name instead of "-"
    add_dependencies(MissionLedger MissionLedgerCLI)
    add_custom_command(TARGET MissionLedger POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:MissionLedgerCLI>" "$<TARGET_BUNDLE_CONTENT_DIR:MissionLedger>/MacOS/mledger"
        COMMAND codesign --force --sign - "$<TARGET_BUNDLE_CONTENT_DIR:MissionLedger>/MacOS/mledger"
        COMMAND codesign --force --sign - "$<TARGET_BUNDLE_DIR:MissionLedger>"
        COMMENT "Copying mledger into MissionLedger.app and signing (ad-hoc)"
        VERBATIM)
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${GUI_SOURCES})
