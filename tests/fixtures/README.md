# Test Fixtures

## sample.ml

A small `.ml` file with **made-up data** (no personal information) for smoke tests and for trying the app on a new PC.
It was created with `MissionLedgerCLI`, so it uses exactly the format the app saves.

| Content | Purpose |
|---|---|
| 9 budgets (income 3, expense 6) in 6 categories | Settlement report: budget vs actual comparison |
| 10 transactions (income 3, expense 7), January 2026 | Both PDF reports |
| 4 expense transactions in PHP at 24.5 KRW/PHP | Exchange rate section, inline `(PHP 5,000 @ 24.5)` |
| Several items under the same category / item | Merged cells in the transaction list |
| One long description | Text wrapping and row height |
| Receipt numbers on most expenses, some empty | Optional column |

Expected totals: income 4,800,000원, expense 4,309,750원, balance 490,250원.

### Smoke test

```bash
ctest --preset windows-release   # after cmake --build --preset windows-release (or windows-debug for both)
```

Defined in [`tests/CMakeLists.txt`](../CMakeLists.txt), run by [`tests/cli_smoke.cmake`](../cli_smoke.cmake):

| Test | Command | Passes when |
|---|---|---|
| `cli_list_totals` | `list sample.ml` | exit 0; output has 10 transactions and the totals above |
| `cli_budget_totals` | `budget list sample.ml` | exit 0; output has income budget 5,000,000원, expense budget 4,800,000원 |
| `cli_export_settlement` | `export settlement sample.ml --out <out>` | exit 0; the PDF exists, is not empty, starts with `%PDF` and embeds the bundled fonts `NotoSansKR-Regular` and `NotoSansKR-Bold` |
| `cli_export_transactions` | `export transactions sample.ml --out <out>` | same |
| `cli_missing_file` | `list <path that does not exist>` | exit code is not 0 and no file is created there |
| `cli_new_existing` | `new <copy of sample.ml>` | exit code is not 0 and the existing file is unchanged (SHA-256) |
| `cli_edit_cycle` | on a copy: `add` → `update` → `show` → `delete --yes`, then `list` | each step exits 0 and prints its result line (new ID 10, updated amount, description); the final totals are the fixture's again |
| `cli_usage_error` | `add <copy> ... --amount 12abc` | exit code exactly 2 (usage error) and the file is unchanged |
| `cli_json_list` | `list sample.ml --json` | exit 0; the JSON starts with `ok`, `command`, `count` 10 and the totals |
| `cli_json_error` | `add <copy> ... --amount 12abc --json` | exit code exactly 2, a JSON error with `"code":"usage"` on stdout, the file is unchanged |
| `cli_dry_run` | `add <copy> ... --dry-run` | exit 0, prints the preview line, the file is unchanged and no `.bak` is written |
| `cli_backup` | `budget add <copy> ... --json` | exit 0; `<copy>.bak` equals the file before the write and the file itself changed |
| `cli_shell_add` | `shell <copy>` with answers on stdin (`add`, every field, `y`, then `list`) | exit 0; prints the added transaction (ID 10, 50,000원) and 11 transactions |
| `cli_shell_cancel` | `shell <copy>` with `add`, `2`, `취소` on stdin | exit 0; prints "취소했습니다." and the file is unchanged |
| `cli_*_x86_64` (macOS only) | the fourteen above, run as `arch -x86_64 mledger ...` | the same; tests the Intel half of the Universal build through Rosetta 2. Reported as **skipped** (not passed) on a Mac without Rosetta. Output in `out/<Config>/x86_64/` |

PDFs go to `build/<windows|macos>/tests/out/<Config>/`. Apart from the embedded font names, PDF contents and layout are not checked. The expected values live in `cli_smoke.cmake`: update them there and in this file if `sample.ml` is regenerated with different data.

### Regenerating

The file is committed as is. To rebuild it from scratch (for example after a schema change), run from the repository root with a built CLI:

```bash
CLI=./build/windows/bin/Release/mledger.exe   # CMake windows preset output
F=tests/fixtures/sample.ml
rm -f $F && $CLI new $F

b() { $CLI budget add --file $F "$@"; }
b --type income  --category "참가비" --item "팀원 참가비" --amount 3000000
b --type income  --category "후원금" --item "교회 후원"   --amount 1500000
b --type income  --category "후원금" --item "개인 후원"   --amount 500000
b --type expense --category "교통"   --item "항공권"      --amount 2400000
b --type expense --category "교통"   --item "현지 교통"   --amount 200000
b --type expense --category "숙식"   --item "숙박"        --amount 800000
b --type expense --category "숙식"   --item "식비"        --amount 700000
b --type expense --category "사역"   --item "사역 물품"   --amount 500000
b --type expense --category "기타"   --item "여행자 보험" --amount 200000

t() { $CLI add --file $F "$@"; }
t --type income  --category "참가비" --item "팀원 참가비" --amount 3000000 --date 2026-01-05 --desc "팀원 6명 x 500,000원"
t --type income  --category "후원금" --item "교회 후원"   --amount 1500000 --date 2026-01-07
t --type income  --category "후원금" --item "개인 후원"   --amount 300000  --date 2026-01-08 --desc "개인 후원 3건 합계"
t --type expense --category "교통"   --item "항공권"      --amount 2350000 --date 2026-01-09 --receipt "R-001" --desc "인천-마닐라 왕복 6인"
t --type expense --category "교통"   --item "현지 교통"   --amount 122500  --date 2026-01-12 --receipt "R-002" --desc "공항-숙소 승합차 대절" --currency PHP --original 5000 --rate 24.5
t --type expense --category "숙식"   --item "숙박"        --amount 735000  --date 2026-01-12 --receipt "R-003" --desc "게스트하우스 5박" --currency PHP --original 30000 --rate 24.5
t --type expense --category "숙식"   --item "식비"        --amount 294000  --date 2026-01-13 --receipt "R-004" --desc "현지 식당 및 시장 장보기 (1~3일차)" --currency PHP --original 12000 --rate 24.5
t --type expense --category "숙식"   --item "식비"        --amount 208250  --date 2026-01-15 --receipt "R-005" --desc "현지 식당 및 시장 장보기 (4~5일차)" --currency PHP --original 8500 --rate 24.5
t --type expense --category "사역"   --item "사역 물품"   --amount 420000  --date 2026-01-10 --receipt "R-006" --desc "어린이 프로그램용 미술 재료, 학용품, 간식 준비 및 현지 전달용 포장 비용 포함 (긴 설명 줄바꿈 확인용)"
t --type expense --category "기타"   --item "여행자 보험" --amount 180000  --date 2026-01-09 --receipt "R-007"
```

Keep the data fictional. Real `.ml` files belong in the repository root, which is gitignored.
