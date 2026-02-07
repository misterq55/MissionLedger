# 거래 내역 상세 보고서 PDF 내보내기 기능 구현 계획

## Context

MissionLedger에 **거래 내역 상세 보고서** PDF 내보내기 기능을 추가합니다. 이는 후원자나 교회에 제출하기 위한 보고서 형식으로, 예산 정보 없이 실제 거래 내역만을 상세히 표시합니다.

**참고 문서**: `결산안.csv` - 실제 사용 중인 보고서 형식

**현재 상황**:
- 거래 데이터: 수입/지출, 카테고리, 금액, 날짜, 환율 정보
- Summary 기능: 총 수입/지출/잔액 계산 (FMLTransactionSummary)
- 카테고리 관리: GetAllCategories() 메서드 존재
- PDF 라이브러리: wxPdfDocument (예산/결산 비교 보고서 기능과 공유)

**요구사항**:
1. 거래 내역 상세 보고서: 실제 거래만 표시 (예산 정보 제외)
2. PDF 형식: 결산안.csv 레이아웃 참고
   - 상단: 수입/지출 카테고리별 합계 테이블
   - 중단: 총계 (총수입, 총지출, 잔액)
   - 하단: 수입/지출 상세 내역 (양분할 레이아웃)
3. 메뉴: 파일 → 내보내기 → 거래 내역 상세 보고서

## 결산안.csv 패턴 분석

### 레이아웃 구조

```
┌─────────────────────────────────────────────────────┐
│ [1] 수입 카테고리별 합계                             │
│     회비 총합 | 후원 | 결산이자                      │
│     16,300,000 | 6,420,000 | 1,042                  │
├─────────────────────────────────────────────────────┤
│ [2] 지출 카테고리별 합계                             │
│     사역준비 | 팀사역 | 항공                         │
│     2,735,490 | 3,218,230 | 13,800,000              │
├─────────────────────────────────────────────────────┤
│ [3] 총계                                             │
│     총 수입 | 총 지출 | 남은 금액                    │
│     22,721,042 | 21,234,950 | 1,486,092             │
├─────────────────────────────────────────────────────┤
│ [4] 상세 내역 (양분할 레이아웃)                      │
│ ┌────────────────┬────────────────────────────────┐ │
│ │ 수입           │ 지출                           │ │
│ ├────────────────┼────────────────────────────────┤ │
│ │ 구분 | 내용 |  │ 구분 | 항목 | 내용 | 금액 |   │ │
│ │      | 금액 |  │      |      |      | 영수증 │ │ │
│ ├────────────────┼────────────────────────────────┤ │
│ │ 네네 | 박세영 │ 항공 | 항공료 | 선결제...    │ │
│ │      | 800,000│      | 13,800,000 |           │ │
│ │ ...            │ ...                            │ │
│ └────────────────┴────────────────────────────────┘ │
└─────────────────────────────────────────────────────┘
```

### 데이터 계층 구조

**수입 카테고리** (2단계):
```
네네 (회비)
  ├─ 박세영: 800,000
  ├─ 서문규: 800,000
  └─ ...

후원
  ├─ 당회 지원: 200,000
  ├─ 목적 헌금: 4,220,000
  └─ 재정 지원: 2,000,000

없음 (기타)
  └─ 결산 이자: 1,042
```

**지출 카테고리** (3단계):
```
항공
  ├─ 항공료: 13,800,000
  └─ 여행자 보험: 277,490

사역준비
  ├─ 사역팀 모임비
  │   ├─ 맥도날드: 42,300
  │   ├─ BBQ: 52,500
  │   └─ ...
  ├─ 전체 모임비
  ├─ 사역팀 로밍
  ├─ 인쇄물
  ├─ 단체티
  └─ 여행 간식비

팀사역
  ├─ 어린이 사역
  ├─ 의료 사역
  ├─ 찬양
  ├─ 보수
  ├─ 공연 사역
  └─ 기타
```

## 핵심 설계 결정

### 1. 데이터 구조

