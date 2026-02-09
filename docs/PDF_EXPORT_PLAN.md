# 예산안/결산안 PDF 내보내기 기능 구현 계획

> **⚠️ 2026-02-09 업데이트: 예산 구조 변경**
> 결산안.pdf 분석 결과, 예산 구조를 **카테고리별 → 항목별**로 변경 중입니다.
> 자세한 내용은 하단 "[2026-02-09 구조 변경사항](#2026-02-09-구조-변경사항)" 섹션 참조

## Context

MissionLedger는 재정 거래 관리 C++ 애플리케이션입니다. 사용자가 결산안.pdf와 같은 형식으로 예산안/결산안을 PDF로 내보낼 수 있는 기능이 필요합니다.

**현재 상황**:
- 거래 데이터: 수입/지출, 카테고리, 금액, 날짜, 환율 정보
- Summary 기능: 총 수입/지출/잔액 계산 (FMLTransactionSummary)
- 카테고리 관리: GetAllCategories() 메서드 존재
- 파일 메뉴: New/Open/Save/SaveAs 구현됨

**요구사항**:
1. 예산 관리: 카테고리별 예산 입력 (수입/지출 총액)
2. 탭 UI: "거래 내역" 탭과 "예산 관리" 탭 분리
3. 결산안: 카테고리별 집계, 환율 정보 포함
4. PDF 형식: 결산안.pdf 레이아웃 참고
   - Summary table (총 수입, 총 지출, 잔액)
   - Category grouping (카테고리별 소계)
   - Budget vs Actual comparison (예산 대비 실제)
   - Exchange rate information

## 핵심 설계 결정

### 1. 예산 데이터 구조: 카테고리별 단일 예산
**이유**: 실제 사용 패턴과 일치 ("숙박비 총 200만원" 같은 카테고리별 총액 예산)

```cpp
// MLDefine.h에 추가
struct FMLBudgetData {
    std::string Category;
    int64_t IncomeAmount = 0;   // 이 카테고리의 총 수입 예산
    int64_t ExpenseAmount = 0;  // 이 카테고리의 총 지출 예산
};

struct FMLCategorySummary {
    std::string Category;
    int64_t BudgetIncome = 0;      // budgets 테이블에서
    int64_t BudgetExpense = 0;     // budgets 테이블에서
    int64_t ActualIncome = 0;      // transactions 테이블에서 집계
    int64_t ActualExpense = 0;     // transactions 테이블에서 집계
    int64_t IncomeVariance = 0;    // 실제 - 예산
    int64_t ExpenseVariance = 0;   // 실제 - 예산
    int TransactionCount = 0;      // 실제 거래 건수
};

struct FMLPdfExportRequest {
    std::string FilePath;
    std::string Title;              // "2024 필리핀 단기 선교 결산안"
    std::string DateRange;          // "2024-01-01 ~ 2024-12-31"
    bool IncludeBudget = true;
    bool IncludeExchangeRate = true;
    bool GroupByCategory = true;
    FMLFilterCriteria FilterCriteria;
};
```

### 2. UI 구조: 탭 방식
```
메인 프레임
┌─────────────────────────────────────────┐
│ 파일(F)  편집(E)  보기(V)  도구(T)  도움말(H) │
├─────────────────────────────────────────┤
│ [거래 내역] [예산 관리]                   │ ← wxNotebook 탭
├─────────────────────────────────────────┤
│                                         │
│  ┌─ 예산 관리 탭 ──────────────────┐    │
│  │ 카테고리    수입 예산    지출 예산   │    │
│  │ ─────────────────────────────  │    │
│  │ 숙박비           0      2,000,000 │    │
│  │ 식비             0      1,500,000 │    │
│  │ 교통비           0        800,000 │    │
│  │                                  │    │
│  │ [추가] [수정] [삭제]               │    │
│  └─────────────────────────────────┘    │
│                                         │
└─────────────────────────────────────────┘
```

**장점**:
- ✅ 접근성: 클릭 한 번으로 예산 확인
- ✅ 비교 용이: 거래 내역 ↔ 예산 관리 탭 전환으로 즉시 비교
- ✅ 일관된 UX: 모든 작업이 한 창 안에서 이루어짐

### 3. PDF 라이브러리: wxPdfDocument
- wxWidgets 네이티브 통합
- 한글 폰트 지원 (Malgun Gothic / NanumGothic)
- 설치: `vcpkg install wxpdfdoc:x64-windows`

### 4. 메뉴 구조
```
파일(F)
  ├─ 내보내기(E) >
  │   └─ PDF로 내보내기(P)... Ctrl+E
```

## 구현 계획

### Phase 1: 데이터 구조 및 DB 스키마 (2.5-3.5시간)

#### 1.1 데이터 구조 추가

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\MLDefine.h`
```cpp
// 새 구조체 추가
struct FMLBudgetData {
    std::string Category;
    int64_t IncomeAmount = 0;
    int64_t ExpenseAmount = 0;
};

