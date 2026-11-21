# Smoke tests: run the CLI on tests/fixtures/sample.ml (made-up data).
# Run with: ctest --preset windows-release   (or windows-debug)

set(SMOKE_FIXTURE "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/sample.ml")

foreach(case list_totals budget_totals export_settlement export_transactions missing_file new_existing edit_cycle usage_error json_list json_error dry_run backup shell_add shell_cancel)
    add_test(NAME cli_${case}
        COMMAND ${CMAKE_COMMAND}
            -DCASE=${case}
            -DCLI=$<TARGET_FILE:MissionLedgerCLI>
            -DFIXTURE=${SMOKE_FIXTURE}
            -DOUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/out/$<CONFIG>
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cli_smoke.cmake)
    set_tests_properties(cli_${case} PROPERTIES TIMEOUT 60 LABELS smoke)
endforeach()

# macOS Universal build: run the same cases on the Intel (x86_64) half of the CLI as well.
# The normal tests above run the build Mac's own architecture (arm64 on the Jenkins Mac);
# the x86_64 half runs through Rosetta 2. Without Rosetta the cases are reported as skipped.
if(APPLE AND "x86_64" IN_LIST CMAKE_OSX_ARCHITECTURES)
    foreach(case list_totals budget_totals export_settlement export_transactions missing_file new_existing edit_cycle usage_error json_list json_error dry_run backup shell_add shell_cancel)
        add_test(NAME cli_${case}_x86_64
            COMMAND ${CMAKE_COMMAND}
                -DCASE=${case}
                -DARCH=x86_64
                -DCLI=$<TARGET_FILE:MissionLedgerCLI>
                -DFIXTURE=${SMOKE_FIXTURE}
                -DOUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/out/$<CONFIG>/x86_64
                -P ${CMAKE_CURRENT_SOURCE_DIR}/cli_smoke.cmake)
        set_tests_properties(cli_${case}_x86_64 PROPERTIES TIMEOUT 60 LABELS "smoke;x86_64"
            SKIP_REGULAR_EXPRESSION "SMOKE_SKIPPED")
    endforeach()
endif()