```cpp
// MLDefine.h에 추가
struct FMLTransactionReportRequest {
    std::string FilePath;
    std::string Title;              // "2024 필리핀 단기 선교 결산안"
    std::string DateRange;          // "2024-01-01 ~ 2024-12-31"
    bool IncludeExchangeRate = true;
    bool GroupByCategory = true;
    FMLFilterCriteria FilterCriteria;
};

struct FMLCategoryTotal {
    std::string Category;
    int64_t IncomeTotal = 0;
    int64_t ExpenseTotal = 0;
    int TransactionCount = 0;
};
```

### 2. PDF 레이아웃: 결산안.csv 패턴

**특징**:
- ✅ 수입/지출을 좌우로 분할 배치
- ✅ 카테고리별 계층 구조 (수입 2단계, 지출 3단계)
- ✅ 상단에 카테고리별 합계 요약
- ✅ 환율 정보는 금액 옆에 표시 (예: "2,270,128원 (PHP 91,400 × 24.84)")

### 3. 메뉴 구조

```
파일(F)
  ├─ 내보내기(E) >
  │   ├─ 예산/결산 비교 보고서(B)... Ctrl+E
  │   ├─ 거래 내역 상세 보고서(D)... Ctrl+Shift+E
  │   └─ ─────────────────
  │   └─ CSV로 내보내기(C)... (향후)
```

## 구현 계획

### Phase 1: 데이터 구조 추가 (30분)

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\MLDefine.h`
```cpp
// 새 구조체 추가
struct FMLTransactionReportRequest {
    std::string FilePath;
    std::string Title;
    std::string DateRange;
    bool IncludeExchangeRate = true;
    bool GroupByCategory = true;
    FMLFilterCriteria FilterCriteria;
};

struct FMLCategoryTotal {
    std::string Category;
    int64_t IncomeTotal = 0;
    int64_t ExpenseTotal = 0;
    int TransactionCount = 0;
};
```

### Phase 2: Model 계층 구현 (1-2시간)

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\interface\IMLModel.h`
```cpp
// 새 메서드 선언
virtual std::map<std::string, FMLCategoryTotal> GetCategoryTotals() const = 0;
virtual std::map<std::string, FMLCategoryTotal> GetFilteredCategoryTotals(
    const FMLFilterCriteria& criteria) const = 0;
```

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\model\MLModel.cpp`
```cpp
// 카테고리별 합계 계산
std::map<std::string, FMLCategoryTotal> FMLModel::GetCategoryTotals() const
{
    std::map<std::string, FMLCategoryTotal> result;

    for (const auto& [id, transaction] : Transactions) {
        const auto& data = transaction->GetData();
        auto& total = result[data.Category];

        total.Category = data.Category;
        total.TransactionCount++;

        if (data.Type == E_MLTransactionType::Income) {
            total.IncomeTotal += data.Amount;
        } else {
            total.ExpenseTotal += data.Amount;
        }
    }

    return result;
}

std::map<std::string, FMLCategoryTotal> FMLModel::GetFilteredCategoryTotals(
    const FMLFilterCriteria& criteria) const
{
    std::map<std::string, FMLCategoryTotal> result;

    for (const auto& [id, transaction] : Transactions) {
        if (!transaction->MatchesFilter(criteria)) continue;

        const auto& data = transaction->GetData();
        auto& total = result[data.Category];

        total.Category = data.Category;
        total.TransactionCount++;

        if (data.Type == E_MLTransactionType::Income) {
            total.IncomeTotal += data.Amount;
        } else {
            total.ExpenseTotal += data.Amount;
        }
    }

    return result;
}
```

### Phase 3: View 계층 구현 (4-5시간)

#### 3.1 메뉴 확장

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\view\MLMainFrame.cpp`
```cpp
enum {
    // ... 기존 ID
    ID_EXPORT_PDF_BUDGET = wxID_HIGHEST + 100,      // 예산/결산 비교
    ID_EXPORT_PDF_TRANSACTION = wxID_HIGHEST + 101, // 거래 내역 상세
};

void wxMLMainFrame::createMenuBar()
{
    // ... 기존 파일 메뉴 코드 ...

    // 내보내기 서브메뉴
    wxMenu* exportMenu = new wxMenu();
    exportMenu->Append(ID_EXPORT_PDF_BUDGET,
        wxString::FromUTF8("예산/결산 비교 보고서(&B)...\tCtrl+E"));
    exportMenu->Append(ID_EXPORT_PDF_TRANSACTION,
        wxString::FromUTF8("거래 내역 상세 보고서(&D)...\tCtrl+Shift+E"));

    fileMenu->AppendSubMenu(exportMenu, wxString::FromUTF8("내보내기(&E)"));

    Bind(wxEVT_MENU, &wxMLMainFrame::OnExportPdfBudget, this, ID_EXPORT_PDF_BUDGET);
    Bind(wxEVT_MENU, &wxMLMainFrame::OnExportPdfTransaction, this, ID_EXPORT_PDF_TRANSACTION);
}
```