struct FMLCategorySummary {
    std::string Category;
    int64_t BudgetIncome = 0;
    int64_t BudgetExpense = 0;
    int64_t ActualIncome = 0;
    int64_t ActualExpense = 0;
    int64_t IncomeVariance = 0;
    int64_t ExpenseVariance = 0;
    int TransactionCount = 0;
};

struct FMLPdfExportRequest {
    std::string FilePath;
    std::string Title;
    std::string DateRange;
    bool IncludeBudget = true;
    bool IncludeExchangeRate = true;
    bool GroupByCategory = true;
    FMLFilterCriteria FilterCriteria;
};
```

#### 1.2 예산 Entity 추가

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\model\budget\MLBudget.h`
```cpp
#pragma once

#include "MLDefine.h"

// 예산 Entity (Data-Oriented Design)
class FMLBudget {
private:
    FMLBudgetData Data;

public:
    FMLBudget(const FMLBudgetData& data) : Data(data) {}

    // DTO 접근
    const FMLBudgetData& GetData() const { return Data; }
    void SetData(const FMLBudgetData& data) { Data = data; }

    // 비즈니스 로직
    bool IsValid() const {
        return !Data.Category.empty() &&
               Data.IncomeAmount >= 0 &&
               Data.ExpenseAmount >= 0;
    }

    // 예산 사용률 계산
    double GetIncomeUtilizationRate(int64_t actualIncome) const {
        if (Data.IncomeAmount == 0) return 0.0;
        return static_cast<double>(actualIncome) / Data.IncomeAmount * 100.0;
    }

    double GetExpenseUtilizationRate(int64_t actualExpense) const {
        if (Data.ExpenseAmount == 0) return 0.0;
        return static_cast<double>(actualExpense) / Data.ExpenseAmount * 100.0;
    }

    // 예산 초과 확인
    bool IsIncomeOverBudget(int64_t actualIncome) const {
        return actualIncome > Data.IncomeAmount;
    }

    bool IsExpenseOverBudget(int64_t actualExpense) const {
        return actualExpense > Data.ExpenseAmount;
    }
};
```

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\model\budget\MLBudget.cpp`
```cpp
#include "MLBudget.h"

// 필요 시 추가 구현
// 현재는 헤더에 모든 구현이 inline으로 되어 있음
```

#### 1.3 DB 스키마 추가

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\interface\IMLStorageProvider.h`
```cpp
// IMLStorageProvider 인터페이스에 새 메서드 추가
virtual bool SaveBudget(const FMLBudgetData& budget) = 0;
virtual bool LoadAllBudgets(std::vector<FMLBudgetData>& outBudgets) = 0;
virtual bool DeleteBudget(const std::string& category) = 0;
```

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\module\storage\MLSQLiteStorage.cpp`
```cpp
// createTable() 수정 - budgets 테이블 추가
bool FMLSQLiteStorage::createTable()
{
    // 기존 transactions 테이블 생성 코드...

    // budgets 테이블 생성
    const char* budgetSql = R"(
        CREATE TABLE IF NOT EXISTS budgets (
            category TEXT PRIMARY KEY,
            income_amount INTEGER DEFAULT 0,
            expense_amount INTEGER DEFAULT 0
        );
    )";

    char* errorMsg = nullptr;
    int result = sqlite3_exec(Database, budgetSql, nullptr, nullptr, &errorMsg);
    if (result != SQLITE_OK) {
        sqlite3_free(errorMsg);
        return false;
    }

    return true;
}

// SaveBudget 구현
bool FMLSQLiteStorage::SaveBudget(const FMLBudgetData& budget)
{
    if (!IsInitialized) return false;

    const char* sql = R"(
        INSERT OR REPLACE INTO budgets (category, income_amount, expense_amount)
        VALUES (?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(Database, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, budget.Category.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, budget.IncomeAmount);
    sqlite3_bind_int64(stmt, 3, budget.ExpenseAmount);

    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return result == SQLITE_DONE;
}

// LoadAllBudgets 구현
bool FMLSQLiteStorage::LoadAllBudgets(std::vector<FMLBudgetData>& outBudgets)
{
    if (!IsInitialized) return false;

    outBudgets.clear();

    const char* sql = "SELECT category, income_amount, expense_amount FROM budgets ORDER BY category;";

    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(Database, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) return false;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        FMLBudgetData budget;
        budget.Category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        budget.IncomeAmount = sqlite3_column_int64(stmt, 1);
        budget.ExpenseAmount = sqlite3_column_int64(stmt, 2);
        outBudgets.push_back(budget);
    }

    sqlite3_finalize(stmt);
    return true;
}

