# mledger: MissionLedger 명령줄 도구

> 명령 목록은 `mledger help`, 명령별 옵션과 예는 `mledger help <명령>`. 이 문서는 개념과 예를 설명하고, 옵션 전체는 `help`가 기준입니다.

`mledger`는 미션레저(MissionLedger) 프로그램과 같은 `.ml` 파일을 읽고 씁니다. 사람이 터미널에서 쓸 수도 있고, 스크립트나 AI(예: Claude Code)가 쓸 수도 있습니다.

## 1. 실행 준비

**Windows**: 설치 화면에서 "명령줄에서 mledger 사용 (PATH에 추가)"를 켜 두면(기본) 새로 연 터미널(명령 프롬프트, PowerShell)에서 바로 실행됩니다. 이미 열려 있던 터미널은 다시 열어야 합니다.

```
mledger help
```

PATH에 넣지 않았다면 전체 경로로 실행합니다: `"C:\Program Files\MissionLedger\mledger.exe" help`

**macOS**: `mledger`는 앱 안에 있습니다. 터미널에서 한 번 아래 명령을 실행하면 이후 `mledger`로 실행됩니다 (관리자 비밀번호를 묻습니다).

```
sudo mkdir -p /usr/local/bin
sudo ln -s /Applications/MissionLedger.app/Contents/MacOS/mledger /usr/local/bin/mledger
```

링크 없이 쓰려면 전체 경로로: `/Applications/MissionLedger.app/Contents/MacOS/mledger help`

앱을 지울 때는 링크도 지웁니다: `sudo rm /usr/local/bin/mledger` (앱을 새 버전으로 바꿀 때는 그대로 두면 됩니다)

## 2. 기본 형식

```
mledger <명령> <파일.ml> [옵션]
```

- 옵션은 `--이름 값` 또는 `--이름=값`
- 띄어쓰기가 있는 값은 따옴표로: `--desc "현지 식당 저녁"`, `"2026 필리핀 단기선교.ml"`
- 날짜: `YYYY-MM-DD` (예: `2026-01-15`)
- 금액: 원 단위 정수, 쉼표 허용 (`50000`, `50,000`). 외화 금액과 환율은 소수 가능 (`12.5`, `1350.5`)
- 유형: `income`/`i`/`수입`, `expense`/`e`/`지출`
- 거래와 예산은 ID로 가리킵니다. ID는 `list`, `budget list`, `--json` 출력에 나옵니다

## 3. 자주 쓰는 예

파일 만들기와 요약:

```
mledger new 2026선교.ml
mledger info 2026선교.ml
```

```
파일: 2026선교.ml
거래: 10건 (수입 3건, 지출 7건)
합계: 수입 4,800,000원, 지출 4,309,750원, 잔액 490,250원
기간: 2026-01-05 ~ 2026-01-15
예산: 9건 (수입 5,000,000원, 지출 4,800,000원)
...
```

거래 목록과 필터 (`--type`, `--category`, `--from`, `--to`, `--search`):

```
mledger list 2026선교.ml
mledger list 2026선교.ml --type expense --from 2026-01-10 --to 2026-01-31
mledger list 2026선교.ml --search 숙박
mledger show 2026선교.ml --id 4
```

거래 추가 (`--date`를 생략하면 오늘):

```
mledger add 2026선교.ml --type expense --category 숙식 --item 식비 --amount 45000 --date 2026-01-15 --desc "현지 식당"
```

```
추가됨: ID 10 | 지출 | 숙식 / 식비 | 45,000원 | 2026-01-15
(백업: 2026선교.ml.bak)
```

외화 거래 (원화 금액은 외화 금액 × 환율로 계산):

```
mledger add 2026선교.ml --type expense --category 교통 --currency PHP --original 2000 --rate 24.5
```

거래 수정 (준 항목만 바뀜, 빈 값 `""`은 지움):

```
mledger update 2026선교.ml --id 10 --amount 48000 --receipt R-010
mledger update 2026선교.ml --id 10 --desc ""
```