#### 3.2 거래 내역 상세 보고서 다이얼로그

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\view\MLTransactionReportDialog.h`
**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\view\MLTransactionReportDialog.cpp`

```cpp
class MLTransactionReportDialog : public wxDialog {
public:
    MLTransactionReportDialog(wxWindow* parent);
    FMLTransactionReportRequest GetExportRequest() const;

private:
    wxTextCtrl* titleText;
    wxDatePickerCtrl* startDatePicker;
    wxDatePickerCtrl* endDatePicker;
    wxCheckBox* includeExchangeRateCheck;
    wxCheckBox* groupByCategoryCheck;
    wxTextCtrl* filePathText;
    wxButton* browseButton;

    void OnBrowse(wxCommandEvent& event);
};
```

#### 3.3 PDF 생성 로직

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\view\MLMainFrame.h`
```cpp
// 이벤트 핸들러 추가
void OnExportPdfTransaction(wxCommandEvent& event);

// PDF 렌더링 메서드
bool GenerateTransactionReport(
    const FMLTransactionReportRequest& request,
    const std::vector<FMLTransactionData>& transactions,
    const std::map<std::string, FMLCategoryTotal>& categoryTotals,
    const FMLTransactionSummary& summary,
    const std::map<std::string, double>& exchangeRates);

// 렌더링 헬퍼 메서드
void renderCategorySummaryTables(wxPdfDocument& pdf,
    const std::map<std::string, FMLCategoryTotal>& categoryTotals);
void renderGrandTotalTable(wxPdfDocument& pdf, const FMLTransactionSummary& summary);
void renderTransactionDetails(wxPdfDocument& pdf,
    const std::vector<FMLTransactionData>& transactions,
    const std::map<std::string, double>& exchangeRates);
void renderIncomeColumn(wxPdfDocument& pdf, double startX, double startY, double width,
    const std::vector<FMLTransactionData>& incomeTransactions);
void renderExpenseColumn(wxPdfDocument& pdf, double startX, double startY, double width,
    const std::vector<FMLTransactionData>& expenseTransactions,
    const std::map<std::string, double>& exchangeRates);