// DeleteBudget 구현
bool FMLSQLiteStorage::DeleteBudget(const std::string& category)
{
    if (!IsInitialized) return false;

    const char* sql = "DELETE FROM budgets WHERE category = ?;";

    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(Database, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, category.c_str(), -1, SQLITE_TRANSIENT);
    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return result == SQLITE_DONE;
}
```

### Phase 2: Model 계층 구현 (3-4시간)

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\interface\IMLModel.h`
```cpp
// 새 메서드 선언
virtual bool AddBudget(const FMLBudgetData& budget) = 0;
virtual bool UpdateBudget(const FMLBudgetData& budget) = 0;
virtual bool DeleteBudget(const std::string& category) = 0;
virtual std::vector<FMLBudgetData> GetAllBudgets() const = 0;
virtual FMLBudgetData GetBudget(const std::string& category) const = 0;

virtual std::map<std::string, FMLCategorySummary> GetCategorySummary() const = 0;
virtual std::map<std::string, FMLCategorySummary> GetFilteredCategorySummary(
    const FMLFilterCriteria& criteria) const = 0;
virtual std::map<std::string, double> GetUsedExchangeRates() const = 0;
```

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\model\MLModel.h`
```cpp
// 멤버 변수 추가
private:
    std::map<std::string, std::shared_ptr<FMLBudget>> Budgets;  // 카테고리별 예산 Entity
```

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\model\MLModel.cpp`
```cpp
// 예산 CRUD 구현
bool FMLModel::AddBudget(const FMLBudgetData& budget)
{
    // Entity 생성 및 검증
    auto budgetEntity = std::make_shared<FMLBudget>(budget);
    if (!budgetEntity->IsValid()) {
        return false;
    }

    Budgets[budget.Category] = budgetEntity;

    if (StorageProvider && StorageProvider->IsReady()) {
        return StorageProvider->SaveBudget(budget);
    }
    return true;
}

bool FMLModel::UpdateBudget(const FMLBudgetData& budget)
{
    return AddBudget(budget);  // INSERT OR REPLACE
}

bool FMLModel::DeleteBudget(const std::string& category)
{
    Budgets.erase(category);

    if (StorageProvider && StorageProvider->IsReady()) {
        return StorageProvider->DeleteBudget(category);
    }
    return true;
}

std::vector<FMLBudgetData> FMLModel::GetAllBudgets() const
{
    std::vector<FMLBudgetData> result;
    for (const auto& [category, budgetEntity] : Budgets) {
        result.push_back(budgetEntity->GetData());
    }
    return result;
}

FMLBudgetData FMLModel::GetBudget(const std::string& category) const
{
    auto it = Budgets.find(category);
    if (it != Budgets.end()) {
        return it->second->GetData();
    }
    return FMLBudgetData{category, 0, 0};
}

// 카테고리별 집계 구현
std::map<std::string, FMLCategorySummary> FMLModel::GetCategorySummary() const
{
    std::map<std::string, FMLCategorySummary> result;

    // 1. 예산 데이터 로드
    for (const auto& [category, budgetEntity] : Budgets) {
        const auto& budgetData = budgetEntity->GetData();
        auto& summary = result[category];
        summary.Category = category;
        summary.BudgetIncome = budgetData.IncomeAmount;
        summary.BudgetExpense = budgetData.ExpenseAmount;
    }

    // 2. 실제 거래 집계
    for (const auto& [id, transaction] : Transactions) {
        const auto& data = transaction->GetData();
        auto& summary = result[data.Category];

        if (summary.Category.empty()) {
            summary.Category = data.Category;
        }

        summary.TransactionCount++;
        if (data.Type == E_MLTransactionType::Income) {
            summary.ActualIncome += data.Amount;
        } else {
            summary.ActualExpense += data.Amount;
        }
    }

    // 3. 차이 계산
    for (auto& [category, summary] : result) {
        summary.IncomeVariance = summary.ActualIncome - summary.BudgetIncome;
        summary.ExpenseVariance = summary.ActualExpense - summary.BudgetExpense;
    }

    return result;
}

std::map<std::string, FMLCategorySummary> FMLModel::GetFilteredCategorySummary(
    const FMLFilterCriteria& criteria) const
{
    std::map<std::string, FMLCategorySummary> result;

    // 1. 예산 데이터 로드 (필터 무관)
    for (const auto& [category, budgetEntity] : Budgets) {
        const auto& budgetData = budgetEntity->GetData();
        auto& summary = result[category];
        summary.Category = category;
        summary.BudgetIncome = budgetData.IncomeAmount;
        summary.BudgetExpense = budgetData.ExpenseAmount;
    }

    // 2. 필터링된 거래만 집계
    for (const auto& [id, transaction] : Transactions) {
        if (!transaction->MatchesFilter(criteria)) continue;

        const auto& data = transaction->GetData();
        auto& summary = result[data.Category];

        if (summary.Category.empty()) {
            summary.Category = data.Category;
        }

        summary.TransactionCount++;
        if (data.Type == E_MLTransactionType::Income) {
            summary.ActualIncome += data.Amount;
        } else {
            summary.ActualExpense += data.Amount;
        }
    }

    // 3. 차이 계산
    for (auto& [category, summary] : result) {
        summary.IncomeVariance = summary.ActualIncome - summary.BudgetIncome;
        summary.ExpenseVariance = summary.ActualExpense - summary.BudgetExpense;
    }

    return result;
}

std::map<std::string, double> FMLModel::GetUsedExchangeRates() const
{
    std::map<std::string, double> rates;
    for (const auto& [id, transaction] : Transactions) {
        const auto& data = transaction->GetData();
        if (data.UseExchangeRate && data.Currency != "KRW") {
            rates[data.Currency] = data.ExchangeRate;
        }
    }
    return rates;
}

// LoadFile 수정 - 예산 데이터도 로드
bool FMLModel::LoadFile(const std::string& filePath)
{
    // ... 기존 거래 로드 코드 ...

    // 예산 데이터 로드
    std::vector<FMLBudgetData> budgets;
    if (StorageProvider->LoadAllBudgets(budgets)) {
        Budgets.clear();
        for (const auto& budgetData : budgets) {
            auto budgetEntity = std::make_shared<FMLBudget>(budgetData);
            Budgets[budgetData.Category] = budgetEntity;
        }
    }

    return true;
}
```