거래 삭제 (터미널에서는 확인을 묻고, 스크립트에서는 `--yes`가 필요):

```
mledger delete 2026선교.ml --id 10
mledger delete 2026선교.ml --id 10 --yes
```

예산 (`budget list`, `budget add`, `budget update`, `budget delete`):

```
mledger budget list 2026선교.ml
mledger budget add 2026선교.ml --type expense --category 숙식 --item 숙박 --amount 800000
mledger budget update 2026선교.ml --id 3 --amount 900000
mledger budget delete 2026선교.ml --id 3 --yes
```

PDF 내보내기 (`--out`을 생략하면 `.ml` 옆에 `<파일이름>-결산보고서.pdf`, `<파일이름>-거래내역서.pdf`):

```
mledger export settlement 2026선교.ml
mledger export transactions 2026선교.ml --out 거래내역.pdf
mledger export settlement 2026선교.ml --no-subtitle
```

## 4. 대화형 모드

인수 없이 `mledger`를 실행하거나 `mledger shell <파일>`로 시작합니다. 명령은 위와 같고, 열린 파일은 명령에서 생략합니다.

```
> open "2026 필리핀 단기선교.ml"
열림: 2026 필리핀 단기선교.ml (거래 10건, 예산 9건)
[2026 필리핀 단기선교.ml] > add
유형  1) 수입  2) 지출 : 2
카테고리  1) 교통  2) 기타  3) 사역  4) 숙식  (번호 또는 새 이름) : 4
...
저장할까요? [Y/n] :
```

- 옵션 없이 `add`, `update`, `budget add`, `budget update`를 입력하면 항목을 차례로 묻고, 저장 전에 미리 보기를 보여 줍니다. 수정에서는 Enter = 그대로, `-` = 지움
- 옵션을 일부 주면(`add --amount 5000`) 빠진 필수 항목만 묻습니다
- 질문 중 `취소`를 입력하면 그 명령을 취소합니다 (저장 안 함)
- 대화형 전용 명령: `open <파일>`, `new <파일>`, `close`, `help`, `exit`
- 수정에서 원화 ↔ 외화 전환, 통화 코드 변경은 옵션으로만 됩니다 (`update --id 10 --currency KRW --amount 50000`)

## 5. 스크립트와 AI에서 쓰기

### JSON 출력 (`--json`)

모든 명령은 `--json`을 주면 결과나 오류를 JSON 한 줄로 출력합니다. 키는 영어, 금액은 원 단위 정수, 날짜는 `YYYY-MM-DD`입니다.

성공:

```json
{"ok":true,"command":"add","dryRun":false,"backup":"2026선교.ml.bak","transaction":{"id":10,"type":"expense","category":"교통","item":"","description":"","amount":49000,"date":"2026-01-16","receipt":"","exchange":{"currency":"PHP","original":2000.0,"rate":24.5}}}
```

`list --json`:

```json
{"ok":true,"command":"list","count":10,"summary":{"income":4800000,"expense":4309750,"balance":490250},"transactions":[{"id":0,"type":"income","category":"참가비","item":"팀원 참가비","description":"팀원 6명 x 500,000원","amount":3000000,"date":"2026-01-05","receipt":"","exchange":null}, ...]}
```

오류:

```json
{"ok":false,"command":"show","error":{"code":"failed","message":"ID 99인 거래가 없습니다."}}
```

### 종료 코드

| 코드 | 뜻 | JSON `error.code` |
|---|---|---|
| 0 | 성공 | (없음, `"ok":true`) |
| 1 | 실행 실패: 파일이나 ID가 없음, 저장 실패 | `failed` |
| 2 | 사용법 오류: 알 수 없는 명령이나 옵션, 잘못된 값, 빠진 필수 옵션 | `usage` |

### 미리 보기, 확인, 백업

