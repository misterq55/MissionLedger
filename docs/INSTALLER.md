# Windows 설치 프로그램

> 상태: ✅ Setup 생성 및 설치 기능 구현 완료 (v1.0.0, 2026-11-18) · 깨끗한 PC에서의 검증은 하지 않음 (상용 배포 대상 아님)
> 도구: [Inno Setup](https://jrsoftware.org/isinfo.php) 6.7 이상
> 초기 구현 계획 문서: `git show 4495514:docs/INSTALLER.md`

사용자가 MissionLedger를 설치하고 `.ml` 파일을 더블클릭으로 열 수 있게 하는 Windows 설치 프로그램입니다.
결과물은 `installer\output\MissionLedger-<버전>-Setup.exe` 하나입니다.

---

## 1. 구성 파일

| 파일 | 역할 |
|---|---|
| `installer/MissionLedger.iss` | 설치 프로그램 정의. 복사할 파일, 바로가기, `.ml` 파일 연결, 제거 동작 |
| `build_installer.bat` | Release 빌드 → exe 확인 → 설치 파일 컴파일을 한 번에 실행하는 편의 스크립트 |
| `VERSION` | 버전의 유일한 원천. 설치 파일 버전과 파일 이름이 여기서 정해집니다 ([VERSIONING.md](VERSIONING.md)) |
| `MissionLedger/MissionLedger.rc` | exe에 아이콘과 버전 정보를 넣는 리소스 스크립트 |
| `MissionLedger/resources/app.ico` | 앱 아이콘. `.ml` 파일 아이콘과 설치 프로그램 아이콘으로도 쓰입니다 |
| `LICENSE.txt` | 설치 중 표시되는 라이선스 (MIT) |

`main.cpp`는 명령줄 인수로 받은 `.ml` 파일을 자동으로 열기 때문에, 더블클릭으로 열기는 레지스트리 등록만으로 동작합니다.

## 2. 설치 파일 만들기

### 방법 A: 스크립트 (권장)

저장소 루트에서 실행합니다.

```bat
build_installer.bat
```

### 방법 B: 수동

```bat
cmake --preset windows
cmake --build --preset windows-release
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer\MissionLedger.iss
```

`.iss`는 기본으로 `build\windows\bin\Release\`의 exe를 포장합니다. 다른 폴더의 exe를 쓰려면 `ISCC /DExeDir="..."`로 지정합니다.

Inno Setup GUI에서 `.iss`를 열고 Compile을 눌러도 됩니다.

### 주의

- 설치 파일 버전은 `VERSION` 파일에서 읽습니다. `.iss`에 버전을 직접 적지 않습니다.
- **Release 빌드를 먼저 해야 합니다.** `build\windows\bin\Release\MissionLedger.exe`의 버전이 `VERSION`과 다르면 ISCC가 오류를 내고 중단합니다. 예전 exe가 새 버전 이름으로 포장되는 사고를 막기 위해서입니다.
- Visual C++ 런타임 설치 파일(`vc_redist.x64.exe`)은 Visual Studio 2022 설치 폴더에서 자동으로 찾습니다 (Community → Professional → Enterprise → Build Tools 순서, `VC\Redist\MSVC\v143\`). 따로 내려받을 필요가 없습니다. 다른 위치의 파일을 쓰려면 `ISCC /DVCRedistPath="...\vc_redist.x64.exe"`로 지정합니다.

## 3. 설치 프로그램 동작

### 설치

| 항목 | 내용 |
|---|---|
| 설치 위치 | `C:\Program Files\MissionLedger\` |
| 권한 | **관리자만** (모든 사용자 설치). Visual C++ 런타임 설치에 관리자 권한이 필요해서 "현재 사용자만 설치"는 허용하지 않습니다 |
| 설치되는 파일 | `MissionLedger.exe`, `README.md`, `LICENSE.txt` |
| Visual C++ 런타임 | 런타임이 없거나 포함된 버전보다 낮은 PC에서만 화면 표시 없이 설치합니다. 이미 있으면 건너뜁니다 (아래 참고) |
| 바로가기 | 시작 메뉴 "미션레저" 폴더 (프로그램 + 제거). 바탕화면 바로가기는 선택 사항입니다 |
| 요구 사항 | Windows 10 1809 이상, x64 |
| 언어 | 한국어 |
| 완료 후 | "미션레저 실행" 체크박스 |

### `.ml` 파일 연결 (레지스트리)

`HKA` 루트를 씁니다. 관리자 설치이므로 `HKLM\Software\Classes`에 기록됩니다.
(`HKEY_CLASSES_ROOT`는 `HKLM`과 `HKCU`의 `Software\Classes`를 합친 보기이며, 같은 키가 있으면 `HKCU`가 우선합니다.)

| 키 | 값 | 용도 |
|---|---|---|
| `Software\Classes\MissionLedgerDocument.ml` | `MissionLedger Document` | ProgID (파일 형식 이름) |
| `...\MissionLedgerDocument.ml\DefaultIcon` | `"{app}\MissionLedger.exe",0` | `.ml` 파일 아이콘 |
| `...\MissionLedgerDocument.ml\shell\open\command` | `"{app}\MissionLedger.exe" "%1"` | 더블클릭 시 실행 명령 |
| `Software\Classes\.ml` | `MissionLedgerDocument.ml` | 확장자 → ProgID 연결 |
| `Software\Classes\.ml\OpenWithProgids` | `MissionLedgerDocument.ml` | "연결 프로그램" 목록에 표시 |
| `Software\Classes\Applications\MissionLedger.exe` | `FriendlyAppName`, `SupportedTypes`, `shell\open\command` | "연결 프로그램" 메뉴의 앱 이름과 지원 형식 |

- `ChangesAssociations=yes`이므로 설치와 제거 후 탐색기 아이콘이 바로 갱신됩니다.
- `%1`을 따옴표로 감싸서 공백이나 한글이 있는 경로도 열립니다.
- Windows 10/11에서는 사용자가 다른 앱을 기본값으로 고른 적이 있으면(`UserChoice`) 그 선택이 우선합니다. 이 경우 "연결 프로그램 → 미션레저"를 한 번 선택해야 합니다.

### Visual C++ 런타임

exe가 동적 런타임(`/MD`)으로 빌드되어 `MSVCP140.dll`, `VCRUNTIME140.dll`, `VCRUNTIME140_1.dll`이 필요합니다. 런타임이 없는 PC에서는 *"MSVCP140.dll이 없어 코드 실행을 진행할 수 없습니다"* 오류로 실행되지 않으므로, 설치 프로그램에 런타임 설치 파일(약 25MB)을 포함했습니다.

- 판단 기준: `HKLM\SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64`의 `Installed`, `Major`, `Minor`, `Bld` 값을 포함된 `vc_redist.x64.exe`의 버전과 비교합니다. 없거나 낮을 때만 설치합니다.
- 설치 옵션: `/install /quiet /norestart`
- 제거할 때 런타임은 지우지 않습니다. 다른 프로그램도 함께 쓰는 구성 요소이기 때문입니다.
- 대안: 정적 런타임(`/MT`)으로 빌드하면 런타임 동봉이 필요 없습니다. 그 대신 wxWidgets와 PDF-Writer를 `/MT`로 다시 빌드해야 합니다. wxWidgets를 다시 빌드할 때 전환을 검토합니다 (5절).

### 제거

- 설치 폴더 전체를 삭제합니다.
- 시작 메뉴와 바탕화면 바로가기를 삭제합니다.
- ProgID 키와 `Applications\MissionLedger.exe` 키를 삭제합니다.
- `.ml`의 기본값과 `OpenWithProgids` 값을 삭제합니다. 다른 앱이 `.ml`에 등록한 값은 건드리지 않습니다.
- 사용자가 저장한 `.ml` 파일은 삭제하지 않습니다.

## 4. 설치 테스트 체크리스트

필요할 때 참고하는 체크리스트입니다. 다른 사람에게 배포하기 전이라면 **Windows Sandbox** 또는 VM처럼 깨끗한 환경에서 확인하는 것이 좋습니다. 개발 PC에는 VC++ 런타임과 이전 레지스트리 등록이 남아 있어 문제가 가려질 수 있습니다.

### 설치
- [ ] `MissionLedger-<버전>-Setup.exe` 실행 → 기본 설정으로 설치 완료
- [ ] `C:\Program Files\MissionLedger\MissionLedger.exe` 존재
- [ ] (런타임이 없는 환경) 설치 중 "Visual C++ 런타임을 설치하는 중..." 표시 후 프로그램이 정상 실행
- [ ] exe 속성 → 자세히에서 버전과 아이콘 확인
- [ ] 시작 메뉴에 "미션레저" 폴더 생성
- [ ] 프로그램 추가/제거에 "미션레저"가 올바른 버전으로 표시
- [ ] 설치 완료 화면에서 실행 → 프로그램이 정상적으로 뜸

### `.ml` 파일 연결
- [ ] 탐색기에서 `.ml` 파일 아이콘이 MissionLedger 아이콘으로 표시
- [ ] `.ml` 파일 더블클릭 → 프로그램이 실행되고 파일이 열림
- [ ] 경로에 공백과 한글이 있는 `.ml` 파일도 열림
- [ ] 결산 PDF와 거래 내역서 PDF 내보내기가 동작하고 한글이 정상 표시
- [ ] regedit에서 `HKEY_CLASSES_ROOT\.ml` (기본값) = `MissionLedgerDocument.ml`

### 제거
- [ ] 프로그램 추가/제거 또는 시작 메뉴 "미션레저 제거"로 제거
- [ ] 설치 폴더, 시작 메뉴 폴더, 바탕화면 바로가기 삭제
- [ ] `.ml` 파일 아이콘이 기본 아이콘으로 돌아감
- [ ] regedit에서 `MissionLedgerDocument.ml` 키 삭제 확인

### 업그레이드
- [ ] 이전 버전이 설치된 상태에서 새 버전 설치 → 덮어쓰기로 정상 설치, 프로그램 목록에 하나만 표시

## 5. 알려진 이슈와 남은 작업

| 항목 | 내용 | 우선순위 |
|---|---|---|
| 정적 런타임(`/MT`) 전환 검토 | 현재는 `vc_redist.x64.exe`를 동봉해 해결했습니다 (3절). `/MT`로 바꾸면 설치 파일이 약 4MB로 줄고, 관리자 권한 없이 설치하거나 exe만 복사해 쓸 수 있습니다. wxWidgets와 PDF-Writer를 `/MT`로 다시 빌드해야 하므로 wxWidgets 재빌드 때 함께 검토합니다 | 낮음 |
| **개발 PC의 수동 등록과 충돌** | `register_ml_extension.reg`를 실행한 적이 있는 PC에는 `HKCU\Software\Classes\.ml` → `MissionLedger.Document`(Debug exe 경로)가 남아 있습니다. HKCU가 HKLM보다 우선하므로, 관리자 설치 후에도 더블클릭하면 Debug exe가 실행될 수 있습니다. 해당 키를 지우거나 `.reg` 파일을 삭제하는 것을 검토합니다 | 중간 |
| CLI 미포함 | `MissionLedgerCLI.exe`는 설치되지 않습니다. 필요하면 `[Files]`에 추가합니다 | 낮음 |
| 폰트 번들 미사용 | `[Files]`의 `fonts\*.ttf` 항목이 주석 처리돼 있습니다. PDF는 시스템의 맑은 고딕을 씁니다. 코드는 `{app}\fonts\`를 먼저 찾도록 준비돼 있습니다 | 낮음 |
| 코드 서명 없음 | 서명하지 않은 설치 파일은 SmartScreen 경고가 뜹니다. 인증서 비용이 연 $100–300입니다 | 선택 |
| CI 연동 | `build_installer.bat`은 CMake 프리셋을 쓰므로 VS 에디션 경로 문제는 없어졌지만, 끝에 `pause`가 있어 사람이 실행하는 용도입니다. CI는 bat을 쓰지 않고 Jenkinsfile에서 단계별로 실행합니다 ([CI_PLAN.md](CI_PLAN.md)) | CI 도입 시 |
| 자동 업데이트, 다국어 | 미구현 | 선택 |
| 회사명/제품명 | `MissionLedger Project`, `선교 및 프로젝트 예산 관리 프로그램`은 임시값입니다. `MLVersion.h`와 `.iss`의 `MyAppPublisher`에서 바꿉니다 | 선택 |
| 아이콘 | `app.ico`가 임시 아이콘이라면 장부나 선교와 관련된 단순한 심볼로 교체를 검토합니다 | 선택 |
| 라이선스 | 현재 MIT (`LICENSE.txt`). 필요하면 다시 검토합니다 | 선택 |

### 배포 이후 고려할 것

1. 설치 파일 배포 위치 (GitHub Releases 등)
2. 사용자 가이드 작성
3. 다운로드 페이지나 웹사이트
4. 자동 업데이트 (선택)

## 6. 구현 이력

| 커밋 | 내용 |
|---|---|
| `d52588c` | 리소스 스크립트, 아이콘, 버전 헤더 추가 |
| `0d32271` | Inno Setup 스크립트, `build_installer.bat`, `LICENSE.txt` 추가 |
| `392b303` | 폰트 파일을 선택적으로 포함하도록 수정 |
| `c8196d1` | `.ml` 파일 연결 레지스트리 정리 (`HKA`, `OpenWithProgids`, 제거 시 정리) |
| `20a8f7e` | `VERSION` 파일 기반 버전 단일화, exe 버전 대조, bat 괄호 버그 수정 |
| (v1.0.0 태그 커밋) | Visual C++ 런타임 동봉, 관리자 설치로 고정 |

### 도구 선택 이유 (Inno Setup)

- 스크립트 기반이라 배우기 쉽고, 단일 exe 앱의 파일 연결을 간단하게 처리할 수 있습니다.
- WiX Toolset은 XML 기반으로 복잡하고, 이 규모에 필요 없는 기업용 기능이 많아 제외했습니다.
- NSIS는 Inno Setup보다 나은 점이 없어 제외했습니다.