### Phase 3: wxPdfDocument 설치 및 설정 (1-2시간)

**vcpkg 설치**:
```bash
vcpkg install wxpdfdoc:x64-windows
```

**프로젝트 설정** (MissionLedger.vcxproj):
```xml
<AdditionalIncludeDirectories>
  $(VCPKG_ROOT)\installed\x64-windows\include;
</AdditionalIncludeDirectories>

<AdditionalLibraryDirectories>
  $(VCPKG_ROOT)\installed\x64-windows\lib;
</AdditionalLibraryDirectories>

<AdditionalDependencies>
  wxpdfdoc.lib;
</AdditionalDependencies>
```

**폰트 테스트**:
```cpp
wxPdfDocument pdf;
pdf.AddFont("MalgunGothic", "", "C:\\Windows\\Fonts\\malgun.ttf");
pdf.SetFont("MalgunGothic", "", 12);
pdf.AddPage();
pdf.Cell(0, 10, wxString::FromUTF8("한글 테스트"), 0, 1);
pdf.SaveAsFile("test.pdf");
```

### Phase 4: View 계층 구현 (6-8시간)

#### 4.1 탭 UI 추가

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\view\MLMainFrame.h`
```cpp
class wxMLMainFrame : public wxFrame, public IMLView, public IMLModelObserver
{
private:
    // 탭 관련
    wxNotebook* notebook;
    wxPanel* transactionPanel;  // 거래 내역 탭
    wxPanel* budgetPanel;       // 예산 관리 탭

    // 예산 관리 탭 컨트롤
    wxListCtrl* budgetListCtrl;
    wxButton* addBudgetButton;
    wxButton* editBudgetButton;
    wxButton* deleteBudgetButton;

    // 탭 생성 메서드
    void createNotebook(wxBoxSizer* mainSizer);
    wxPanel* createTransactionPanel();
    wxPanel* createBudgetPanel();

    // 예산 관리 이벤트
    void OnAddBudget(wxCommandEvent& event);
    void OnEditBudget(wxCommandEvent& event);
    void OnDeleteBudget(wxCommandEvent& event);
    void OnBudgetListItemSelected(wxListEvent& event);

