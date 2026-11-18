# MissionLedger

선교 및 프로젝트 예산 관리를 위한 C++ 금융 관리 애플리케이션

## 주요 기능

- **거래 내역 관리**: 수입/지출 거래 등록, 수정, 삭제, 필터링
- **예산 관리**: 카테고리별 예산 계획 수립 및 추적
- **환율 지원**: 외화 거래 자동 환산 및 기록
- **PDF 내보내기**:
  - 거래 내역서: 개별 거래 상세 내역 (카테고리별 그룹화, 환율 정보 포함)
  - 결산 보고서: 예산 대비 실적 비교 (5개 섹션 구조)
- **데이터 저장**: SQLite 기반 안정적인 로컬 저장
- **GUI/CLI 지원**: wxWidgets 기반 GUI 및 명령줄 인터페이스

## 빌드 요구사항

- Visual Studio 2022 (C++ 워크로드, Windows 10 SDK, v143 toolset)
- CMake 3.25 이상
- (설치 파일을 만들 때만) Inno Setup 6.7 이상

wxWidgets, PDF-Writer, SQLite는 저장소의 `third_party/`에 포함돼 있어 따로 설치하거나 환경 변수를 설정할 필요가 없습니다.

## 빌드

Visual Studio 솔루션은 `CMakeLists.txt`에서 생성합니다 (저장소에는 넣지 않습니다).

```bash
# 1. 솔루션 생성 (클론 후 한 번): build/windows/MissionLedger.sln
GenerateProjectFiles.bat          # 또는 cmake --preset windows

# 2. 빌드 (또는 생성된 .sln을 Visual Studio로 열어 빌드/F5)
cmake --build --preset windows-debug
cmake --build --preset windows-release

# 3. 스모크 테스트 (CLI로 tests/fixtures/sample.ml 확인)
ctest --preset windows-release

# 설치 파일: installer/output/MissionLedger-<버전>-Setup.exe
build_installer.bat               # = cmake --workflow --preset windows-release (구성 → 빌드 → 테스트 → 설치 파일)
```

- 실행 파일: `build/windows/bin/<Debug|Release>/`
- 첫 빌드는 wxWidgets와 PDF-Writer도 함께 컴파일하므로 1~2분 정도 걸립니다.
- 소스 파일은 각 프로젝트 폴더에 만들기만 하면 다음 빌드에 포함됩니다. Visual Studio의 "새 항목 추가"나 속성 페이지는 생성된 프로젝트에만 반영되므로 쓰지 않습니다. 자세한 내용은 [CLAUDE.md](CLAUDE.md)의 "Working in the IDE"를 참고하세요.

## 프로젝트 구조

- `MissionLedger/` - GUI 애플리케이션
- `MissionLedgerCore/` - 코어 비즈니스 로직 라이브러리 (정적 라이브러리)
- `MissionLedgerCLI/` - 명령줄 인터페이스
- `third_party/` - 외부 라이브러리 (SQLite, PDF-Writer, wxWidgets)

## 추가 정보

자세한 아키텍처 및 개발 가이드는 [CLAUDE.md](CLAUDE.md)를 참고하세요.
