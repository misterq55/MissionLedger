# MissionLedger TODO

## 🐛 버그 / 개선 사항

### 날짜 필드 선택적 입력 지원

**문제**:
- 날짜 필드를 비워도 저장 후 다시 불러오면 현재 날짜가 자동으로 입력됨
- UI에서 날짜를 선택적으로 입력할 수 없음

**원인**:
- `MLTransaction::parseDateTime()` (MLTransaction.cpp:130-155행)
- 빈 문자열 입력 시 `std::chrono::system_clock::now()` 반환
- Entity의 `DateTime` 필드가 `std::chrono::system_clock::time_point` 타입으로 null 표현 불가

**해결 방안**:

#### 방안 1: std::optional 사용 ⭐ (권장)
**요구사항**: C++17
**작업량**: 중간 (10~15곳 수정)
**장점**:
- 타입 시스템으로 "값 없음" 표현 (의미 명확)
- 컴파일러/런타임 안전성 제공
- SQLite NULL과 개념적 일치
- 장기 유지보수 용이

**수정 파일**:
- `MLTransaction.h` - `DateTime` 타입 변경, getter/setter 시그니처
- `MLTransaction.cpp` - `parseDateTime()`, `GetDateTimeString()`, 생성자
- `MLModel.cpp` - DTO 변환 (거의 수정 불필요)
- `MLSQLiteStorage.cpp` - 저장/로드 (거의 수정 불필요)
- 필터링 로직 - 날짜 없는 항목 처리 정책

**선행 작업**:
```bash
# Visual Studio 프로젝트 설정 변경
# C/C++ → 언어 → C++ 언어 표준 → ISO C++17 표준 (/std:c++17)
# 또는 .vcxproj 파일에 추가:
# <LanguageStandard>stdcpp17</LanguageStandard>
```

#### 방안 2: 특수 값 사용
**요구사항**: C++11 (현재 버전)
**작업량**: 적음 (3~4곳 수정)
**장점**: 빠른 구현
**단점**:
- epoch 0 (1970-01-01)을 "날짜 없음"으로 사용 (마법 숫자)
- 의미 모호, 유지보수 어려움
- 실제 1970-01-01 입력 불가

**수정 내용**:
```cpp
// MLTransaction.cpp
std::chrono::system_clock::time_point parseDateTime(const std::string& dateTimeStr)
{
    if (dateTimeStr.empty())
        return std::chrono::system_clock::time_point();  // epoch 0
    // ...
}

std::string GetDateTimeString() const
{
    if (DateTime == std::chrono::system_clock::time_point())
        return "";  // 빈 문자열 반환
    // ...
}
```

#### 방안 3: 현재 동작 유지
- 날짜를 필수 입력으로 유지
- 수정 없음

---

## 📝 최근 완료된 작업

### ✅ 증분 업데이트 → 전체 재로드 방식 변경
**완료일**: 2026-02-01
**변경 내용**:
- `applyCurrentFilter()` 증분 업데이트 로직 제거
- `Freeze()/Thaw()` + `DeleteAllItems()` + 전체 재로드 방식으로 변경
- `OnClearFilter()` 정렬 상태 초기화 추가
- `getCurrentListIds()` 함수 제거

**해결된 버그**:
- 파일 전환 시 기존 데이터 남는 문제
- 필터 초기화 시 정렬 유지 문제
- 항목이 하나씩 사라지는 시각적 문제

**수정 파일**:
- `MLMainFrame.cpp`
- `MLMainFrame.h`

### ✅ CLI UTF-8 한글 지원
**완료일**: 2026-02-01
**변경 내용**:
- Windows에서 명령 인자 UTF-8 변환 (CommandLineToArgvW + WideCharToMultiByte)
- 콘솔 코드 페이지 UTF-8 설정
- stdout 버퍼링 비활성화

**수정 파일**:
- `MissionLedgerCLI/main_cli.cpp`

---

## 🚀 향후 계획

### Phase 4: 데이터 Export & 배포
- [ ] Excel/CSV export 기능
- [ ] Installer 제작
- [ ] .ml 파일 연결 등록