    // 예산 UI 업데이트
    void refreshBudgetList();
    void displayBudget(const FMLBudgetData& budget);
};
```

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\view\MLMainFrame.cpp`
```cpp
// 생성자 수정
wxMLMainFrame::wxMLMainFrame()
    : wxFrame(nullptr, wxID_ANY, wxString::FromUTF8("MissionLedger"))
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    createMenuBar();
    createNotebook(mainSizer);  // 탭 UI 생성

    SetSizer(mainSizer);
    SetSize(1000, 700);
}

// 탭 생성
void wxMLMainFrame::createNotebook(wxBoxSizer* mainSizer)
{
    notebook = new wxNotebook(this, wxID_ANY);

    // 탭 1: 거래 내역
    transactionPanel = createTransactionPanel();
    notebook->AddPage(transactionPanel, wxString::FromUTF8("거래 내역"), true);

    // 탭 2: 예산 관리
    budgetPanel = createBudgetPanel();
    notebook->AddPage(budgetPanel, wxString::FromUTF8("예산 관리"), false);

    mainSizer->Add(notebook, 1, wxEXPAND | wxALL, 5);
}

// 거래 내역 탭 생성
wxPanel* wxMLMainFrame::createTransactionPanel()
{
    wxPanel* panel = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // 기존 거래 입력 UI 이동
    createInputPanel(sizer);
    createFilterPanel(sizer);
    createListPanel(sizer);
    createSummaryPanel(sizer);

    panel->SetSizer(sizer);
    return panel;
}

// 예산 관리 탭 생성
wxPanel* wxMLMainFrame::createBudgetPanel()
{
    wxPanel* panel = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // 예산 리스트
    budgetListCtrl = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                     wxLC_REPORT | wxLC_SINGLE_SEL);
    budgetListCtrl->AppendColumn(wxString::FromUTF8("카테고리"), wxLIST_FORMAT_LEFT, 200);
    budgetListCtrl->AppendColumn(wxString::FromUTF8("수입 예산"), wxLIST_FORMAT_RIGHT, 150);
    budgetListCtrl->AppendColumn(wxString::FromUTF8("지출 예산"), wxLIST_FORMAT_RIGHT, 150);

    sizer->Add(budgetListCtrl, 1, wxEXPAND | wxALL, 5);

    // 버튼 패널
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    addBudgetButton = new wxButton(panel, wxID_ANY, wxString::FromUTF8("추가"));
    editBudgetButton = new wxButton(panel, wxID_ANY, wxString::FromUTF8("수정"));
    deleteBudgetButton = new wxButton(panel, wxID_ANY, wxString::FromUTF8("삭제"));

    buttonSizer->Add(addBudgetButton, 0, wxALL, 5);
    buttonSizer->Add(editBudgetButton, 0, wxALL, 5);
    buttonSizer->Add(deleteBudgetButton, 0, wxALL, 5);

    sizer->Add(buttonSizer, 0, wxALIGN_CENTER);

    // 이벤트 바인딩
    addBudgetButton->Bind(wxEVT_BUTTON, &wxMLMainFrame::OnAddBudget, this);
    editBudgetButton->Bind(wxEVT_BUTTON, &wxMLMainFrame::OnEditBudget, this);
    deleteBudgetButton->Bind(wxEVT_BUTTON, &wxMLMainFrame::OnDeleteBudget, this);
    budgetListCtrl->Bind(wxEVT_LIST_ITEM_SELECTED, &wxMLMainFrame::OnBudgetListItemSelected, this);

    panel->SetSizer(sizer);
    return panel;
}

// 예산 추가 이벤트
void wxMLMainFrame::OnAddBudget(wxCommandEvent& event)
{
    wxTextEntryDialog categoryDialog(this,
        wxString::FromUTF8("카테고리를 입력하세요:"),
        wxString::FromUTF8("예산 추가"));

    if (categoryDialog.ShowModal() != wxID_OK) return;

    const wxString category = categoryDialog.GetValue();
    if (category.IsEmpty()) return;

    // 수입 예산 입력
    wxTextEntryDialog incomeDialog(this,
        wxString::FromUTF8("수입 예산을 입력하세요:"),
        wxString::FromUTF8("수입 예산"));
    incomeDialog.SetValue("0");
    if (incomeDialog.ShowModal() != wxID_OK) return;

    // 지출 예산 입력
    wxTextEntryDialog expenseDialog(this,
        wxString::FromUTF8("지출 예산을 입력하세요:"),
        wxString::FromUTF8("지출 예산"));
    expenseDialog.SetValue("0");
    if (expenseDialog.ShowModal() != wxID_OK) return;

    FMLBudgetData budget;
    budget.Category = category.ToUTF8().data();
    budget.IncomeAmount = wxAtoll(incomeDialog.GetValue());
    budget.ExpenseAmount = wxAtoll(expenseDialog.GetValue());

    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (controller->AddBudget(budget)) {
        refreshBudgetList();
    }
}

// 예산 수정 이벤트
void wxMLMainFrame::OnEditBudget(wxCommandEvent& event)
{
    const long selected = budgetListCtrl->GetFirstSelected();
    if (selected == -1) return;

    const wxString category = budgetListCtrl->GetItemText(selected, 0);

    // 현재 값 가져오기
    auto controller = FMLMVCHolder::GetInstance().GetController();
    const FMLBudgetData currentBudget = controller->GetBudget(category.ToUTF8().data());

    // 수입 예산 수정
    wxTextEntryDialog incomeDialog(this,
        wxString::FromUTF8("수입 예산을 입력하세요:"),
        wxString::FromUTF8("수입 예산"));
    incomeDialog.SetValue(wxString::Format("%lld", currentBudget.IncomeAmount));
    if (incomeDialog.ShowModal() != wxID_OK) return;

    // 지출 예산 수정
    wxTextEntryDialog expenseDialog(this,
        wxString::FromUTF8("지출 예산을 입력하세요:"),
        wxString::FromUTF8("지출 예산"));
    expenseDialog.SetValue(wxString::Format("%lld", currentBudget.ExpenseAmount));
    if (expenseDialog.ShowModal() != wxID_OK) return;

    FMLBudgetData budget;
    budget.Category = category.ToUTF8().data();
    budget.IncomeAmount = wxAtoll(incomeDialog.GetValue());
    budget.ExpenseAmount = wxAtoll(expenseDialog.GetValue());

    if (controller->UpdateBudget(budget)) {
        refreshBudgetList();
    }
}

// 예산 삭제 이벤트
void wxMLMainFrame::OnDeleteBudget(wxCommandEvent& event)
{
    const long selected = budgetListCtrl->GetFirstSelected();
    if (selected == -1) return;

    const wxString category = budgetListCtrl->GetItemText(selected, 0);

    const int answer = wxMessageBox(
        wxString::FromUTF8("선택한 예산을 삭제하시겠습니까?"),
        wxString::FromUTF8("삭제 확인"),
        wxYES_NO | wxICON_QUESTION);

    if (answer != wxYES) return;

    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (controller->DeleteBudget(category.ToUTF8().data())) {
        refreshBudgetList();
    }
}

// 예산 리스트 갱신
void wxMLMainFrame::refreshBudgetList()
{
    budgetListCtrl->DeleteAllItems();

    auto controller = FMLMVCHolder::GetInstance().GetController();
    const auto budgets = controller->GetAllBudgets();

    for (const auto& budget : budgets) {
        displayBudget(budget);
    }
}

// 예산 표시
void wxMLMainFrame::displayBudget(const FMLBudgetData& budget)
{
    const long index = budgetListCtrl->InsertItem(budgetListCtrl->GetItemCount(),
        wxString::FromUTF8(budget.Category));

    budgetListCtrl->SetItem(index, 1, formatAmountWithComma(budget.IncomeAmount));
    budgetListCtrl->SetItem(index, 2, formatAmountWithComma(budget.ExpenseAmount));
}
```

