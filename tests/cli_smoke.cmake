# CLI smoke test runner (called by CTest, see tests/CMakeLists.txt)
#
#   cmake -DCASE=<case> -DCLI=<MissionLedgerCLI.exe> -DFIXTURE=<sample.ml> -DOUT_DIR=<dir> [-DARCH=x86_64] -P cli_smoke.cmake
#
# ARCH (macOS only): run that architecture of a Universal binary with "arch -<ARCH>" (x86_64 on an
# Apple Silicon Mac goes through Rosetta 2). If the Mac cannot run it, print SMOKE_SKIPPED: CTest
# then reports the test as skipped (SKIP_REGULAR_EXPRESSION in tests/CMakeLists.txt), not passed.
#
# Each case runs the CLI once and fails unless
#   - the exit code is 0 (missing_file: non-zero, and no file is created;
#     new_existing: non-zero, and the existing file is unchanged),
#   - every expected line appears in the output (totals from tests/fixtures/README.md),
#   - the PDF (export cases) exists, is not empty and starts with "%PDF".
# PDF contents/layout are not checked: this only tells whether the build still works.
# Expected strings live here (UTF-8 file) instead of on the command line to avoid code page issues.

foreach(var CASE CLI FIXTURE OUT_DIR)
    if(NOT DEFINED ${var})
        message(FATAL_ERROR "cli_smoke.cmake: -D${var}=... is required")
    endif()
endforeach()

set(pdf "")
set(expect_fail FALSE)
if(CASE STREQUAL "list_totals")
    set(args list "${FIXTURE}")
    set(expected "총 10개의 트랜잭션" "총 수입: 4,800,000원" "총 지출: 4,309,750원" "잔액: 490,250원")
elseif(CASE STREQUAL "budget_totals")
    set(args budget list "${FIXTURE}")
    set(expected "수입 예산: 5,000,000원" "지출 예산: 4,800,000원")
elseif(CASE STREQUAL "export_settlement")
    set(pdf "${OUT_DIR}/settlement.pdf")
    set(args export-settlement "${FIXTURE}" "${pdf}")
    set(expected "")
elseif(CASE STREQUAL "export_transactions")
    set(pdf "${OUT_DIR}/transactions.pdf")
    set(args export-transactions "${FIXTURE}" "${pdf}")
    set(expected "")
elseif(CASE STREQUAL "missing_file")
    # Opening a path that does not exist must fail and must not create an empty .ml there
    set(missing "${OUT_DIR}/does_not_exist.ml")
    file(REMOVE "${missing}")
    set(args list "${missing}")
    set(expected "")
    set(expect_fail TRUE)
elseif(CASE STREQUAL "new_existing")
    # "new" on an existing file must fail and leave the file untouched (it used to wipe all data)
    set(existing "${OUT_DIR}/new_existing.ml")
    file(MAKE_DIRECTORY "${OUT_DIR}")
    file(COPY_FILE "${FIXTURE}" "${existing}")
    file(SHA256 "${existing}" existing_hash)
    set(args new "${existing}")
    set(expected "")
    set(expect_fail TRUE)
else()
    message(FATAL_ERROR "cli_smoke.cmake: unknown CASE '${CASE}'")
endif()

set(runner "")
if(DEFINED ARCH)
    execute_process(COMMAND arch -${ARCH} /usr/bin/true RESULT_VARIABLE arch_rc OUTPUT_QUIET ERROR_QUIET)
    if(NOT arch_rc EQUAL 0)
        message("SMOKE_SKIPPED: this Mac cannot run ${ARCH} code (for x86_64 on Apple Silicon: install Rosetta 2 with "
                "'softwareupdate --install-rosetta --agree-to-license')")
        return()
    endif()
    set(runner arch -${ARCH})
endif()

file(MAKE_DIRECTORY "${OUT_DIR}")
if(pdf)
    file(REMOVE "${pdf}")   # a PDF left over from an earlier run must not make the test pass
endif()

# Empty stdin: the CLI never waits for input, even if it falls into interactive mode
set(empty_input "${OUT_DIR}/empty_stdin.txt")
file(WRITE "${empty_input}" "")

execute_process(
    COMMAND ${runner} "${CLI}" ${args}
    INPUT_FILE "${empty_input}"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE rc
    ENCODING UTF8   # the CLI writes UTF-8; the default (console code page, CP949 on Korean Windows) garbles it
    TIMEOUT 30)

message("${out}${err}")

set(errors "")
if(expect_fail)
    if(rc EQUAL 0)
        list(APPEND errors "exit code 0 (expected an error)")
    endif()
    if(missing AND EXISTS "${missing}")
        list(APPEND errors "file was created: ${missing}")
    endif()
    if(existing)
        file(SHA256 "${existing}" after_hash)
        if(NOT after_hash STREQUAL existing_hash)
            list(APPEND errors "existing file was changed: ${existing}")
        endif()
    endif()
elseif(NOT rc EQUAL 0)
    list(APPEND errors "exit code ${rc} (expected 0)")
endif()
foreach(line IN LISTS expected)
    string(FIND "${out}" "${line}" pos)
    if(pos EQUAL -1)
        list(APPEND errors "missing in output: ${line}")
    endif()
endforeach()
if(pdf)
    if(NOT EXISTS "${pdf}")
        list(APPEND errors "PDF not created: ${pdf}")
    else()
        file(SIZE "${pdf}" size)
        file(READ "${pdf}" header LIMIT 4 HEX)   # HEX: text-mode reads are not byte-exact on Windows
        if(size EQUAL 0 OR NOT header STREQUAL "25504446")   # "%PDF"
            list(APPEND errors "not a PDF or empty (${size} bytes): ${pdf}")
        endif()
        # The bundled fonts (third_party/fonts/NotoSansKR, copied to bin/<Config>/fonts/) must be the ones
        # embedded: catches a missing copy step or a broken font lookup (e.g. the executable path on macOS)
        file(STRINGS "${pdf}" font_lines REGEX "/BaseFont")
        foreach(face NotoSansKR-Regular NotoSansKR-Bold)
            string(FIND "${font_lines}" "${face}" pos)
            if(pos EQUAL -1)
                list(APPEND errors "font ${face} not embedded (found: ${font_lines})")
            endif()
        endforeach()
    endif()
endif()

if(errors)
    list(JOIN errors "\n  " msg)
    message(FATAL_ERROR "FAILED (${CASE}):\n  ${msg}")
endif()
message(STATUS "OK (${CASE})")