```

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\view\MLMainFrame.cpp`
```cpp
void wxMLMainFrame::OnExportPdfTransaction(wxCommandEvent& event)
{
    MLTransactionReportDialog dialog(this);
    if (dialog.ShowModal() != wxID_OK) return;

    auto request = dialog.GetExportRequest();
    auto controller = FMLMVCHolder::GetInstance().GetController();

    if (controller->ExportTransactionReport(request)) {
        wxMessageBox(wxString::FromUTF8("PDF 파일이 성공적으로 생성되었습니다."),
                     wxString::FromUTF8("완료"), wxOK | wxICON_INFORMATION);
    } else {
        wxMessageBox(wxString::FromUTF8("PDF 생성에 실패했습니다."),
                     wxString::FromUTF8("오류"), wxOK | wxICON_ERROR);
    }
}

bool wxMLMainFrame::GenerateTransactionReport(
    const FMLTransactionReportRequest& request,
    const std::vector<FMLTransactionData>& transactions,
    const std::map<std::string, FMLCategoryTotal>& categoryTotals,
    const FMLTransactionSummary& summary,
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

    // 3. 카테고리별 합계 테이블 (수입/지출 분리)
    renderCategorySummaryTables(pdf, categoryTotals);
    pdf.Ln(5);

    // 4. 총계 테이블
    renderGrandTotalTable(pdf, summary);
    pdf.Ln(5);

    // 5. 상세 내역 (양분할 레이아웃)
    renderTransactionDetails(pdf, transactions, exchangeRates);

    return pdf.SaveAsFile(wxString::FromUTF8(request.FilePath));
}

// 카테고리별 합계 테이블 렌더링
void wxMLMainFrame::renderCategorySummaryTables(wxPdfDocument& pdf,
    const std::map<std::string, FMLCategoryTotal>& categoryTotals)
{
    // 수입 카테고리 합계
    pdf.SetFontSize(11);
    pdf.SetFont("MalgunGothic", "B", 11);
    pdf.Cell(0, 8, wxString::FromUTF8("수입 카테고리별 합계"), 0, 1);
    pdf.SetFont("MalgunGothic", "", 10);

    for (const auto& [category, total] : categoryTotals) {
        if (total.IncomeTotal > 0) {
            pdf.Cell(60, 6, wxString::FromUTF8(category), 1);
            pdf.Cell(40, 6, formatAmountWithComma(total.IncomeTotal), 1, 1, wxPDF_ALIGN_RIGHT);
        }
    }

    pdf.Ln(3);

    // 지출 카테고리 합계
    pdf.SetFont("MalgunGothic", "B", 11);
    pdf.Cell(0, 8, wxString::FromUTF8("지출 카테고리별 합계"), 0, 1);
    pdf.SetFont("MalgunGothic", "", 10);

    for (const auto& [category, total] : categoryTotals) {
        if (total.ExpenseTotal > 0) {
            pdf.Cell(60, 6, wxString::FromUTF8(category), 1);
            pdf.Cell(40, 6, formatAmountWithComma(total.ExpenseTotal), 1, 1, wxPDF_ALIGN_RIGHT);
        }
    }
}

// 총계 테이블 렌더링
void wxMLMainFrame::renderGrandTotalTable(wxPdfDocument& pdf,
    const FMLTransactionSummary& summary)
{
    pdf.SetFont("MalgunGothic", "B", 11);
    pdf.Cell(0, 8, wxString::FromUTF8("총계"), 0, 1);
    pdf.SetFont("MalgunGothic", "", 10);

    // 헤더
    pdf.Cell(60, 6, wxString::FromUTF8("총 수입"), 1);
    pdf.Cell(60, 6, wxString::FromUTF8("총 지출"), 1);
    pdf.Cell(60, 6, wxString::FromUTF8("남은 금액"), 1, 1);

    // 값
    pdf.Cell(60, 6, formatAmountWithComma(summary.TotalIncome), 1, 0, wxPDF_ALIGN_RIGHT);
    pdf.Cell(60, 6, formatAmountWithComma(summary.TotalExpense), 1, 0, wxPDF_ALIGN_RIGHT);

    // 잔액 색상 (양수: 파랑, 음수: 빨강)
    if (summary.Balance >= 0) {
        pdf.SetTextColour(0, 0, 255);  // 파랑
    } else {
        pdf.SetTextColour(255, 0, 0);  // 빨강
    }
    pdf.Cell(60, 6, formatAmountWithComma(summary.Balance), 1, 1, wxPDF_ALIGN_RIGHT);
    pdf.SetTextColour(0, 0, 0);  // 검정으로 복원
}

// 상세 내역 렌더링 (양분할 레이아웃)
void wxMLMainFrame::renderTransactionDetails(wxPdfDocument& pdf,
    const std::vector<FMLTransactionData>& transactions,
    const std::map<std::string, double>& exchangeRates)
{
    pdf.Ln(5);
    pdf.SetFont("MalgunGothic", "B", 11);
    pdf.Cell(0, 8, wxString::FromUTF8("상세 내역"), 0, 1);

    // 수입/지출 분리
    std::vector<FMLTransactionData> incomeTransactions;
    std::vector<FMLTransactionData> expenseTransactions;

    for (const auto& trans : transactions) {
        if (trans.Type == E_MLTransactionType::Income) {
            incomeTransactions.push_back(trans);
        } else {
            expenseTransactions.push_back(trans);
        }
    }

    // 페이지 너비 계산 (A4: 210mm, 여백 제외 약 180mm)
    const double pageWidth = 180.0;
    const double columnWidth = pageWidth / 2.0 - 5.0;  // 5mm 중앙 간격
    const double startY = pdf.GetY();

    // 좌측: 수입 컬럼
    renderIncomeColumn(pdf, 10.0, startY, columnWidth, incomeTransactions);

    // 우측: 지출 컬럼
    renderExpenseColumn(pdf, 10.0 + columnWidth + 5.0, startY, columnWidth,
                        expenseTransactions, exchangeRates);
}

// 수입 컬럼 렌더링
void wxMLMainFrame::renderIncomeColumn(wxPdfDocument& pdf,
    double startX, double startY, double width,
    const std::vector<FMLTransactionData>& incomeTransactions)
{
    pdf.SetXY(startX, startY);
    pdf.SetFont("MalgunGothic", "B", 10);

    // 헤더
    pdf.Cell(width * 0.3, 6, wxString::FromUTF8("구분"), 1);
    pdf.Cell(width * 0.4, 6, wxString::FromUTF8("내용"), 1);
    pdf.Cell(width * 0.3, 6, wxString::FromUTF8("금액"), 1, 1);

    pdf.SetFont("MalgunGothic", "", 9);

    // 카테고리별 그룹화
    std::map<std::string, std::vector<FMLTransactionData>> grouped;
    for (const auto& trans : incomeTransactions) {
        grouped[trans.Category].push_back(trans);
    }

    for (const auto& [category, transactions] : grouped) {
        for (size_t i = 0; i < transactions.size(); ++i) {
            const auto& trans = transactions[i];

            pdf.SetX(startX);

            // 구분 (첫 번째만 표시)
            if (i == 0) {
                pdf.Cell(width * 0.3, 6, wxString::FromUTF8(category), 1);
            } else {
                pdf.Cell(width * 0.3, 6, "", 1);
            }

            pdf.Cell(width * 0.4, 6, wxString::FromUTF8(trans.Item), 1);
            pdf.Cell(width * 0.3, 6, formatAmountWithComma(trans.Amount), 1, 1, wxPDF_ALIGN_RIGHT);
        }
    }
}

// 지출 컬럼 렌더링
void wxMLMainFrame::renderExpenseColumn(wxPdfDocument& pdf,
    double startX, double startY, double width,
    const std::vector<FMLTransactionData>& expenseTransactions,
    const std::map<std::string, double>& exchangeRates)
{
    pdf.SetXY(startX, startY);
    pdf.SetFont("MalgunGothic", "B", 10);

    // 헤더
    pdf.Cell(width * 0.2, 6, wxString::FromUTF8("구분"), 1);
    pdf.Cell(width * 0.2, 6, wxString::FromUTF8("항목"), 1);
    pdf.Cell(width * 0.3, 6, wxString::FromUTF8("내용"), 1);
    pdf.Cell(width * 0.3, 6, wxString::FromUTF8("금액"), 1, 1);

    pdf.SetFont("MalgunGothic", "", 9);

    // 카테고리별 그룹화
    std::map<std::string, std::vector<FMLTransactionData>> grouped;
    for (const auto& trans : expenseTransactions) {
        grouped[trans.Category].push_back(trans);
    }

    for (const auto& [category, transactions] : grouped) {
        for (size_t i = 0; i < transactions.size(); ++i) {
            const auto& trans = transactions[i];

            pdf.SetX(startX);

            // 구분 (첫 번째만 표시)
            if (i == 0) {
                pdf.Cell(width * 0.2, 6, wxString::FromUTF8(category), 1);
            } else {
                pdf.Cell(width * 0.2, 6, "", 1);
            }

            pdf.Cell(width * 0.2, 6, wxString::FromUTF8(trans.Description), 1);
            pdf.Cell(width * 0.3, 6, wxString::FromUTF8(trans.Item), 1);

            // 금액 (환율 정보 포함 시 표시)
            wxString amountStr = formatAmountWithComma(trans.Amount);
            if (trans.UseExchangeRate && !trans.Currency.empty() && trans.Currency != "KRW") {
                amountStr += wxString::Format(" (%s %.0f × %.2f)",
                    trans.Currency.c_str(), trans.OriginalAmount, trans.ExchangeRate);
            }

            pdf.Cell(width * 0.3, 6, amountStr, 1, 1, wxPDF_ALIGN_RIGHT);
        }
    }
}
```