#### 4.2 메뉴 확장

```cpp
enum {
    // ... 기존 ID
    ID_EXPORT_PDF = wxID_HIGHEST + 100,
};

void wxMLMainFrame::createMenuBar()
{
    // ... 기존 파일 메뉴 코드 ...

    // 내보내기 서브메뉴
    wxMenu* exportMenu = new wxMenu();
    exportMenu->Append(ID_EXPORT_PDF, wxString::FromUTF8("PDF로 내보내기(&P)...\tCtrl+E"));
    fileMenu->AppendSubMenu(exportMenu, wxString::FromUTF8("내보내기(&E)"));

    Bind(wxEVT_MENU, &wxMLMainFrame::OnExportPdf, this, ID_EXPORT_PDF);
}
```

#### 4.3 PDF 내보내기 다이얼로그 (새 파일)

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\view\MLPdfExportDialog.h`
**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\view\MLPdfExportDialog.cpp`

```cpp
class MLPdfExportDialog : public wxDialog {
public:
    MLPdfExportDialog(wxWindow* parent);
    FMLPdfExportRequest GetExportRequest() const;

private:
    wxTextCtrl* titleText;
    wxDatePickerCtrl* startDatePicker;
    wxDatePickerCtrl* endDatePicker;
    wxCheckBox* includeBudgetCheck;
    wxCheckBox* includeExchangeRateCheck;
    wxCheckBox* groupByCategoryCheck;
    wxTextCtrl* filePathText;
    wxButton* browseButton;

    void OnBrowse(wxCommandEvent& event);
};
```

#### 4.4 PDF 생성 로직

```cpp
void wxMLMainFrame::OnExportPdf(wxCommandEvent& event)
{
    MLPdfExportDialog dialog(this);
    if (dialog.ShowModal() != wxID_OK) return;

    auto request = dialog.GetExportRequest();
    auto controller = FMLMVCHolder::GetInstance().GetController();

    if (controller->ExportToPdf(request)) {
        wxMessageBox(wxString::FromUTF8("PDF 파일이 성공적으로 생성되었습니다."),
                     wxString::FromUTF8("완료"), wxOK | wxICON_INFORMATION);
    } else {
        wxMessageBox(wxString::FromUTF8("PDF 생성에 실패했습니다."),
                     wxString::FromUTF8("오류"), wxOK | wxICON_ERROR);
    }
}

bool wxMLMainFrame::GeneratePdfReport(
    const FMLPdfExportRequest& request,
    const std::vector<FMLTransactionData>& transactions,
    const std::map<std::string, FMLCategorySummary>& categorySummary,
    const std::map<std::string, double>& exchangeRates)
{
    wxPdfDocument pdf;

    // 1. 폰트 설정
    if (!pdf.AddFont("MalgunGothic", "", "C:\\Windows\\Fonts\\malgun.ttf"))
        return false;
    pdf.SetFont("MalgunGothic", "", 12);

    // 2. 페이지 추가 및 헤더
    pdf.AddPage();
    pdf.SetFontSize(16);
    pdf.Cell(0, 10, wxString::FromUTF8(request.Title), 0, 1, wxPDF_ALIGN_CENTER);
    pdf.SetFontSize(12);
    pdf.Cell(0, 8, wxString::FromUTF8(request.DateRange), 0, 1, wxPDF_ALIGN_CENTER);
    pdf.Ln(5);

    // 3. 수입 섹션
    renderIncomeSection(pdf, categorySummary);

    // 4. 지출 섹션 (카테고리별 그룹화)
    renderExpenseSection(pdf, categorySummary, transactions);

    // 5. 요약 섹션
    renderSummarySection(pdf, categorySummary);

    // 6. 환율 정보
    if (request.IncludeExchangeRate && !exchangeRates.empty()) {
        renderExchangeRateInfo(pdf, exchangeRates);
    }

    return pdf.SaveAsFile(wxString::FromUTF8(request.FilePath));
}
```

