# Smoke tests: run the CLI on tests/fixtures/sample.ml (made-up data).
# Run with: ctest --preset windows-release   (or windows-debug)

set(SMOKE_FIXTURE "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/sample.ml")

foreach(case list_totals budget_totals export_settlement export_transactions missing_file)
    add_test(NAME cli_${case}
        COMMAND ${CMAKE_COMMAND}
            -DCASE=${case}
            -DCLI=$<TARGET_FILE:MissionLedgerCLI>
            -DFIXTURE=${SMOKE_FIXTURE}
            -DOUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/out/$<CONFIG>
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cli_smoke.cmake)
    set_tests_properties(cli_${case} PROPERTIES TIMEOUT 60 LABELS smoke)
endforeach()