- `--dry-run`: 쓰기 명령(`add`, `update`, `delete`, `budget add/update/delete`)의 결과만 보여주고 저장하지 않습니다 (백업도 안 만듦)
- `--yes`: 확인 질문을 생략합니다. 터미널이 아닌 곳(스크립트, AI)에서 삭제하려면 꼭 필요합니다
- 백업: 쓰기 명령은 저장하기 직전 상태를 `<파일>.bak`(예: `2026선교.ml.bak`)으로 남깁니다. 직전 1개만 남고 다음 쓰기 때 덮어씁니다. 되돌리려면 `.bak` 파일을 원래 이름으로 복사합니다

### AI가 지킬 규칙

AI(Claude Code 등)에게 이 문서를 주고 `mledger`로 장부를 다루게 할 때의 규칙입니다.

1. 결과를 읽을 때는 항상 `--json`을 붙이고, `ok`와 종료 코드로 성공을 판단합니다
2. 거래나 예산 ID는 `list --json`, `budget list --json`, `info`로 확인합니다. ID를 추측하지 않습니다
3. 쓰기 명령은 먼저 `--dry-run`으로 실행해 결과를 사람에게 보여주고, 확인을 받은 뒤 실행합니다
4. `delete`와 `budget delete`의 `--yes`는 사람이 그 삭제를 명시적으로 허락했을 때만 붙입니다
5. 옵션이 불확실하면 `mledger help <명령>`을 실행해 확인합니다. 이 문서보다 `help`가 정확합니다
6. 대화형 모드(`mledger`, `mledger shell`)는 쓰지 않습니다. 사람용입니다
7. 실수로 잘못 썼다면 사람에게 알리고, `<파일>.bak`에 직전 상태가 있다고 안내합니다 (백업은 직전 1개뿐이므로 다음 쓰기 전에)

## 6. 프로그램(GUI)과 함께 쓸 때

같은 파일을 프로그램에서 열어 둔 채로 `mledger`를 써도 됩니다.

- 프로그램은 다른 곳에서 파일이 바뀐 것을 알아챕니다. 프로그램에 저장하지 않은 변경이 없으면 바로 다시 불러오고, 있으면 다시 불러올지 묻습니다
- 프로그램에서 저장할 때 `mledger`의 변경을 덮어쓰게 되면 "덮어쓰기 / 다시 불러오기 / 취소"를 묻습니다
- 동시에 쓰는 중이면 최대 5초 기다립니다

## 7. 문제 해결

| 증상 | 해결 |
|---|---|
| `mledger`을(를) 찾을 수 없음 / command not found | Windows: 설치 때 PATH 체크를 켰는지 확인하고 터미널을 새로 엽니다. macOS: 1절의 `ln -s`를 실행합니다. 또는 전체 경로로 실행 |
| 파일을 열 수 없습니다 | 경로와 이름을 확인합니다. 띄어쓰기가 있으면 따옴표로 묶습니다. 없는 파일은 `new`로 만듭니다 |
| 파일이 이미 있습니다 (`new`) | `new`는 기존 파일을 덮어쓰지 않습니다. 다른 이름을 쓰거나 그 파일을 그대로 씁니다 |
| 삭제하려면 --yes가 필요합니다 | 터미널이 아닌 곳에서 실행 중입니다. 지울 것이 확실하면 `--yes`를 붙입니다 |
| 파일 저장에 실패했습니다 | 다른 프로그램이 5초 넘게 쓰고 있거나, 폴더에 쓰기 권한이 없습니다. 잠시 뒤 다시 실행합니다. 파일은 저장 전 상태 그대로이고 `.bak`도 남아 있습니다 |
| 백업을 만들 수 없어 저장하지 않았습니다 | `.ml` 옆에 `.bak`을 만들 수 없습니다 (쓰기 권한, 디스크 공간). 파일은 바뀌지 않았습니다 |
| PDF 한글이 깨짐 | 설치 폴더의 `fonts` 폴더(macOS: 앱 안)가 있는지 확인합니다. 다시 설치하면 복구됩니다 |
