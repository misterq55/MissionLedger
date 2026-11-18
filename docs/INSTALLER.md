# Windows 설치 프로그램 구현 계획

## Context

MissionLedger는 현재 수동으로 빌드하고 배포해야 하는 상태입니다. 사용자가 프로그램을 설치하고 `.ml` 파일을 더블클릭하여 열 수 있도록 Windows 설치 프로그램이 필요합니다.

**현재 상태:**
- 실행 파일: `x64/Release/MissionLedger.exe` (7.5 MB)
- `.ml` 파일 연결: `register_ml_extension.reg`로 수동 등록 (하드코딩된 경로)
- 리소스 파일: 없음 (아이콘, 버전 정보 미존재)
- 설치 프로그램: 없음

**목표:**
1. `.ml` 파일 확장자를 MissionLedger와 연결 (더블클릭으로 열기)
2. Windows 프로그램 추가/제거에 등록
3. 시작 메뉴 바로가기 생성
4. 프로그램 제거 기능
5. 전문적인 설치 경험 제공

**이미 구현된 기능:**
- `main.cpp`의 명령줄 인수 처리 (34-39줄)
  - `argc > 1`이면 `argv[1]`을 파일 경로로 사용
  - `.ml` 파일 확인 후 자동 열기
  - 더블클릭 지원이 완전히 준비됨

## 선택한 도구: Inno Setup

**이유:**
- wxWidgets 커뮤니티에서 "매우 간단하고 효과적"이라고 평가
- 학습 곡선이 낮음 (스크립트 기반)
- `.ml` 파일 확장자 연결이 쉬움
- 무료 오픈소스, GUI 도구 제공 (Inno Script Studio)
- 단일 실행 파일 프로젝트에 적합

**대안 (제외 이유):**
- WiX Toolset: XML 기반으로 복잡함, 기업 수준 기능 불필요
- NSIS: Inno Setup보다 장점 없음

## 구현 단계

### Phase 1: 리소스 파일 준비

#### 1.1 애플리케이션 아이콘 생성
**파일:** `MissionLedger/resources/app.ico`

**방법:**
1. 온라인 도구 (converticon.com 등)로 임시 아이콘 생성
   - 크기: 32x32, 48x48, 256x256 포함
   - 디자인: 장부/선교 관련 심볼 (단순하게)
2. 또는 Visual Studio에서 새 아이콘 리소스 생성

**테스트용 대안:** Windows 기본 아이콘 임시 사용

#### 1.2 리소스 스크립트 생성
**파일:** `MissionLedger/MissionLedger.rc`

Windows 리소스 컴파일러가 읽는 파일:
- 아이콘 임베딩
- 버전 정보 (회사명, 제품명, 파일 버전 등)
- UTF-8 지원 (Korean, Unicode)

#### 1.3 버전 정보 헤더
**파일:** `MissionLedger/src/MLVersion.h`

버전 상수 정의:
- `ML_VERSION_MAJOR`, `MINOR`, `PATCH`
- `ML_VERSION_STRING` (예: "1.0.0")
- `ML_APP_NAME`, `ML_APP_NAME_KR`

#### 1.4 프로젝트 파일 수정
**파일:** `MissionLedger/MissionLedger.vcxproj`

`<ItemGroup>` 섹션에 추가:
```xml
<ItemGroup>
  <ResourceCompile Include="MissionLedger.rc" />
</ItemGroup>
<ItemGroup>
  <Image Include="resources\app.ico" />
</ItemGroup>
```

빌드 시 리소스 컴파일러(rc.exe)가 자동 실행되어 EXE에 임베딩

### Phase 2: Inno Setup 설치

1. https://jrsoftware.org/isinfo.php에서 다운로드
2. Inno Setup 6.7.0 이상 설치
3. (선택) Inno Script Studio 설치 (GUI 편집기)

### Phase 3: 설치 스크립트 작성

**파일:** `installer/MissionLedger.iss`

**주요 섹션:**
- `[Setup]`: 앱 이름, 버전, 경로, 출력 설정
- `[Files]`: 포함할 파일 (MissionLedger.exe, CLI 등)
- `[Icons]`: 시작 메뉴/바탕화면 바로가기
- `[Registry]`: `.ml` 파일 확장자 연결 (HKEY_CLASSES_ROOT)
- `[Run]`: 설치 완료 후 실행 옵션

**레지스트리 설정 (핵심):**
```pascal
; .ml 파일 확장자 연결
Root: HKA; Subkey: "Software\Classes\.ml\OpenWithProgids"; ...
Root: HKA; Subkey: "Software\Classes\MissionLedgerDocument.ml"; ...
Root: HKA; Subkey: "...\DefaultIcon"; ValueData: "{app}\MissionLedger.exe,0"
Root: HKA; Subkey: "...\shell\open\command"; ValueData: "{app}\MissionLedger.exe %1"
```

**GUID 생성 필요:** Visual Studio → Tools → Create GUID → Registry Format

### Phase 4: 빌드 자동화

**파일:** `build_installer.bat`

단계:
1. Release 빌드 (`msbuild MissionLedger.sln /p:Configuration=Release`)
2. 실행 파일 확인 (`x64\Release\MissionLedger.exe` 존재 여부)
3. Inno Setup 컴파일 (`ISCC.exe installer\MissionLedger.iss`)
4. 출력: `installer/output/MissionLedger-1.0.0-Setup.exe`

### Phase 5: 라이선스 파일 (선택)

**파일:** `LICENSE.txt`

MIT, GPL, 또는 사유 라이선스 추가 (나중에 결정 가능)

## 수정/생성 파일 목록

