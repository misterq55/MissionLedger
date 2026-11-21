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
#     new_existing: non-zero, and the existing file is unchanged;
#     usage_error: exactly 2 (usage error), and the file is unchanged;
#     json_error: exactly 2, JSON error on stdout, file unchanged;
#     dry_run: 0, file unchanged and no .bak; backup: 0, .bak equals the file before the write;
#     edit_cycle: add/update/show/delete steps on a copy each succeed and print their line first),
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
    set(args export settlement "${FIXTURE}" --out "${pdf}")
    set(expected "")
elseif(CASE STREQUAL "export_transactions")
    set(pdf "${OUT_DIR}/transactions.pdf")
    set(args export transactions "${FIXTURE}" --out "${pdf}")
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
elseif(CASE STREQUAL "edit_cycle")
    # add → update → delete on a copy (steps below), then the totals must be the fixture's again
    set(work "${OUT_DIR}/edit_cycle.ml")
    file(MAKE_DIRECTORY "${OUT_DIR}")
    file(COPY_FILE "${FIXTURE}" "${work}")
    set(steps add update show delete)
    set(step_add_args add "${work}" --type expense --category 테스트 --item 항목 --amount 1000 --date 2026-02-01)
    set(step_add_expected "추가됨: ID 10 | 지출 | 테스트 / 항목 | 1,000원 | 2026-02-01")
    set(step_update_args update "${work}" --id 10 --amount 2,500 --desc 수정됨)
    set(step_update_expected "수정됨: ID 10 | 지출 | 테스트 / 항목 | 2,500원 | 2026-02-01")
    set(step_show_args show "${work}" --id 10)
    set(step_show_expected "설명: 수정됨")
    set(step_delete_args delete "${work}" --id 10 --yes)
    set(step_delete_expected "삭제됨: ID 10")
    set(args list "${work}")
    set(expected "총 10개의 트랜잭션" "총 수입: 4,800,000원" "총 지출: 4,309,750원" "잔액: 490,250원")
elseif(CASE STREQUAL "usage_error")
    # A bad value must be a usage error (exit code 2) and must not touch the file
    set(existing "${OUT_DIR}/usage_error.ml")
    file(MAKE_DIRECTORY "${OUT_DIR}")
    file(COPY_FILE "${FIXTURE}" "${existing}")
    file(SHA256 "${existing}" existing_hash)
    set(args add "${existing}" --type expense --category 테스트 --amount 12abc)
    set(expected "")
    set(expect_fail TRUE)
    set(expect_rc 2)
