# CLI 설계안 (`mledger`)

작성 2026-11-21. 상태: **1~3단계, 7단계 구현됨 (2026-11-21~22)**, 나머지는 설계안. 구현하면서 이 문서를 갱신한다.

## 0. 목적과 원칙

- **목적**: CLI를 사람과 AI(Claude Code 등) 모두가 쓰는 두 번째 입구로 만든다. AI는 CLI 참조 문서(`CLI.md`)를 읽고 명령을 실행해 거래 입력, 조회, PDF 내보내기를 한다 (예: "영수증 보고 이번 주 지출 입력해 줘")
- **CLI는 하나**: 사람용과 AI용을 따로 만들지 않는다. 기본 출력은 사람이 읽는 표, `--json`이면 기계가 읽는 JSON
- **명령 체계는 하나**: 인수 모드와 대화형이 같은 명령, 같은 옵션, 같은 처리 코드를 쓴다. 도움말과 문서도 한 벌
- **인수 모드는 묻지 않는다**: 빠진 값이 있으면 오류 (AI, 스크립트가 멈추지 않게). **대화형은 빠진 값을 차례대로 묻는다**
- **데이터를 조용히 잃지 않는다**: 덮어쓰기 전 확인, 백업, GUI와의 충돌 감지

## 1. 재작업 전 상태 (2026-11-21 코드 확인, 2단계에서 대부분 바뀜)

`MissionLedgerCLI/src/module/mvc/view/MLCLIView.cpp`, `MissionLedgerCLI/main_cli.cpp`

| 현재 명령 | 비고 |
|---|---|
| `help` | 인수 모드 도움말 (`printUsage`) |
| `list [파일]` | 파일을 위치 인수로. 파일 없이 실행하면 "0개"를 출력하고 성공(0) |
| `add --file <파일> --type --amount --category [...]` | 파일을 `--file`로 |
| `open <파일>` | 파일을 열고 대화형 모드로 |
| `new [파일]` | ~~기존 파일이면 모든 데이터 삭제~~ → 2026-11-21 수정: 기존 파일이면 거부 |
| `budget list [파일]` | 위치 인수 |
| `budget add/update/delete --file <파일> ...` | `--file`. `update --type 잘못된값`은 오류 없이 무시 |
| `export-settlement <입력> <출력>` | 출력 필수 |
| `export-transactions <입력> [출력]` | 출력 생략 시 `<입력>.pdf` |
| (인수 없이 실행) | 대화형: `new`, `open`, `save`, `add`(질문형), `list`, `budget-add`, `budget-list`, `help`, `exit`. 내보내기, 수정, 삭제 없음. `open`이 `std::cin >>`로 읽어 띄어쓰기 있는 파일 이름을 못 엶 |

그 밖에:
- 거래 수정/삭제 명령 없음 (Controller에는 `UpdateTransaction`, `DeleteTransaction`이 있음)
- 숫자 파싱이 느슨함: `std::stoll`/`std::stod`가 앞부분만 읽어 `--amount 12abc`를 12로 받음
- 도움말이 두 벌 (`printUsage`, `printHelp`)이고 명령 목록이 다름
- CLI는 설치 파일(Windows, `.dmg`)에 들어 있지 않음: 스모크 테스트와 개발용으로만 쓰임
- 저장 방식: `FMLModel::SaveFile`이 거래 전체 삭제 후 메모리 내용을 다시 쓰고, 예산도 전체 삭제 후 다시 씀 (`MLModel.cpp`, `MLSQLiteStorage::SaveAllTransactions`). GUI와 CLI가 같은 파일을 쓰면 늦게 저장한 쪽이 이긴다 (§6)

## 2. 이름과 배포

- 실행 파일 이름: **`mledger`** (Windows `mledger.exe`). `ml`은 Windows에서 MASM(`ml.exe`, Visual Studio 개발자 프롬프트)과, 연구용 서버에서 Lmod의 `ml`과 겹쳐서 쓰지 않음
- CMake: `MissionLedgerCLI` 타깃은 그대로 두고 `OUTPUT_NAME mledger` (타깃 이름 변경은 선택)
- 설치 파일에 포함
  - Windows: `{app}\mledger.exe`. 설치 화면 체크박스 "명령줄에서 사용 (PATH에 추가)", 제거 시 PATH에서도 제거
  - macOS: 앱 번들 안 (`MissionLedger.app/Contents/MacOS/mledger`, 글꼴은 `Contents/Resources/fonts/`를 이미 찾음). PATH는 문서로 안내: `sudo ln -s /Applications/MissionLedger.app/Contents/MacOS/mledger /usr/local/bin/mledger`. 앱 메뉴 "명령줄 도구 설치..."는 나중 선택