### Phase 4: Controller 계층 구현 (30분)

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\interface\IMLController.h`
```cpp
// 새 메서드 선언
virtual std::map<std::string, FMLCategoryTotal> GetCategoryTotals() = 0;
virtual std::map<std::string, FMLCategoryTotal> GetFilteredCategoryTotals(
    const FMLFilterCriteria& criteria) = 0;
virtual bool ExportTransactionReport(const FMLTransactionReportRequest& request) = 0;
```

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\controller\MLController.cpp`
```cpp
std::map<std::string, FMLCategoryTotal> FMLController::GetCategoryTotals()
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    return model ? model->GetCategoryTotals() : std::map<std::string, FMLCategoryTotal>();
}

std::map<std::string, FMLCategoryTotal> FMLController::GetFilteredCategoryTotals(
    const FMLFilterCriteria& criteria)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    return model ? model->GetFilteredCategoryTotals(criteria)
                 : std::map<std::string, FMLCategoryTotal>();
}

bool FMLController::ExportTransactionReport(const FMLTransactionReportRequest& request)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (!model) return false;

    // 데이터 수집
    auto transactions = model->GetFilteredTransactionData(request.FilterCriteria);
    auto categoryTotals = model->GetFilteredCategoryTotals(request.FilterCriteria);
    auto summary = model->GetTransactionSummary(request.FilterCriteria);
    auto exchangeRates = model->GetUsedExchangeRates();

    // View에 PDF 생성 요청
    auto view = FMLMVCHolder::GetInstance().GetView();
    return view->GenerateTransactionReport(request, transactions, categoryTotals,
                                           summary, exchangeRates);
}
```