elseif(CASE STREQUAL "json_list")
    set(args list "${FIXTURE}" --json)
    set(expected [[{"ok":true,"command":"list","count":10,"summary":{"income":4800000,"expense":4309750,"balance":490250}]])
elseif(CASE STREQUAL "json_error")
    # With --json an error is JSON on stdout too (exit code 2 for a usage error)
    set(existing "${OUT_DIR}/json_error.ml")
    file(MAKE_DIRECTORY "${OUT_DIR}")
    file(COPY_FILE "${FIXTURE}" "${existing}")
    file(SHA256 "${existing}" existing_hash)
    set(args add "${existing}" --type expense --category 테스트 --amount 12abc --json)
    set(expected [[{"ok":false,"command":"add","error":{"code":"usage"]])
    set(expect_fail TRUE)
    set(expect_rc 2)
elseif(CASE STREQUAL "dry_run")
    # --dry-run: succeeds and prints the result, but neither the file nor a backup is written
    set(existing "${OUT_DIR}/dry_run.ml")
    file(MAKE_DIRECTORY "${OUT_DIR}")
    file(COPY_FILE "${FIXTURE}" "${existing}")
    file(REMOVE "${existing}.bak")
    file(SHA256 "${existing}" existing_hash)
    set(args add "${existing}" --type expense --category 테스트 --amount 1000 --dry-run)
    set(expected "(미리 보기, 저장 안 함) 추가됨: ID 10")
    set(expect_no_backup TRUE)
elseif(CASE STREQUAL "backup")
    # A write command leaves <file>.bak with the state before the write
    set(work "${OUT_DIR}/backup.ml")
    file(MAKE_DIRECTORY "${OUT_DIR}")
    file(COPY_FILE "${FIXTURE}" "${work}")
    file(REMOVE "${work}.bak")
    file(SHA256 "${work}" before_hash)
    set(args budget add "${work}" --type expense --category 테스트 --amount 1000 --json)
    set(expected [["backup":"]] [["budget":{"id":9,]])
    set(expect_backup TRUE)
elseif(CASE STREQUAL "shell_add")
    # Interactive mode on a copy: "add" asks every field in turn, previews, saves after "y"
    set(work "${OUT_DIR}/shell_add.ml")
    file(MAKE_DIRECTORY "${OUT_DIR}")
    file(COPY_FILE "${FIXTURE}" "${work}")
    set(args shell "${work}")
    # type 2 (expense), category, item (none), foreign? n, amount, date, description, receipt, save? y
    set(stdin_text "add\n2\n숙식\n\nn\n50000\n2026-02-01\n\n\ny\nlist\nexit\n")
    set(expected "추가됨: ID 10 | 지출 | 숙식 | 50,000원 | 2026-02-01" "총 11개의 트랜잭션")
elseif(CASE STREQUAL "shell_cancel")
    # Interactive mode: "취소" at a question cancels the command and leaves the file untouched
    set(existing "${OUT_DIR}/shell_cancel.ml")
    file(MAKE_DIRECTORY "${OUT_DIR}")
    file(COPY_FILE "${FIXTURE}" "${existing}")
    file(SHA256 "${existing}" existing_hash)
    set(args shell "${existing}")
    set(stdin_text "add\n2\n취소\nexit\n")
    set(expected "취소했습니다.")
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
if(DEFINED stdin_text)
    # Interactive cases: answers fed through stdin (the steps below keep the empty input)
    set(case_input "${OUT_DIR}/${CASE}_stdin.txt")
    file(WRITE "${case_input}" "${stdin_text}")
else()
    set(case_input "${empty_input}")
endif()

# Steps before the checked command (edit_cycle): each must succeed and print its expected line
foreach(step IN LISTS steps)
    execute_process(
        COMMAND ${runner} "${CLI}" ${step_${step}_args}
        INPUT_FILE "${empty_input}"
        OUTPUT_VARIABLE step_out
        ERROR_VARIABLE step_err
        RESULT_VARIABLE step_rc
        ENCODING UTF8
        TIMEOUT 30)
    message("[${step}] ${step_out}${step_err}")
    if(NOT step_rc EQUAL 0)
        message(FATAL_ERROR "FAILED (${CASE}): step ${step} exit code ${step_rc}")
    endif()
    string(FIND "${step_out}" "${step_${step}_expected}" pos)
    if(pos EQUAL -1)
        message(FATAL_ERROR "FAILED (${CASE}): step ${step} missing in output: ${step_${step}_expected}")
    endif()
endforeach()

execute_process(
    COMMAND ${runner} "${CLI}" ${args}
    INPUT_FILE "${case_input}"
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
    elseif(DEFINED expect_rc AND NOT rc EQUAL expect_rc)
        list(APPEND errors "exit code ${rc} (expected ${expect_rc})")
    endif()
    if(missing AND EXISTS "${missing}")
        list(APPEND errors "file was created: ${missing}")
    endif()
elseif(NOT rc EQUAL 0)
    list(APPEND errors "exit code ${rc} (expected 0)")
endif()
if(existing)
    file(SHA256 "${existing}" after_hash)
    if(NOT after_hash STREQUAL existing_hash)
        list(APPEND errors "file was changed: ${existing}")
    endif()
endif()
if(expect_no_backup AND EXISTS "${existing}.bak")
    list(APPEND errors "backup was written: ${existing}.bak")
endif()
if(expect_backup)
    if(NOT EXISTS "${work}.bak")
        list(APPEND errors "no backup: ${work}.bak")
    else()
        file(SHA256 "${work}.bak" backup_hash)
        file(SHA256 "${work}" after_hash)
        if(NOT backup_hash STREQUAL before_hash)
            list(APPEND errors "backup differs from the file before the write")
        endif()
        if(after_hash STREQUAL before_hash)
            list(APPEND errors "file was not changed by the write")
        endif()
    endif()
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