- 이름 변경 시 고칠 곳: `MissionLedgerCLI/CMakeLists.txt`, 루트 `CMakeLists.txt`(fonts 복사), `tests/CMakeLists.txt`, `tests/cli_smoke.cmake` 주석, `README.md`, `docs/CI_PLAN.md`, `docs/INSTALLER.md`, `tests/fixtures/README.md`. Jenkinsfile은 CLI 이름을 직접 쓰지 않음 (2026-11-21 확인)

## 3. 명령 체계

**구현 (2단계, 2026-11-21)**: `MissionLedgerCLI/src/module/mvc/view/MLCLIViewCommands.cpp`의 `commandSpecs()` 표 하나에 명령, 위치 인수, 옵션, 처리 함수, 예를 적고 파싱(`MLCLIArgs`), 필수 옵션 검사, 도움말(`help`, `help <명령>`, `<명령> --help`)이 모두 이 표를 쓴다. 실행 파일 이름은 아직 `MissionLedgerCLI` (도움말의 이름은 `kProgram` 한곳). 아래 표에서 구현과 다른 점: `--item` 필터는 없음(`--search`가 항목/설명/영수증 번호를 찾음), `--version`은 아직 없음. 외화 거래의 `--amount`는 생략 가능하고, 주면 계산값과 1원 이내여야 함 (옛 스크립트 호환: 옛 `add`는 `--amount`가 필수였고 외화 옵션이 있으면 계산값으로 덮어씀)

**문법**: `mledger <명령> [하위명령] <파일.ml> [옵션]`. 파일은 항상 명령 바로 뒤의 위치 인수 (`--file` 없앰)

| 명령 | 내용 | 현재 대응 |
|---|---|---|
| `mledger help [명령]` | 전체 또는 명령별 도움말 | `help` |
| `mledger new <파일>` | 새 파일. 이미 있으면 거부 | `new` |
| `mledger info <파일>` | 거래/예산 건수, 수입/지출/잔액 요약 | (새로) |
| `mledger list <파일> [--type income\|expense] [--category C] [--item I] [--from YYYY-MM-DD] [--to YYYY-MM-DD]` | 거래 목록 (GUI 필터와 같은 조건, `GetFilteredTransactionData`) | `list` |
| `mledger show <파일> --id N` | 거래 하나의 모든 필드 | (새로) |
| `mledger add <파일> --type --category --amount [--item --desc --receipt --date --currency --original --rate]` | 거래 추가. 결과로 새 ID 출력 | `add --file` |
| `mledger update <파일> --id N [바꿀 필드들]` | 거래 수정 (준 필드만) | (새로) |
| `mledger delete <파일> --id N` | 거래 삭제 (사람이 터미널에서 쓰면 확인, `--yes`로 생략) | (새로) |
| `mledger budget list <파일>` | 예산 목록 | `budget list` |
| `mledger budget add <파일> --type --category --amount [--item]` | | `budget add --file` |
| `mledger budget update <파일> --id N [...]` | | `budget update --file` |
| `mledger budget delete <파일> --id N` | | `budget delete --file` |
| `mledger export settlement <파일> [--out 경로] [--no-subtitle]` | 결산 보고서 PDF. `--out` 생략 시 `<파일이름>-결산보고서.pdf` | `export-settlement` |
| `mledger export transactions <파일> [--out 경로] [--no-subtitle]` | 거래 내역서 PDF. 생략 시 `<파일이름>-거래내역서.pdf` | `export-transactions` |
| `mledger` 또는 `mledger shell [파일]` | 대화형 (§5) | 인수 없이 / `open` |

(출력 파일 이름 기본값: `.ml`과 같은 폴더, 위 이름으로 확정)

### 공통 옵션

| 옵션 | 내용 |
|---|---|
| `--json` | 결과를 JSON 한 줄로 (목록, 추가된 ID, 오류 포함). 금액은 정수, 날짜는 ISO 문자열. 형식은 아래 "JSON 형식" |
| `--yes` | 확인 질문 생략 (삭제, 덮어쓰기) |
| `--dry-run` | 쓰기 명령을 실제로 저장하지 않고 결과만 출력 |
| `--help` | 해당 명령 도움말 |
| `--version` | 버전 (`VERSION` 파일 값) |

### JSON 형식 (3단계, 2026-11-21)