**렌더링 헬퍼 메서드**:
- `renderIncomeSection()`: 수입 테이블 (카테고리, 예산, 실제, 차이)
- `renderExpenseSection()`: 지출 카테고리별 그룹 (소계 포함)
- `renderSummarySection()`: 총 수입/지출/잔액
- `renderExchangeRateInfo()`: 환율 정보 (예: "PHP 환율: 24.84")

### Phase 5: Controller 계층 구현 (1-2시간)

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\interface\IMLController.h`
```cpp
// 예산 관리 메서드
virtual bool AddBudget(const FMLBudgetData& budget) = 0;
virtual bool UpdateBudget(const FMLBudgetData& budget) = 0;
virtual bool DeleteBudget(const std::string& category) = 0;
virtual std::vector<FMLBudgetData> GetAllBudgets() const = 0;
virtual FMLBudgetData GetBudget(const std::string& category) const = 0;

// PDF 내보내기 메서드
virtual std::map<std::string, FMLCategorySummary> GetCategorySummary() = 0;
virtual std::map<std::string, FMLCategorySummary> GetFilteredCategorySummary(
    const FMLFilterCriteria& criteria) = 0;
virtual std::map<std::string, double> GetUsedExchangeRates() const = 0;
virtual bool ExportToPdf(const FMLPdfExportRequest& request) = 0;
```

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\controller\MLController.cpp`
```cpp
// 예산 관리 구현 (Model에 위임)
bool FMLController::AddBudget(const FMLBudgetData& budget)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    return model ? model->AddBudget(budget) : false;
}

bool FMLController::UpdateBudget(const FMLBudgetData& budget)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    return model ? model->UpdateBudget(budget) : false;
}

bool FMLController::DeleteBudget(const std::string& category)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    return model ? model->DeleteBudget(category) : false;
}

std::vector<FMLBudgetData> FMLController::GetAllBudgets() const
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    return model ? model->GetAllBudgets() : std::vector<FMLBudgetData>();
}

FMLBudgetData FMLController::GetBudget(const std::string& category) const
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    return model ? model->GetBudget(category) : FMLBudgetData{category, 0, 0};
}

// PDF 내보내기 구현
bool FMLController::ExportToPdf(const FMLPdfExportRequest& request)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (!model) return false;

    // 데이터 수집
    auto transactions = model->GetFilteredTransactionData(request.FilterCriteria);
    auto categorySummary = model->GetFilteredCategorySummary(request.FilterCriteria);
    auto exchangeRates = model->GetUsedExchangeRates();

    // View에 PDF 생성 요청
    auto view = FMLMVCHolder::GetInstance().GetView();
    return view->GeneratePdfReport(request, transactions, categorySummary, exchangeRates);
}
```

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\interface\IMLView.h`
```cpp
virtual bool GeneratePdfReport(
    const FMLPdfExportRequest& request,
    const std::vector<FMLTransactionData>& transactions,
    const std::map<std::string, FMLCategorySummary>& categorySummary,
    const std::map<std::string, double>& exchangeRates) = 0;