### 새로 생성
1. `MissionLedger/resources/app.ico` - 애플리케이션 아이콘
2. `MissionLedger/MissionLedger.rc` - 리소스 스크립트 (~50줄)
3. `MissionLedger/src/MLVersion.h` - 버전 헤더 (~10줄)
4. `installer/MissionLedger.iss` - Inno Setup 스크립트 (~100줄)
5. `build_installer.bat` - 빌드 자동화 (~30줄)
6. `LICENSE.txt` - 라이선스 (선택)

### 수정
1. `MissionLedger/MissionLedger.vcxproj` - 리소스 파일 추가 (2-3줄)

**변경 없음:**
- `main.cpp` - 명령줄 인수 처리가 이미 완벽하게 구현됨 (34-39줄)

## 검증 방법

### 1. 빌드 검증
```bash
# Release 빌드
msbuild MissionLedger.sln /p:Configuration=Release /p:Platform=x64

# 실행 파일 속성 확인 (리소스 임베딩 확인)
# x64\Release\MissionLedger.exe 우클릭 → 속성 → 자세히
# 버전 정보, 아이콘이 표시되어야 함

# 설치 프로그램 빌드
build_installer.bat
```

### 2. 설치 테스트

**환경:** 가상 머신 또는 Windows Sandbox 권장

**절차:**
1. `installer\output\MissionLedger-1.0.0-Setup.exe` 실행
2. 설치 마법사 진행 (기본 설정으로)
3. 설치 완료 확인

**확인 항목:**
- [ ] `C:\Program Files\MissionLedger\MissionLedger.exe` 존재
- [ ] 시작 메뉴에 "미션레저" 폴더 생성
- [ ] 바탕화면 바로가기 생성 (선택한 경우)
- [ ] 프로그램 추가/제거에 "미션레저" 등록

### 3. .ml 파일 연결 테스트

**파일 생성:**
1. MissionLedger 실행 → 거래 추가 → "test.ml"로 저장
2. 또는 기존 .ml 파일 사용

**테스트:**
- [ ] 파일 탐색기에서 `test.ml` 아이콘이 MissionLedger 아이콘으로 표시
- [ ] `test.ml` 더블클릭 → MissionLedger 실행 및 파일 열기
- [ ] 거래 내역이 정상 표시

**레지스트리 확인 (regedit):**
- [ ] `HKEY_CLASSES_ROOT\.ml` → "MissionLedgerDocument.ml"
- [ ] `HKEY_CLASSES_ROOT\MissionLedgerDocument.ml\DefaultIcon` → 아이콘 경로
- [ ] `HKEY_CLASSES_ROOT\MissionLedgerDocument.ml\shell\open\command` → 실행 명령

### 4. 제거 테스트

**절차:**
1. 프로그램 추가/제거에서 "미션레저" 제거
2. 또는 시작 메뉴 "미션레저 제거" 실행

**확인 항목:**
- [ ] `C:\Program Files\MissionLedger\` 디렉토리 삭제
- [ ] 시작 메뉴 폴더 삭제
- [ ] 바탕화면 바로가기 삭제
- [ ] 프로그램 목록에서 제거
- [ ] `.ml` 파일 연결 해제 (기본 아이콘으로 변경)
- [ ] 레지스트리 항목 삭제

## 예상 소요 시간

- Phase 1: 리소스 준비 - 1-2시간
- Phase 2: Inno Setup 설치 - 15분
- Phase 3: 스크립트 작성 - 1-2시간
- Phase 4: 빌드 통합 - 30분
- Phase 5: 테스트 및 검증 - 1-2시간

**총 예상:** 4-7시간

## 구현 우선순위

### 필수 (MVP)
1. 애플리케이션 아이콘 (임시라도)
2. 리소스 스크립트 및 버전 정보
3. Inno Setup 스크립트 (.ml 파일 연결)
4. 기본 설치/제거 테스트

### 권장
1. 빌드 자동화 스크립트
2. 레지스트리 검증
3. 라이선스 파일

### 선택 (미래)
1. 코드 서명 (Windows Defender SmartScreen 경고 방지)
2. 자동 업데이트 기능
3. 다중 언어 지원 (영어, 일본어 등)

## Critical Files

설치 프로그램 구현의 핵심 파일 5개:

1. **installer/MissionLedger.iss**
   - Inno Setup 메인 스크립트
   - .ml 파일 연결, 레지스트리 설정 정의
   - 전체 설치 프로세스의 핵심

2. **MissionLedger/MissionLedger.rc**
   - Windows 리소스 스크립트
   - 아이콘 및 버전 정보 임베딩
   - 프로그램 속성에 표시되는 메타데이터

3. **MissionLedger/resources/app.ico**
   - 애플리케이션 아이콘
   - .ml 파일의 파일 탐색기 아이콘
   - 모든 UI 위치에서 표시

4. **build_installer.bat**
   - 빌드 자동화 스크립트
   - Release 빌드 → Inno Setup 컴파일
   - 단일 명령으로 설치 프로그램 생성

5. **MissionLedger/MissionLedger.vcxproj**
   - 프로젝트 파일 (수정)
   - 리소스 파일을 빌드에 포함
   - 빌드 시 rc.exe 자동 실행

## 참고 사항

- **아이콘 디자인**: 장부/선교 관련 심볼 권장 (단순하게)
- **회사/제품명**: 나중에 수정 가능
  - 기본값: "MissionLedger Project", "선교 및 프로젝트 예산 관리 프로그램"
- **라이선스**: 나중에 결정 (MIT/GPL/사유)
- **코드 서명**: 비용 발생 ($100-300/년), 선택 사항

## 다음 단계 (구현 후)

설치 프로그램 완성 후 고려할 사항:
1. GitHub Releases에 설치 파일 배포
2. 사용자 가이드/문서 작성
3. 다운로드 페이지 또는 웹사이트 제작
4. 자동 업데이트 기능 추가 (선택)