`nlohmann::ordered_json` (third_party/nlohmann_json)으로 한 줄 출력, 키 순서는 아래와 같다. 성공도 오류도 stdout, 종료 코드는 그대로 (0/1/2)

```json
{"ok":true,"command":"add","dryRun":false,"backup":"data.ml.bak","transaction":{"id":10,"type":"expense","category":"숙식","item":"식비","description":"","amount":50000,"date":"2026-01-13","receipt":"","exchange":{"currency":"PHP","original":2000.0,"rate":25.0}}}
{"ok":true,"command":"list","count":5,"summary":{"income":0,"expense":1779750,"balance":-1779750},"transactions":[...]}
{"ok":false,"command":"add","error":{"code":"usage","message":"잘못된 금액입니다: 12abc ..."}}
```

- 거래: `id, type(income|expense), category, item, description, amount, date, receipt, exchange`(원화 거래는 `null`)
- 예산: `id, type, category, item, amount, notes`
- 쓰기 명령: `dryRun`, `backup`(경로 또는 `null`), 결과 `transaction`/`budget`/`deleted`
- `info`: `file, transactions{count,income,expense}, summary{income,expense,balance}, period{from,to}|null, budgets{count,income,expense}, categories{income[],expense[]}`
- `export`: `output`, `new`: `file`
- 오류 `code`: `usage`(2) / `failed`(1). 명령을 못 찾은 오류는 `"command": null`
- `--json`이면 삭제 확인 질문을 하지 않음: `--yes`가 없으면 사용법 오류. `help`는 `--json`이어도 글자 도움말

### 값 규칙

- `--type`: `income` / `expense` (별칭 `i`/`e`, `수입`/`지출`). 잘못된 값은 항상 오류 (지금 `budget update`는 무시함)
- 숫자: 문자열 전체가 숫자여야 함 (`12abc` 오류). 금액은 정수(원), 외화와 환율은 소수
- 날짜: `YYYY-MM-DD` (생략 시 오늘)
- 외화: `--currency --original --rate` 셋 다 주면 원화 금액을 계산 (`FMLCurrency::ConvertToKRW`, 원 단위 반올림). 일부만 주면 오류
- 알 수 없는 옵션, 값 없는 옵션, 중복 옵션: 오류

### 출력과 종료 코드

- 종료 코드: **0** 성공, **1** 실행 실패(파일 없음, ID 없음, 저장 실패), **2** 사용법 오류(옵션 누락/잘못된 값)
- 오류 메시지는 stderr, 결과는 stdout. 오류 메시지에 무엇이 왜 틀렸는지와 해당 명령의 사용법
- 쓰기 명령은 결과를 한 줄로 확인 (예: `추가됨: ID 11, 지출 / 숙식 / 식비 / 50,000원 / 2026-11-21`)

## 4. 안전장치

- `new`: 기존 파일이면 거부 (✅ 2026-11-21 수정, 스모크 테스트 `cli_new_existing`)
- 쓰기 전 백업 (✅ 3단계): 쓰기 명령이 저장하기 직전에 원본을 `<파일>.bak`(예: `data.ml.bak`, 같은 폴더)으로 복사. 직전 상태 1개, 다음 쓰기 때 덮어씀. 복사에 실패하면 저장하지 않고 1로 끝남. `--dry-run`과 `new`는 백업 없음. GUI 저장에는 아직 없음 (7단계에서 판단)
- `--dry-run`으로 미리 보기
- 삭제는 사람이 터미널에서 쓸 때 확인 질문, AI/스크립트는 `--yes` 필요 (터미널이 아니면 `--yes` 없이 오류)

## 5. 대화형

- 시작: `mledger` (파일 없이) 또는 `mledger shell <파일>`
- **같은 명령을 그대로 받는다**: 프롬프트에서 `add --type expense ...`, `budget list`, `export settlement` 등. 열린 파일이 있으면 파일 인수 생략
- 대화형 전용: `open <파일>`, `close`, `exit`/`quit`, `help`. 변경은 명령마다 바로 저장 (지금의 `save` 명령 없앰)
- **빠진 옵션은 차례대로 묻는다** (인수 모드에서는 오류). 이미 있는 카테고리와 항목은 번호로 고르게 하고 새 이름도 입력 가능. Enter = 기본값. 마지막에 요약을 보여주고 확인
- 따옴표로 띄어쓰기 있는 인수 (`open "2026 필리핀.ml"`)
- 입력이 끝나면(EOF, 파이프) 종료 (지금처럼 무한 루프 없이)

예:

```
> open "2026 필리핀 단기선교.ml"
열림: 2026 필리핀 단기선교.ml (거래 10건, 예산 9건)
> add
유형  1) 수입  2) 지출 : 2
카테고리  1) 교통  2) 기타  3) 사역  4) 숙식  (새 이름 입력 가능) : 4
항목  1) 숙박  2) 식비  (새 이름 입력 가능) : 2
금액 : 50000
날짜 [2026-11-21] :
설명 (선택) : 현지 시장
영수증 번호 (선택) :
외화 거래? [N] :
→ 지출 / 숙식 / 식비 / 50,000원 / 2026-11-21  저장할까요? [Y/n]
```

## 6. GUI와 같은 파일을 쓸 때

**문제** (코드 확인): GUI는 파일을 메모리에 읽어 편집하고, 저장할 때 전체를 지우고 다시 쓴다. GUI로 파일을 연 사이 CLI가 바꾼 내용은 GUI에서 저장하는 순간 사라진다. 지금은 아무 경고도 없다

**방식: 외부 변경 감지 + 다시 불러오기** (VS Code의 "디스크에서 변경됨", Unreal 에디터의 외부 변경 에셋 다시 불러오기와 같은 방식)

- **감지 (Core)**: SQLite `PRAGMA data_version` (다른 연결/프로세스가 커밋했을 때만 바뀜, 자기 쓰기로는 안 바뀜). GUI는 파일을 연 뒤 연결을 유지하므로 그대로 쓸 수 있음. 클라우드 동기화 폴더(Drive, OneDrive)처럼 파일이 통째로 교체되는 경우를 위해 파일 수정 시각/크기 비교도 같이. 저장소 인터페이스에 함수 1개, Model/Controller에 `HasExternalChanges()`, `Reload()`
- **GUI**: 창 활성화(`wxEVT_ACTIVATE`), 타이머(약 2초), 저장 직전에 확인
  - 저장 안 한 변경이 없으면: 자동으로 다시 불러옴 (기존 `OpenFile` → `OnDataLoaded` 경로)
  - 있으면: "파일이 외부에서 변경되었습니다: 다시 불러오기 / 내 변경으로 덮어쓰기 / 취소"
  - 저장 직전에 발견하면 묻지 않고 덮어쓰지 않음
  - 질문 창이 떠 있는 동안 중복 질문 방지
- **SQLite 대기**: 동시에 쓰는 순간의 BUSY 실패를 막기 위해 `busy_timeout` 설정 (현재 설정 여부 확인 필요)
- **잠금 표시 (선택)**: GUI가 `<파일>.ml.lock`에 "사용 중"(프로세스 ID)을 남기고, CLI는 이를 보면 경고하고 `--yes`가 있어야 진행. 비정상 종료로 남은 잠금은 프로세스가 살아 있는지 확인해 무시
- 결과: GUI를 켜 둔 채 AI에게 시키면 GUI 화면에 바로 반영되고, 사람이 동시에 편집하던 경우에도 데이터가 조용히 사라지지 않음

**구현 (7단계, 2026-11-22)**
- 저장소 (`IMLStorageProvider`): `SaveAll`(거래+예산 전체를 `BEGIN IMMEDIATE` 트랜잭션 하나로, 실패하면 그대로), `LoadAll`(읽기 트랜잭션 하나로 두 테이블), `GetChangeToken`(`PRAGMA data_version` + 경로의 수정 시각 + 크기). `Initialize`에서 `sqlite3_busy_timeout` 5초. 전에는 거래 저장, 예산 전체 삭제, 예산 하나씩 추가가 따로 커밋되어 다른 프로그램이 반쯤 쓴 상태(예산이 빈 상태)를 읽을 수 있었음
- Model/Controller: 불러오기·저장 직후 표식을 기억하고 `HasExternalChanges()`로 비교, `ReloadFile()`
- GUI (`MLMainFrame.cpp` "외부 변경 감지"): 2초 타이머 + 창 활성화(`CallAfter`). 모달 창(OS 기본 창 포함, `wxModalDialogHook`으로 셈)이 떠 있으면 건너뜀
  - 저장 안 한 변경 없음 → 묻지 않고 다시 불러옴 (창이 뒤에 있어도)
  - 있음 → 창이 앞에 있을 때 한 번만 "다시 불러오기 / 나중에" (다시 불러오거나 저장하면 초기화)
  - 저장 직전에 외부 변경이 있으면 "덮어쓰기 / 다시 불러오기 / 취소" (나중에를 골랐어도 다시 물음). 닫기·새 파일·열기 때의 "저장하시겠습니까?"도 같은 저장 경로
  - 다시 불러오기 실패(파일이 지워짐 등) → 화면 내용 유지, 저장하면 다시 씀. 선택했던 거래가 없어졌으면 선택 해제