**파일**: `C:\Work\Others\MissionLedger\MissionLedger\src\interface\IMLView.h`
```cpp
virtual bool GenerateTransactionReport(
    const FMLTransactionReportRequest& request,
    const std::vector<FMLTransactionData>& transactions,
    const std::map<std::string, FMLCategoryTotal>& categoryTotals,
    const FMLTransactionSummary& summary,
    const std::map<std::string, double>& exchangeRates) = 0;
```

## 주요 파일 목록

**수정 파일**:
1. `C:\Work\Others\MissionLedger\MissionLedger\src\MLDefine.h` (FMLTransactionReportRequest, FMLCategoryTotal 추가)
2. `C:\Work\Others\MissionLedger\MissionLedger\src\interface\IMLModel.h` (카테고리 합계 인터페이스)
3. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\model\MLModel.h` (헤더)
4. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\model\MLModel.cpp` (카테고리 합계 구현)
5. `C:\Work\Others\MissionLedger\MissionLedger\src\interface\IMLController.h` (거래 내역 보고서 인터페이스)
6. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\controller\MLController.h` (헤더)
7. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\controller\MLController.cpp` (구현)
8. `C:\Work\Others\MissionLedger\MissionLedger\src\interface\IMLView.h` (거래 내역 보고서 인터페이스)
9. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\view\MLMainFrame.h` (메뉴, PDF 생성)
10. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\view\MLMainFrame.cpp` (메뉴, PDF 생성)

