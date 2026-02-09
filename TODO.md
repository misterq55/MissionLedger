# MissionLedger TODO

## 🐛 버그 / 개선 사항

### IMLModel 인터페이스 일관성 개선

**문제**:
- Transaction과 Budget CRUD 메서드들의 인터페이스가 일관성 없게 설계됨
- 반환 타입, 메서드 이름, const 한정자가 불일치

**불일치 항목**:

1. **Add 반환 타입 불일치**
   ```cpp
   void AddTransaction(const FMLTransactionData& transactionData);  // void - 실패 표현 불가
   bool AddBudget(const FMLItemBudgetData& budgetData);             // bool - 성공/실패 반환
   ```

2. **삭제 메서드 이름 불일치**
   ```cpp
   bool RemoveTransaction(const int transactionId);                 // "Remove"
   bool DeleteBudget(const std::string& category, const std::string& item);  // "Delete"
   ```

3. **GetAll const 일관성 없음**
   ```cpp
   std::vector<FMLTransactionData> GetAllTransactionData();         // non-const
   std::vector<FMLItemBudgetData> GetAllBudgets() const;            // const ✅
   ```

4. **Summary 메서드 이름 + const 불일치**
   ```cpp
   FMLTransactionSummary CalculateTransactionSummary();             // "Calculate", non-const
   FMLBudgetSummary GetBudgetSummary() const;                       // "Get", const ✅
   ```

5. **필터링된 데이터 조회 메서드 누락**
   ```cpp
   std::vector<FMLTransactionData> GetFilteredTransactionData(...); // ✅ 있음
   // Budget에는 GetFilteredBudgets() 없음                         // ❌ 없음
   ```

**수정 방안**: Budget 패턴(bool 반환, const 메서드)을 기준으로 통일

**수정 파일**:
- `IMLModel.h` - 인터페이스 선언 수정
- `IMLController.h` - 동일하게 수정
- `MLModel.h/cpp` - 구현 시그니처 수정
- `MLController.cpp` - 구현 시그니처 수정
- 모든 호출부 - 반환값 처리 수정

**수정 내용**:
```cpp
// 1. AddTransaction 반환 타입 변경
virtual bool AddTransaction(const FMLTransactionData& transactionData) = 0;

// 2. RemoveTransaction → DeleteTransaction 이름 통일
virtual bool DeleteTransaction(const int transactionId) = 0;

// 3. GetAllTransactionData에 const 추가
virtual std::vector<FMLTransactionData> GetAllTransactionData() const = 0;

// 4. Summary 메서드 이름 통일 + const 추가
virtual FMLTransactionSummary GetTransactionSummary() const = 0;
virtual FMLTransactionSummary GetFilteredTransactionSummary(const FMLFilterCriteria& criteria) const = 0;

// 5. Budget에 필터링된 목록 조회 추가 (선택적)
virtual std::vector<FMLItemBudgetData> GetFilteredBudgets(const FMLFilterCriteria& criteria) const = 0;
```

---

### Budget 식별자 방식 개선 (자연 키 → 대리 키)

**문제**:
- Transaction은 ID 기반(대리 키), Budget은 (Category, Item) 기반(자연 키)으로 식별 방식이 불일치
- 자연 키 사용으로 인한 여러 문제점 발생

**불일치 및 문제점**:

1. **설계 일관성 문제**
   ```cpp
   // Transaction - ID 기반
   DeleteTransaction(int transactionId);
   GetTransactionData(int transactionId);

   // Budget - 자연 키 기반
   DeleteBudget(const string& category, const string& item);
   GetBudget(const string& category, const string& item);
   ```

2. **키 변경의 어려움**
   - Transaction: ID는 불변, Category만 수정하면 됨 (간단)
   - Budget: Category가 키라서 Delete + Insert 필요 (복잡)

3. **UI 구현 복잡도**
   - Transaction: ID만 추적하면 됨
   - Budget: Category와 Item 모두 추적 필요
   - 현재 View는 Category만 저장 → Edit/Delete 불가능

4. **성능 차이**
   - Transaction: 정수 비교 (O(1))
   - Budget: 문자열 쌍 비교 (O(n))
   - 참고: Budget 데이터가 적어 실제 영향은 미미

5. **데이터 무결성**
   - 자연 키: 오타/공백으로 중복 데이터 발생 가능
   - 대리 키: 시스템 생성으로 중복 불가능

**해결 방안**: Budget에도 ID 추가 (대리 키 사용) ⭐ 권장

**수정 파일**:
- `MLDefine.h` - FMLItemBudgetData에 BudgetId 필드 추가
- `IMLModel.h` - DeleteBudget/GetBudget 시그니처 변경 (ID 기반)
- `IMLController.h` - 동일하게 수정
- `IMLStorageProvider.h` - 동일하게 수정
- `MLSQLiteStorage.cpp` - budgets 테이블 스키마 변경, CRUD 수정
- `MLModel.h/cpp` - Budgets 자료구조 변경 (map<int, ...>)
- `MLController.cpp` - 구현 수정
- `MLMainFrame.h/cpp` - SelectedBudgetId만 추적
- 기존 DB 마이그레이션 스크립트

**수정 내용**:

```cpp
// 1. FMLItemBudgetData에 ID 추가
struct FMLItemBudgetData {
    int BudgetId = -1;           // 추가: 대리 키
    std::string Category;
    std::string Item;
    int64_t BudgetAmount = 0;
};

// 2. 인터페이스 메서드 변경
virtual bool DeleteBudget(const int budgetId) = 0;           // 변경: ID로 삭제
virtual FMLItemBudgetData GetBudget(const int budgetId) = 0; // 변경: ID로 조회

// 3. DB 스키마 변경
CREATE TABLE budgets (
    id INTEGER PRIMARY KEY AUTOINCREMENT,  -- 추가
    category TEXT NOT NULL,
    item TEXT NOT NULL,
    budget_amount INTEGER DEFAULT 0,
    UNIQUE(category, item)  -- 자연 키는 유니크 제약조건으로
);

// 4. View 멤버 변수 변경
int SelectedBudgetId = -1;  // Category, Item 대신 ID만 추적
```

**장점**:
- ✅ Transaction과 일관성 유지
- ✅ Category/Item 변경 용이 (UpdateBudget만 호출)
- ✅ UI 구현 간단 (ID만 추적)
- ✅ 데이터 무결성 향상
- ✅ 향후 Budget 간 관계(외래 키) 설정 가능

**주의사항**:
- 기존 budgets 테이블이 있다면 마이그레이션 필요
- Storage/Model/View 모두 수정 필요 (영향 범위 큼)

---

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
