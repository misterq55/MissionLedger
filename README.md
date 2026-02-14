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

- Visual Studio 2022 (v143 toolset)
- Windows 10 SDK
- wxWidgets 3.2.8 (`C:\wxWidgets-3.2.8` 위치 및 `WXWIN` 환경 변수 필요)
- CMake 3.15 이상

## 첫 빌드 (Debug)

처음 저장소를 클론한 경우, 먼저 PDF-Writer 라이브러리를 빌드해야 합니다:

```bash
cd third_party/pdfwriter
mkdir build
cd build

# Visual Studio 프로젝트 생성
cmake .. -G "Visual Studio 17 2022" -A x64

# Debug/Release 빌드
cmake --build . --config Debug
cmake --build . --config Release
```

## 전체 프로젝트 빌드

```bash
# Debug 빌드
msbuild MissionLedger.sln /p:Configuration=Debug /p:Platform=x64

# Release 빌드
msbuild MissionLedger.sln /p:Configuration=Release /p:Platform=x64
```

## 프로젝트 구조

- `MissionLedger/` - GUI 애플리케이션
- `MissionLedgerCore/` - 코어 비즈니스 로직 라이브러리 (정적 라이브러리)
- `MissionLedgerCLI/` - 명령줄 인터페이스
- `third_party/` - 외부 라이브러리 (SQLite, PDF-Writer)

## 추가 정보

자세한 아키텍처 및 개발 가이드는 [CLAUDE.md](CLAUDE.md)를 참고하세요.