- 확인 (2026-11-22, Windows): Python sqlite3로 자기 쓰기 뒤 표식이 3초 뒤에도 그대로인지(NTFS 수정 시각이 늦게 바뀌지 않음), 다른 프로세스 쓰기로 `data_version`과 시각이 바뀌는지. GUI를 연 채 CLI `add` → 2초 안에 GUI 카테고리 필터에 새 카테고리. GUI에 저장 안 한 거래가 있을 때 CLI `add` → 창이 앞으로 오면 "다시 불러오기 / 나중에" 한 번, 나중에 → 다시 묻지 않음, 저장 → "덮어쓰기 / 다시 불러오기 / 취소", 다시 불러오기 → GUI 합계가 파일과 같아지고 GUI 입력은 버려짐 (모두 UI Automation과 Win32 메시지로 확인). macOS는 dev 빌드로 손 확인 필요
- 남은 것: 잠금 표시(8단계, 선택). GUI 저장에는 `.bak` 없음

공수 추정 (2026-11-21, 추정): 감지 1~2시간, GUI 3~4시간, 확인 1~2시간 (Windows, macOS 손으로), 잠금 표시 2시간 정도. 위험: 대화상자 중복, 동시 쓰기 BUSY, 동기화 폴더의 파일 교체 (모두 추정, 재현 안 함)

## 7. 문서

- `CLI.md` 한 벌: 사람과 AI가 같이 읽는 참조 문서. 설치 폴더(Windows `{app}`, macOS 앱 번들 `Contents/Resources/` 또는 `.dmg`)에 포함. 첫 줄에 "`mledger help`로 시작", 파일 경로와 PATH 안내, GUI와 동시 사용 주의
- `help` 출력과 `CLI.md`가 어긋나지 않게 명령/옵션 표를 코드 한곳에서 관리하고, 도움말은 그 표로 만든다
- 사용자 `ReadMe.txt` (짧은 안내, CLAUDE.md Next Steps)에서 `CLI.md`를 가리킴

## 8. 단계

| # | 내용 | 상태 |
|---|---|---|
| 1 | `new`가 기존 파일 데이터를 지우는 문제 수정 + 스모크 테스트 | ✅ 2026-11-21 |
| 2 | 문법 통일(§3), 거래 `show`/`update`/`delete`, `info`, `list` 필터, 값 규칙, 종료 코드, 도움말 한 벌, 스모크 테스트 보강 (`cli_edit_cycle`, `cli_usage_error`) | ✅ 2026-11-21 (대화형은 아직 옛 명령) |
| 3 | `--json`, `--yes`, `--dry-run`, `--no-subtitle`(GUI 부제 토글과 같은 옵션 구조체), 백업. 스모크 테스트 `cli_json_list`, `cli_json_error`, `cli_dry_run`, `cli_backup` | ✅ 2026-11-21 |
| 4 | 대화형 다시 만들기 (§5) | 📋 |
| 5 | `mledger` 이름, 설치 파일 포함, PATH (§2). **7단계 이후에** (AI가 GUI 옆에서 쓰기 시작하기 전에 충돌 감지가 있어야 함) | 📋 |
| 6 | `CLI.md`, 사용자 `ReadMe.txt` (§7) | 📋 |
| 7 | GUI 외부 변경 감지 + 다시 불러오기, `busy_timeout`, 원자적 저장/읽기 (§6) | ✅ 2026-11-22 (Windows 확인, macOS는 손 확인) |
| 8 | 잠금 표시 (§6, 선택) | 📋 |
| 9 | (나중) AI 연결. (a) Agent Skill: `CLI.md`를 `SKILL.md`로 묶은 폴더 (Claude Code, Claude Desktop). 가장 가벼움, 6단계와 함께 가능. (b) MCP 서버: `mledger mcp` 하위 명령으로 CLI 안에 stdio JSON-RPC 서버를 넣음 (선교팀 PC에 Python/Node가 없어도 됨, 도구 하나 = 명령 하나의 `--json` 결과, JSON 라이브러리는 `third_party/`). Claude Desktop은 Desktop Extension(`.mcpb`)으로 한 번에 연결, Claude Code는 `claude mcp add mledger -- mledger mcp`. 공식 SDK는 Python/TypeScript 등이며 C++ 공식 SDK는 없는 것으로 앎 (2026-11-21 기준 기억, 만들 때 최신 문서 확인) | 💡 |
