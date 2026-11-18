# 버전 정책

> 적용일: 2026-11-18 (v1.0.0부터)

## 요약

- 버전 형식은 `MAJOR.MINOR.PATCH`입니다. (예: `1.2.3`)
- 버전은 **릴리스할 때** 올립니다. 평소 커밋할 때는 건드리지 않습니다.
- 몇 번째 자리를 올릴지는 **사람이 판단**하고, 판단한 숫자를 여러 곳에 **반영하는 일은 자동**입니다.
- 버전을 수정하는 곳은 저장소 루트의 **`VERSION` 파일 한 곳뿐**입니다.

---

## 1. 번호를 올리는 기준

릴리스 시점에, 지난 릴리스 이후 쌓인 변경 중 **가장 무거운 것** 하나를 기준으로 한 번만 올립니다.

| 자리 | 언제 올리나 | 예 |
|---|---|---|
| **MAJOR** | 이전 버전에서 저장한 `.ml` 파일을 **열 수 없게 되는** 변경 | DB 스키마를 바꾸고 자동 변환(마이그레이션)을 제공하지 않음 |
| **MINOR** | 기능 추가, 또는 자동 변환되는 `.ml` 스키마 변경 | Excel/CSV 내보내기 추가 |
| **PATCH** | 버그 수정, 사용자에게 보이지 않는 리팩터링 | PDF 레이아웃 버그 수정 |
| 올리지 않음 | 문서, 빌드 설정, CI만 바뀐 경우 | `docs/` 수정 |

- MINOR를 올리면 PATCH는 0으로, MAJOR를 올리면 MINOR와 PATCH를 0으로 되돌립니다.
- MAJOR 1을 유지하는 동안 **1.x에서 저장한 `.ml` 파일은 이후 1.x에서 반드시 열려야 합니다.** 이 프로젝트에서 사용자에게 가장 중요한 약속입니다.
- 커밋 접두어가 판단에 도움이 됩니다. `Feat:`가 있으면 MINOR, `Fix:`/`Refactor:`만 있으면 PATCH입니다. 다만 `.ml` 호환성은 접두어로 알 수 없으므로 직접 확인합니다.

## 2. 버전이 흘러가는 경로

```
VERSION ("1.0.0")  ← 릴리스 때 이것만 수정
  │
  ├─▶ CMakeLists.txt ──▶ 리소스 컴파일러(rc)에 ML_VERSION_MAJOR=1 ... 전달
  │                        └─▶ MLVersion.h 매크로 ──▶ MissionLedger.rc ──▶ exe 파일 속성
  ├─▶ installer/MissionLedger.iss ──▶ 설치 파일 버전, 파일명 (MissionLedger-1.0.0-Setup.exe)
  │       └─ 포장할 exe의 버전과 대조 (다르면 오류로 중단)
  └─▶ build_installer.bat ──▶ 진행 메시지, 출력 파일명
```

| 파일 | 역할 |
|---|---|
| `VERSION` | 버전의 유일한 원천. 숫자 한 줄만 둡니다 (BOM 없는 텍스트) |
| `CMakeLists.txt` (루트) | `VERSION`을 읽고 형식을 검증해 `project(VERSION ...)`에 씁니다. `MissionLedger/CMakeLists.txt`가 그 값을 `MissionLedger.rc`에만 전처리기 정의로 넘깁니다 |
| `MissionLedger/src/MLVersion.h` | 숫자 없이 문자열 조립 매크로와 앱 정보만 둡니다 |
| `installer/MissionLedger.iss` | `VERSION`을 직접 읽습니다. `/DMyAppVersion=x.y.z`로 넘기면 그 값을 우선합니다 |

### 빌드 번호 (네 번째 자리)

- exe 파일 버전의 네 번째 자리(`1.0.0.N`)는 빌드 번호입니다. 기본값은 0입니다.
- CI에서 `cmake --preset windows -DML_BUILD_NUMBER=%BUILD_NUMBER%`로 넘깁니다. 사람이 관리하지 않습니다.

### 안전장치

- `VERSION`이 없거나 `1.0.0` 형식이 아니면 CMake 구성 단계가 오류로 실패합니다.
- `VERSION`을 고치고 Release 빌드를 하지 않은 채 설치 파일을 만들면, exe 버전과 맞지 않아 ISCC가 중단됩니다.

### 참고

- 버전 값은 리소스 컴파일러에만 전달됩니다. 버전을 올릴 때마다 모든 `.cpp`가 다시 컴파일되는 것을 피하기 위해서입니다.
- 앱 코드(정보 대화상자, CLI `--version` 등)에서 버전이 필요해지면 그때 CMake `configure_file`로 버전 헤더를 생성하는 방식을 검토합니다 (해당 헤더를 쓰는 파일만 다시 컴파일됨).

## 3. 릴리스 절차

1. **변경 확인**
   ```bash
   git log --oneline v<이전 버전>..HEAD
   ```
2. **버전 결정**: 1절의 기준에 따릅니다.
3. **`VERSION` 수정**: 예: `1.0.0` → `1.1.0`
4. **`CHANGELOG.md` 정리**: `[미배포]` 섹션 제목을 `[1.1.0] - YYYY-MM-DD`로 바꾸고, 그 위에 빈 `[미배포]` 섹션을 새로 만듭니다.
5. **커밋**
   ```bash
   git commit -am "Release: v1.1.0"
   ```
6. **태그**: annotated 태그로 답니다.
   ```bash
   git tag -a v1.1.0 -m "MissionLedger 1.1.0"
   git push origin main --follow-tags
   ```
7. **설치 파일 생성**
   ```bat
   build_installer.bat
   ```
   결과물은 `installer\output\MissionLedger-1.1.0-Setup.exe`입니다.

## 4. 태그 규칙

- 형식은 `v` + 버전입니다. (예: `v1.1.0`)
- **annotated 태그**를 씁니다 (`git tag -a`). 작성자와 날짜가 기록됩니다.
- 태그 메시지는 짧게 둡니다. 변경 내용은 `CHANGELOG.md`에만 적습니다.
- 태그는 `VERSION`이 해당 버전으로 바뀐 커밋에 답니다. CI는 태그와 `VERSION`이 다르면 빌드를 실패시킵니다 (예정, `docs/CI_PLAN.md` 참고).

## 5. 변경 이력

- 사용자에게 보이는 변경은 루트의 [`CHANGELOG.md`](../CHANGELOG.md)에 기록합니다.
- 작업 중에는 `[미배포]` 섹션에 쌓아 두고, 릴리스할 때 버전 번호를 붙입니다.

## 6. 과거 이력

- 2026-11-18 이전의 빌드(내부 번호 1.0.0, 1.0.1)는 외부에 배포하지 않았으므로 **정식 릴리스로 보지 않습니다.**
- 이 정책을 적용한 시점(2026-11-18)을 **v1.0.0**으로 합니다.
- 과거 커밋 메시지에 남아 있는 `(v1.0.1)` 같은 표기는 당시 내부 번호입니다.