**새 파일**:
1. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\view\MLTransactionReportDialog.h`
2. `C:\Work\Others\MissionLedger\MissionLedger\src\module\mvc\view\MLTransactionReportDialog.cpp`

## 예상 소요 시간

- Phase 1 (데이터 구조): 30분
- Phase 2 (Model 계층): 1-2시간
- Phase 3 (View 계층): 4-5시간
- Phase 4 (Controller 계층): 30분
- Phase 5 (테스트 및 디버깅): 2-3시간
- **총계**: 8-11시간 (약 1-1.5일)

## 기술적 고려사항

### 1. 양분할 레이아웃 구현

PDF에서 수입/지출을 좌우로 나란히 배치:
```cpp
// 좌측 컬럼 시작점
const double leftX = 10.0;
const double columnWidth = 85.0;

// 우측 컬럼 시작점
const double rightX = leftX + columnWidth + 5.0;

// 각 컬럼 독립적으로 렌더링
renderIncomeColumn(pdf, leftX, startY, columnWidth, incomeTransactions);
renderExpenseColumn(pdf, rightX, startY, columnWidth, expenseTransactions);
```

### 2. 카테고리별 그룹화

```cpp
// 카테고리별로 거래 그룹화
std::map<std::string, std::vector<FMLTransactionData>> grouped;
for (const auto& trans : transactions) {
    grouped[trans.Category].push_back(trans);
}

// 카테고리별로 렌더링
for (const auto& [category, categoryTransactions] : grouped) {
    renderCategoryGroup(pdf, category, categoryTransactions);
}
```

### 3. 환율 정보 표시

```cpp
// 금액 (환율 정보 포함 시)
wxString amountStr = formatAmountWithComma(trans.Amount);
if (trans.UseExchangeRate && trans.Currency != "KRW") {
    amountStr += wxString::Format(" (%s %.0f × %.2f)",
        trans.Currency.c_str(),
        trans.OriginalAmount,
        trans.ExchangeRate);
}
// 결과: "2,270,128원 (PHP 91,400 × 24.84)"
```

### 4. 페이지 넘김 처리

```cpp
if (pdf.GetY() > 250) {  // 페이지 하단 근처
    pdf.AddPage();

    // 양분할 레이아웃 재설정
    startY = pdf.GetY();
    renderTableHeader(pdf, leftX, startY);  // 좌측 헤더
    renderTableHeader(pdf, rightX, startY); // 우측 헤더
}
```

### 5. UTF-8 인코딩

- 모든 한글 텍스트: `wxString::FromUTF8()`
- 파일 경로: `wxString::ToUTF8().data()`

### 6. MVC 경계 준수

- View → Controller → Model (단방향)
- PDF 렌더링은 View 책임 (Presentation Layer)
- Controller는 데이터 조율만 담당

## 검증 방법

### 단위 테스트
1. **Model 계층**
   - `GetCategoryTotals()` 테스트
   - `GetFilteredCategoryTotals()` 테스트

### 통합 테스트
1. **PDF 생성**
   - 결산안.csv 데이터로 PDF 생성
   - 레이아웃 비교 (양분할, 카테고리 그룹화)
   - 한글 폰트 출력 검증
   - 환율 정보 표시 확인

### 수동 테스트
1. 메뉴에서 "파일 → 내보내기 → 거래 내역 상세 보고서" 클릭
2. 다이얼로그에서 옵션 선택
3. PDF 파일 생성 및 열어서 확인
4. 결산안.csv와 레이아웃 비교

## 향후 확장 가능성

1. **다양한 보고서 템플릿**
   - 월별 보고서
   - 분기별 보고서
   - 사역별 보고서

2. **차트 추가**
   - 카테고리별 지출 비율 파이 차트
   - 월별 수입/지출 추이 그래프

3. **커스터마이징**
   - 로고 삽입
   - 조직명/교회명 추가
   - 색상 테마 선택