```

## 주요 파일 목록

**수정 파일**:
1. `C:\Work\Others\MissionLedger\MissionLedger\src\MLDefine.h` (FMLBudgetData, FMLCategorySummary 추가)
2. `C:\Work\Others\MissionLedger\MissionLedger\src\interface\IMLStorageProvider.h` (예산 인터페이스)
3. `C:\Work\Others\MissionLedger\MissionLedger\src\module\storage\MLSQLiteStorage.h` (예산 메서드 선언)
4. `C:\Work\Others\MissionLedger\MissionLedger\src\module\storage\MLSQLiteStorage.cpp` (budgets 테이블, 예산 CRUD)
5. `C:\Work\Others\MissionLedger\MissionLedger\src\interface\IMLModel.h` (예산 인터페이스)
6. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\model\MLModel.h` (Budgets 멤버 추가)
7. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\model\MLModel.cpp` (예산 구현, 집계 로직)
8. `C:\Work\Others\MissionLedger\MissionLedger\src\interface\IMLController.h` (예산/PDF 인터페이스)
9. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\controller\MLController.h` (헤더)
10. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\controller\MLController.cpp` (예산/PDF 구현)
11. `C:\Work\Others\MissionLedger\MissionLedger\src\interface\IMLView.h` (PDF 인터페이스)
12. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\view\MLMainFrame.h` (탭, 예산 UI)
13. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\view\MLMainFrame.cpp` (탭 구현, PDF 생성)

**새 파일**:
1. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\model\budget\MLBudget.h` (예산 Entity)
2. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\model\budget\MLBudget.cpp` (예산 Entity 구현)
3. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\view\MLPdfExportDialog.h`
4. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\view\MLPdfExportDialog.cpp`

## 예상 소요 시간

- Phase 1 (데이터 구조 및 DB): 2.5-3.5시간
- Phase 2 (Model 계층): 3-4시간
- Phase 3 (wxPdfDocument 설치): 1-2시간
- Phase 4 (View 계층 - 탭 UI): 6-8시간
- Phase 5 (Controller 계층): 1-2시간
- Phase 6 (테스트 및 디버깅): 3-4시간
- **총계**: 17-24.5시간 (약 2-3일)

## 검증 방법

### 단위 테스트
1. **Model 계층**
   - `AddBudget()`, `UpdateBudget()`, `DeleteBudget()` 테스트
   - `GetCategorySummary()` 테스트 (예산/실제 데이터 비교)
   - `GetUsedExchangeRates()` 테스트 (다중 통화)

### 통합 테스트
1. **데이터베이스**
   - 예산 데이터 저장/로드 (budgets 테이블)
   - 기존 파일 호환성 (budgets 테이블 없는 경우)

2. **탭 UI**
   - 탭 전환 동작 확인
   - 예산 추가/수정/삭제 확인
   - 예산 리스트 표시 확인

3. **PDF 생성**
   - 간단한 테스트 데이터로 PDF 생성
   - 한글 폰트 출력 검증
   - 카테고리별 그룹화 확인
   - 예산 vs 실제 비교 표시 확인
   - 환율 정보 표시 확인

### 수동 테스트
1. "예산 관리" 탭에서 카테고리별 예산 입력
2. "거래 내역" 탭에서 실제 거래 입력
3. 메뉴에서 "파일 → 내보내기 → PDF로 내보내기" 클릭
4. 다이얼로그에서 옵션 선택
5. PDF 파일 생성 및 열어서 확인
6. 결산안.pdf와 레이아웃 비교

## 기술적 고려사항

### 1. 페이지 넘김 처리
```cpp
if (pdf.GetY() > 250) {  // 페이지 하단 근처
    pdf.AddPage();
    renderTableHeader(pdf);  // 헤더 재출력
}
```

### 2. 환율 정보 표시
```
항목: 숙박비
금액: 2,270,128원 (PHP 91,400 × 24.84)
```

### 3. 천 단위 콤마 포맷팅
```cpp
wxString formatAmountWithComma(int64_t amount) {
    wxString result = wxString::Format("%lld", amount);
    int insertPos = result.length() - 3;
    while (insertPos > 0) {
        result.insert(insertPos, ",");
        insertPos -= 3;
    }
    return result;
}
```

### 4. UTF-8 인코딩
- 모든 한글 텍스트: `wxString::FromUTF8()`
- 파일 경로: `wxString::ToUTF8().data()`

### 5. MVC 경계 준수
- View → Controller → Model (단방향)
- PDF 렌더링은 View 책임 (Presentation Layer)
- Controller는 데이터 조율만 담당

### 6. 예산 데이터 관리
- 예산은 별도 테이블 (budgets)에 저장
- 카테고리당 하나의 예산만 존재
- 실제 거래는 기존 transactions 테이블 사용
- PDF 출력 시 두 데이터 소스를 결합하여 비교

## 향후 확장 가능성

1. **CSV 내보내기** (1-2시간)
   - 간단한 스프레드시트 형식
   - UTF-8 BOM 포함

2. **Excel (XLSX) 내보내기** (4-6시간)
   - libxlsxwriter 라이브러리
   - 셀 서식 지원

3. **차트 및 그래프**
   - 카테고리별 지출 비율 파이 차트
   - 월별 수입/지출 추이 그래프
   - 예산 vs 실제 비교 막대 그래프

4. **템플릿 시스템**
   - PDF 레이아웃 템플릿 저장/로드
   - 로고, 조직명 커스터마이징

5. **예산 관리 개선**
   - 월별 예산 관리
   - 예산 초과 경고
   - 예산 사용률 표시

6. **예산 입력 다이얼로그**
   - 현재: 간단한 텍스트 입력
   - 향후: 전용 다이얼로그 (여러 필드 한번에 입력, 검증)
