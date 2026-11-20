#include "MLMainFrame.h"
#include "MLBudgetDialog.h"
#include <wx/statline.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/calctrl.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <set>
#include <algorithm>
#include <cctype>
#include "MLDefine.h"
#include "module/common/holder/MLMVCHolder.h"
#include "interface/IMLController.h"
#include "module/common/format/MLFormat.h"

// 메뉴 ID 정의
enum
{
    ID_NEW_FILE = wxID_NEW,
    ID_OPEN_FILE = wxID_OPEN,
    ID_SAVE_FILE = wxID_SAVE,
    ID_SAVE_FILE_AS = wxID_SAVEAS,
    ID_EXPORT_SETTLEMENT_PDF = wxID_HIGHEST + 1,
    ID_EXPORT_TRANSACTIONS_PDF = wxID_HIGHEST + 2,
    ID_EXIT = wxID_EXIT
};

wxMLMainFrame::wxMLMainFrame()
    : wxFrame(nullptr, wxID_ANY, wxString::FromUTF8("MissionLedger - 거래 관리"), wxDefaultPosition, wxSize(1200, 800))
{
    // wxNotebook 생성 (탭 컨테이너)
    notebook = new wxNotebook(this, wxID_ANY);

    // === 탭1: 거래 내역 ===
    wxPanel* transactionTab = new wxPanel(notebook);
    wxBoxSizer* transactionTabSizer = new wxBoxSizer(wxHORIZONTAL);

    // 거래 내역 탭의 메인 사이저 (기존 mainSizer)
    wxBoxSizer* mainSizer = transactionTabSizer;

    // === 왼쪽 입력 패널 ===
    wxPanel* inputPanel = new wxPanel(transactionTab);
    inputPanel->SetBackgroundColour(wxColour(240, 240, 240));
    wxBoxSizer* inputSizer = new wxBoxSizer(wxVERTICAL);

    // 제목
    wxStaticText* titleText = new wxStaticText(inputPanel, wxID_ANY, wxString::FromUTF8("거래 입력"));
    wxFont titleFont = titleText->GetFont();
    titleFont.SetPointSize(12);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleText->SetFont(titleFont);
    inputSizer->Add(titleText, 0, wxALL | wxALIGN_CENTER, 10);

    // 구분선
    inputSizer->Add(new wxStaticLine(inputPanel), 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    inputSizer->AddSpacer(10);

    // 거래 유형 (라디오 버튼)
    wxStaticText* typeLabel = new wxStaticText(inputPanel, wxID_ANY, wxString::FromUTF8("거래 유형:"));
    inputSizer->Add(typeLabel, 0, wxLEFT | wxRIGHT, 10);

    incomeRadio = new wxRadioButton(inputPanel, wxID_ANY, wxString::FromUTF8("수입"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    expenseRadio = new wxRadioButton(inputPanel, wxID_ANY, wxString::FromUTF8("지출"));
    incomeRadio->SetValue(true);
    inputSizer->Add(incomeRadio, 0, wxLEFT, 20);
    inputSizer->Add(expenseRadio, 0, wxLEFT, 20);
    inputSizer->AddSpacer(10);

    // 카테고리
    wxStaticText* categoryLabel = new wxStaticText(inputPanel, wxID_ANY, wxString::FromUTF8("카테고리:"));
    inputSizer->Add(categoryLabel, 0, wxLEFT | wxRIGHT, 10);
    categoryText = new wxTextCtrl(inputPanel, wxID_ANY);
    inputSizer->Add(categoryText, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    inputSizer->AddSpacer(5);

    // 항목
    wxStaticText* itemLabel = new wxStaticText(inputPanel, wxID_ANY, wxString::FromUTF8("항목:"));
    inputSizer->Add(itemLabel, 0, wxLEFT | wxRIGHT, 10);
    itemText = new wxTextCtrl(inputPanel, wxID_ANY);
    inputSizer->Add(itemText, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    inputSizer->AddSpacer(5);

    // 설명
    wxStaticText* descLabel = new wxStaticText(inputPanel, wxID_ANY, wxString::FromUTF8("설명:"));
    inputSizer->Add(descLabel, 0, wxLEFT | wxRIGHT, 10);
    descriptionText = new wxTextCtrl(inputPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    inputSizer->Add(descriptionText, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);
    inputSizer->AddSpacer(5);

    // 금액
    wxStaticText* amountLabel = new wxStaticText(inputPanel, wxID_ANY, wxString::FromUTF8("금액:"));
    inputSizer->Add(amountLabel, 0, wxLEFT | wxRIGHT, 10);
    amountText = new wxTextCtrl(inputPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_DIGITS));
    inputSizer->Add(amountText, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    inputSizer->AddSpacer(5);

    // 환율 적용 체크박스
    exchangeRateCheckBox = new wxCheckBox(inputPanel, wxID_ANY, wxString::FromUTF8("환율 적용"));
    exchangeRateCheckBox->SetValue(false);
    inputSizer->Add(exchangeRateCheckBox, 0, wxLEFT | wxRIGHT, 10);
    inputSizer->AddSpacer(5);

    // 환율 정보 패널 (체크박스 선택 시 활성화)
    exchangePanel = new wxPanel(inputPanel);
    exchangePanel->SetBackgroundColour(wxColour(235, 235, 235));
    wxBoxSizer* exchangeSizer = new wxBoxSizer(wxVERTICAL);

    // 통화 선택
    wxStaticText* currencyLabel = new wxStaticText(exchangePanel, wxID_ANY, wxString::FromUTF8("통화:"));
    exchangeSizer->Add(currencyLabel, 0, wxALL, 5);

    wxArrayString currencies;
    currencies.Add("KRW");
    currencies.Add("USD");
    currencies.Add("EUR");
    currencies.Add("JPY");
    currencies.Add("CNY");
    currencies.Add("PHP");
    currencies.Add("VND");
    currencies.Add("THB");
    currencyCombo = new wxComboBox(exchangePanel, wxID_ANY, "USD", wxDefaultPosition, wxDefaultSize, currencies, wxCB_READONLY);
    exchangeSizer->Add(currencyCombo, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    // 외화 금액
    wxStaticText* originalAmountLabel = new wxStaticText(exchangePanel, wxID_ANY, wxString::FromUTF8("외화 금액:"));
    exchangeSizer->Add(originalAmountLabel, 0, wxLEFT | wxRIGHT, 5);
    originalAmountText = new wxTextCtrl(exchangePanel, wxID_ANY);
    exchangeSizer->Add(originalAmountText, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    // 환율
    wxStaticText* exchangeRateLabel = new wxStaticText(exchangePanel, wxID_ANY, wxString::FromUTF8("환율 (원/외화):"));
    exchangeSizer->Add(exchangeRateLabel, 0, wxLEFT | wxRIGHT, 5);
    exchangeRateText = new wxTextCtrl(exchangePanel, wxID_ANY);
    exchangeSizer->Add(exchangeRateText, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    // 환산 금액 표시
    convertedAmountLabel = new wxStaticText(exchangePanel, wxID_ANY, wxString::FromUTF8("= 원화 환산: 0원"));
    wxFont convertedFont = convertedAmountLabel->GetFont();
    convertedFont.SetWeight(wxFONTWEIGHT_BOLD);
    convertedAmountLabel->SetFont(convertedFont);
    convertedAmountLabel->SetForegroundColour(wxColour(0, 100, 200));
    exchangeSizer->Add(convertedAmountLabel, 0, wxALL, 5);

    exchangePanel->SetSizer(exchangeSizer);
    inputSizer->Add(exchangePanel, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    inputSizer->AddSpacer(5);

    // 초기 상태: 환율 패널 비활성화
    exchangePanel->Enable(false);

    // 영수증 번호
    wxStaticText* receiptLabel = new wxStaticText(inputPanel, wxID_ANY, wxString::FromUTF8("영수증 번호:"));
    inputSizer->Add(receiptLabel, 0, wxLEFT | wxRIGHT, 10);
    receiptText = new wxTextCtrl(inputPanel, wxID_ANY);
    inputSizer->Add(receiptText, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    inputSizer->AddSpacer(5);

    // 날짜
    wxStaticText* dateLabel = new wxStaticText(inputPanel, wxID_ANY, wxString::FromUTF8("날짜:"));
    inputSizer->Add(dateLabel, 0, wxLEFT | wxRIGHT, 10);

    wxBoxSizer* dateSizer = new wxBoxSizer(wxHORIZONTAL);
    dateText = new wxTextCtrl(inputPanel, wxID_ANY, "", wxDefaultPosition, wxSize(120, -1));
    dateText->SetHint("YYYY-MM-DD");
    dateSizer->Add(dateText, 1, wxEXPAND | wxRIGHT, 5);

    dateCalendarButton = new wxButton(inputPanel, wxID_ANY, wxString::FromUTF8("📅"), wxDefaultPosition, wxSize(30, -1));
    dateCalendarButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { showCalendarDialog(dateText); });
    dateSizer->Add(dateCalendarButton, 0);

    inputSizer->Add(dateSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    inputSizer->AddSpacer(15);

    // 버튼 패널 (추가, 수정, 삭제)
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    addButton = new wxButton(inputPanel, wxID_ANY, wxString::FromUTF8("추가"));
    updateButton = new wxButton(inputPanel, wxID_ANY, wxString::FromUTF8("수정"));
    deleteButton = new wxButton(inputPanel, wxID_ANY, wxString::FromUTF8("삭제"));

    // 초기 상태: 수정/삭제 버튼 비활성화
    updateButton->Enable(false);
    deleteButton->Enable(false);

    buttonSizer->Add(addButton, 1, wxRIGHT, 5);
    buttonSizer->Add(updateButton, 1, wxRIGHT, 5);
    buttonSizer->Add(deleteButton, 1, 0, 0);

    inputSizer->Add(buttonSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    inputPanel->SetSizer(inputSizer);

    // === 오른쪽 패널 (필터 + 리스트) ===
    wxPanel* rightPanel = new wxPanel(transactionTab);
    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

    // 필터 패널 생성
    createFilterPanel(rightPanel, rightSizer);

    // Summary 패널 생성
    createSummaryPanel(rightPanel, rightSizer);

    // 리스트 컨트롤
    listCtrl = new wxListCtrl(rightPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);

    // 컬럼 추가
    listCtrl->InsertColumn(0, wxString::FromUTF8("유형"), wxLIST_FORMAT_LEFT, 60);
    listCtrl->InsertColumn(1, wxString::FromUTF8("카테고리"), wxLIST_FORMAT_LEFT, 120);
    listCtrl->InsertColumn(2, wxString::FromUTF8("항목"), wxLIST_FORMAT_LEFT, 120);
    listCtrl->InsertColumn(3, wxString::FromUTF8("금액"), wxLIST_FORMAT_RIGHT, 100);
    listCtrl->InsertColumn(4, wxString::FromUTF8("영수증번호"), wxLIST_FORMAT_LEFT, 100);
    listCtrl->InsertColumn(5, wxString::FromUTF8("날짜/시간"), wxLIST_FORMAT_LEFT, 150);

    rightSizer->Add(listCtrl, 1, wxEXPAND | wxALL, 5);
    rightPanel->SetSizer(rightSizer);

    // 거래 내역 탭 사이저에 패널 추가
    mainSizer->Add(inputPanel, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(rightPanel, 2, wxEXPAND | wxALL, 5);

    transactionTab->SetSizer(mainSizer);

    // === 탭2: 예산 관리 ===
    wxPanel* budgetTab = createBudgetTab();

    // notebook에 탭 추가
    notebook->AddPage(transactionTab, wxString::FromUTF8("거래 내역"), true);
    notebook->AddPage(budgetTab, wxString::FromUTF8("예산 관리"), false);

    // 버튼 이벤트 바인딩
    addButton->Bind(wxEVT_BUTTON, &wxMLMainFrame::OnAddTransaction, this);
    updateButton->Bind(wxEVT_BUTTON, &wxMLMainFrame::OnUpdateTransaction, this);
    deleteButton->Bind(wxEVT_BUTTON, &wxMLMainFrame::OnDeleteTransaction, this);

    // 리스트 선택 이벤트 바인딩
    listCtrl->Bind(wxEVT_LIST_ITEM_SELECTED, &wxMLMainFrame::OnListItemSelected, this);
    listCtrl->Bind(wxEVT_LIST_ITEM_DESELECTED, &wxMLMainFrame::OnListItemDeselected, this);
    listCtrl->Bind(wxEVT_LIST_COL_CLICK, &wxMLMainFrame::OnColumnHeaderClick, this);

    // 환율 관련 이벤트 바인딩
    exchangeRateCheckBox->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event) {
        bool enabled = event.IsChecked();
        exchangePanel->Enable(enabled);

        // 체크 해제 시 금액 필드 활성화, 체크 시 비활성화
        amountText->Enable(!enabled);

        if (enabled) {
            updateConvertedAmount();
        } else {
            convertedAmountLabel->SetLabel(wxString::FromUTF8("= 원화 환산: 0원"));
        }
    });

    // 외화 금액 또는 환율 변경 시 자동 계산
    originalAmountText->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { updateConvertedAmount(); });
    exchangeRateText->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { updateConvertedAmount(); });

    // 메뉴바 생성
    createMenuBar();

    // 창 닫기 이벤트 바인딩
    Bind(wxEVT_CLOSE_WINDOW, &wxMLMainFrame::OnClose, this);

    // 제목 초기화
    updateTitle();
}

void wxMLMainFrame::AddTransaction(const FMLTransactionData& data)
{
    // Controller를 통해 Model에 추가
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (controller) {
        controller->AddTransaction(data);

        // Observer 패턴으로 자동 UI 업데이트됨 (OnTransactionAdded 호출)

        // 입력 필드 초기화
        clearInputFields();

        // wxMessageBox(wxString::FromUTF8("거래가 추가되었습니다."), wxString::FromUTF8("성공"), wxOK | wxICON_INFORMATION);
    }
}

void wxMLMainFrame::DisplayTransaction(const FMLTransactionData& data)
{
    // ID로 기존 항목 찾기
    long index = findListItemByTransactionId(data.TransactionId);

    if (index == -1) {
        // 새 항목 추가
        index = listCtrl->InsertItem(listCtrl->GetItemCount(),
            data.Type == E_MLTransactionType::Income ? wxString::FromUTF8("수입") : wxString::FromUTF8("지출"));
        listCtrl->SetItemData(index, data.TransactionId);
    } else {
        // 기존 항목의 유형 컬럼 업데이트
        listCtrl->SetItem(index, 0,
            data.Type == E_MLTransactionType::Income ? wxString::FromUTF8("수입") : wxString::FromUTF8("지출"));
    }

    // 추가든 수정이든 공통: 나머지 컬럼 데이터 설정
    listCtrl->SetItem(index, 1, wxString::FromUTF8(data.Category.c_str()));
    listCtrl->SetItem(index, 2, wxString::FromUTF8(data.Item.c_str()));
    listCtrl->SetItem(index, 3, wxString::FromUTF8(FMLFormat::FormatAmount(data.Amount)));
    listCtrl->SetItem(index, 4, wxString::FromUTF8(data.ReceiptNumber.c_str()));
    listCtrl->SetItem(index, 5, wxString::FromUTF8(data.DateTime.c_str()));
}

void wxMLMainFrame::DisplayTransactions(const std::vector<FMLTransactionData>& data)
{
    for (const auto& trans : data) {
        DisplayTransaction(trans);
    }
}

void wxMLMainFrame::OnAddTransaction(wxCommandEvent& event)
{
    // 입력 데이터 수집 및 검증
    FMLTransactionData data;
    if (!collectTransactionDataFromInput(data)) {
        return;  // 검증 실패 (에러 메시지는 함수 내에서 표시됨)
    }

    AddTransaction(data);
}

void wxMLMainFrame::clearInputFields()
{
    incomeRadio->SetValue(true);
    categoryText->Clear();
    itemText->Clear();
    descriptionText->Clear();
    amountText->Clear();
    receiptText->Clear();
    // 날짜 필드는 현재 날짜로 자동 설정 (빈 값 방지 및 UX 개선)
    dateText->SetValue(wxDateTime::Now().FormatISODate());

    // 환율 관련 필드 초기화
    exchangeRateCheckBox->SetValue(false);
    currencyCombo->SetValue("USD");
    originalAmountText->Clear();
    exchangeRateText->Clear();
    convertedAmountLabel->SetLabel(wxString::FromUTF8("= 원화 환산: 0원"));
    amountText->Enable(true);  // 금액 필드 활성화
}

// IMLModelObserver 인터페이스 구현
void wxMLMainFrame::OnTransactionAdded(const FMLTransactionData& transactionData)
{
    updateCategoryFilter();

    if (FilterActive)
    {
        // 필터 활성화 상태: 전체 재로드로 필터 재적용
        applyCurrentFilter();
    }
    else
    {
        // 필터 비활성화 상태: 단순 추가
        DisplayTransaction(transactionData);
    }

    updateTitle();
    updateSummaryPanel();
}

void wxMLMainFrame::OnTransactionDeleted(const int transactionId)
{
    updateCategoryFilter();

    if (FilterActive)
    {
        // 필터 활성화 상태: 전체 재로드로 필터 재적용
        applyCurrentFilter();
    }
    else
    {
        // 필터 비활성화 상태: 리스트에서 항목 제거
        removeListItemByTransactionId(transactionId);
    }

    updateTitle();
    updateSummaryPanel();
}

void wxMLMainFrame::OnTransactionUpdated(const FMLTransactionData& transactionData)
{
    updateCategoryFilter();

    if (FilterActive)
    {
        // 필터 활성화 상태: 전체 재로드로 필터 재적용
        applyCurrentFilter();
    }
    else
    {
        // 필터 비활성화 상태: 리스트에서 항목 업데이트
        DisplayTransaction(transactionData);
    }

    updateTitle();
    updateSummaryPanel();
}

void wxMLMainFrame::OnTransactionsCleared()
{
    // 모든 항목 제거
    listCtrl->DeleteAllItems();
    updateTitle();
    updateSummaryPanel();
}


void wxMLMainFrame::OnDataLoaded()
{
    // 데이터 로드 시 전체 재로드 (FilterActive 상태 유지)
    updateTitle();  // 파일명으로 타이틀 갱신
    updateCategoryFilter();
    applyCurrentFilter();
    updateSummaryPanel();
    updateCategoryList();  // 예산 리스트도 업데이트
}

void wxMLMainFrame::OnDataSaved()
{
    // 저장 완료 메시지 (선택사항)
    // wxMessageBox("데이터가 저장되었습니다.", "저장 완료", wxOK | wxICON_INFORMATION);
}

// 리스트 항목 선택 이벤트
void wxMLMainFrame::OnListItemSelected(wxListEvent& event)
{
    const long selectedIndex = event.GetIndex();
    SelectedTransactionId = static_cast<int>(listCtrl->GetItemData(selectedIndex));

    loadTransactionToInput(SelectedTransactionId);
    updateButtonStates();
}

void wxMLMainFrame::OnListItemDeselected(wxListEvent& event)
{
    SelectedTransactionId = -1;
    updateButtonStates();
}

// 수정 버튼 핸들러
void wxMLMainFrame::OnUpdateTransaction(wxCommandEvent& event)
{
    if (SelectedTransactionId < 0) {
        wxMessageBox(wxString::FromUTF8("수정할 거래를 선택하세요."), wxString::FromUTF8("알림"), wxOK | wxICON_WARNING);
        return;
    }

    // 입력 데이터 수집 및 검증
    FMLTransactionData data;
    data.TransactionId = SelectedTransactionId;

    if (!collectTransactionDataFromInput(data)) {
        return;  // 검증 실패 (에러 메시지는 함수 내에서 표시됨)
    }

    // Controller를 통해 수정
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (controller) {
        if (controller->UpdateTransaction(data)) {
            clearInputFields();
            SelectedTransactionId = -1;
            listCtrl->SetItemState(-1, 0, wxLIST_STATE_SELECTED);  // 선택 해제
            updateButtonStates();
        } else {
            wxMessageBox(wxString::FromUTF8("거래 수정에 실패했습니다."), wxString::FromUTF8("오류"), wxOK | wxICON_ERROR);
        }
    }
}

// 삭제 버튼 핸들러
void wxMLMainFrame::OnDeleteTransaction(wxCommandEvent& event)
{
    if (SelectedTransactionId < 0) {
        wxMessageBox(wxString::FromUTF8("삭제할 거래를 선택하세요."), wxString::FromUTF8("알림"), wxOK | wxICON_WARNING);
        return;
    }

    // 삭제 확인
    const int result = wxMessageBox(
        wxString::FromUTF8("선택한 거래를 삭제하시겠습니까?"),
        wxString::FromUTF8("삭제 확인"),
        wxYES_NO | wxICON_QUESTION
    );

    if (result != wxYES) {
        return;
    }

    // Controller를 통해 삭제
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (controller) {
        if (controller->DeleteTransaction(SelectedTransactionId)) {
            clearInputFields();
            SelectedTransactionId = -1;
            updateButtonStates();
        } else {
            wxMessageBox(wxString::FromUTF8("거래 삭제에 실패했습니다."), wxString::FromUTF8("오류"), wxOK | wxICON_ERROR);
        }
    }
}

// 선택된 거래 데이터를 입력 필드에 로드
void wxMLMainFrame::loadTransactionToInput(int transactionId)
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller) return;

    FMLTransactionData data = controller->GetTransactionData(transactionId);

    // 거래 유형
    if (data.Type == E_MLTransactionType::Income) {
        incomeRadio->SetValue(true);
    } else {
        expenseRadio->SetValue(true);
    }

    // 텍스트 필드
    categoryText->SetValue(wxString::FromUTF8(data.Category.c_str()));
    itemText->SetValue(wxString::FromUTF8(data.Item.c_str()));
    descriptionText->SetValue(wxString::FromUTF8(data.Description.c_str()));
    amountText->SetValue(wxString::Format("%lld", data.Amount));
    receiptText->SetValue(wxString::FromUTF8(data.ReceiptNumber.c_str()));

    // 날짜 파싱 (YYYY-MM-DD 형식)
    dateText->SetValue(wxString::FromUTF8(data.DateTime.c_str()));

    // 환율 정보 로드
    exchangeRateCheckBox->SetValue(data.UseExchangeRate);

    // 환율 필드 활성화/비활성화
    exchangePanel->Enable(data.UseExchangeRate);
    amountText->Enable(!data.UseExchangeRate);

    if (data.UseExchangeRate) {
        currencyCombo->SetValue(wxString::FromUTF8(data.Currency.c_str()));
        originalAmountText->SetValue(wxString::Format("%.2f", data.OriginalAmount));
        exchangeRateText->SetValue(wxString::Format("%.2f", data.ExchangeRate));
        updateConvertedAmount();
    }
}

// 버튼 활성화 상태 업데이트
void wxMLMainFrame::updateButtonStates()
{
    bool hasSelection = (SelectedTransactionId >= 0);
    updateButton->Enable(hasSelection);
    deleteButton->Enable(hasSelection);
}

// 메뉴바 생성
void wxMLMainFrame::createMenuBar()
{
    wxMenuBar* menuBar = new wxMenuBar();

    // 파일 메뉴
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(ID_NEW_FILE, wxString::FromUTF8("새 파일(&N)\tCtrl+N"));
    fileMenu->Append(ID_OPEN_FILE, wxString::FromUTF8("열기(&O)...\tCtrl+O"));
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_SAVE_FILE, wxString::FromUTF8("저장(&S)\tCtrl+S"));
    fileMenu->Append(ID_SAVE_FILE_AS, wxString::FromUTF8("다른 이름으로 저장(&A)...\tCtrl+Shift+S"));
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_EXPORT_SETTLEMENT_PDF, wxString::FromUTF8("결산 보고서 내보내기(&E)...\tCtrl+E"));
    fileMenu->Append(ID_EXPORT_TRANSACTIONS_PDF, wxString::FromUTF8("거래 내역서 내보내기(&T)...\tCtrl+T"));
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_EXIT, wxString::FromUTF8("종료(&X)\tAlt+F4"));

    menuBar->Append(fileMenu, wxString::FromUTF8("파일(&F)"));

    SetMenuBar(menuBar);

    // 메뉴 이벤트 바인딩
    Bind(wxEVT_MENU, &wxMLMainFrame::OnNewFile, this, ID_NEW_FILE);
    Bind(wxEVT_MENU, &wxMLMainFrame::OnOpenFile, this, ID_OPEN_FILE);
    Bind(wxEVT_MENU, &wxMLMainFrame::OnSaveFile, this, ID_SAVE_FILE);
    Bind(wxEVT_MENU, &wxMLMainFrame::OnSaveFileAs, this, ID_SAVE_FILE_AS);
    Bind(wxEVT_MENU, &wxMLMainFrame::OnExportSettlementPDF, this, ID_EXPORT_SETTLEMENT_PDF);
    Bind(wxEVT_MENU, &wxMLMainFrame::OnExportTransactionsPDF, this, ID_EXPORT_TRANSACTIONS_PDF);
    Bind(wxEVT_MENU, &wxMLMainFrame::OnExit, this, ID_EXIT);
}

// 파일 메뉴 핸들러
void wxMLMainFrame::OnNewFile(wxCommandEvent& event)
{
    if (!checkUnsavedChanges()) return;

    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (controller)
    {
        controller->NewFile();
        updateTitle();
        clearInputFields();
    }
}

void wxMLMainFrame::OnOpenFile(wxCommandEvent& event)
{
    if (!checkUnsavedChanges()) return;

    wxFileDialog openDialog(this,
        wxString::FromUTF8("파일 열기"),
        wxEmptyString,
        wxEmptyString,
        wxString::FromUTF8("MissionLedger 파일 (*.ml)|*.ml|모든 파일 (*.*)|*.*"),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openDialog.ShowModal() == wxID_CANCEL) return;

    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (controller)
    {
        if (controller->OpenFile(openDialog.GetPath().ToUTF8().data()))
        {
            updateTitle();
            clearInputFields();
        }
        else
        {
            wxMessageBox(wxString::FromUTF8("파일을 열 수 없습니다."),
                wxString::FromUTF8("오류"), wxOK | wxICON_ERROR);
        }
    }
}

void wxMLMainFrame::OnSaveFile(wxCommandEvent& event)
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller) return;

    if (controller->GetCurrentFilePath().empty())
    {
        OnSaveFileAs(event);
        return;
    }

    if (!controller->SaveFile())
    {
        wxMessageBox(wxString::FromUTF8("파일 저장에 실패했습니다."),
            wxString::FromUTF8("오류"), wxOK | wxICON_ERROR);
    }
    else
    {
        updateTitle();
    }
}

void wxMLMainFrame::OnSaveFileAs(wxCommandEvent& event)
{
    wxFileDialog saveDialog(this,
        wxString::FromUTF8("다른 이름으로 저장"),
        wxEmptyString,
        wxEmptyString,
        wxString::FromUTF8("MissionLedger 파일 (*.ml)|*.ml"),
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveDialog.ShowModal() == wxID_CANCEL) return;

    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (controller)
    {
        wxString filePath = saveDialog.GetPath();
        // .ml 확장자 추가
        if (!filePath.EndsWith(".ml"))
        {
            filePath += ".ml";
        }

        if (controller->SaveFileAs(filePath.ToUTF8().data()))
        {
            updateTitle();
        }
        else
        {
            wxMessageBox(wxString::FromUTF8("파일 저장에 실패했습니다."),
                wxString::FromUTF8("오류"), wxOK | wxICON_ERROR);
        }
    }
}

void wxMLMainFrame::OnExportSettlementPDF(wxCommandEvent& event)
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller) return;

    // 현재 .ml 파일 경로에서 기본 파일명 생성
    wxString defaultFilename = wxString::FromUTF8("결산보고서.pdf");
    std::string currentFilePath = controller->GetCurrentFilePath();
    if (!currentFilePath.empty())
    {
        wxFileName fn(wxString::FromUTF8(currentFilePath));
        defaultFilename = fn.GetName() + ".pdf";  // 확장자를 제거하고 .pdf 추가
    }

    wxFileDialog saveDialog(this,
        wxString::FromUTF8("결산 보고서 내보내기"),
        wxEmptyString,
        defaultFilename,
        wxString::FromUTF8("PDF 파일 (*.pdf)|*.pdf"),
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveDialog.ShowModal() == wxID_CANCEL) return;

    wxString filePath = saveDialog.GetPath();
    // .pdf 확장자 확인
    if (!filePath.EndsWith(".pdf"))
    {
        filePath += ".pdf";
    }

    if (controller->ExportSettlementToPDF(filePath.ToUTF8().data()))
    {
        wxMessageBox(wxString::FromUTF8("결산 보고서가 성공적으로 생성되었습니다."),
            wxString::FromUTF8("완료"), wxOK | wxICON_INFORMATION);
    }
    else
    {
        wxMessageBox(wxString::FromUTF8("결산 보고서 생성에 실패했습니다."),
            wxString::FromUTF8("오류"), wxOK | wxICON_ERROR);
    }
}

void wxMLMainFrame::OnExportTransactionsPDF(wxCommandEvent& event)
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller) return;

    // 현재 .ml 파일 경로에서 기본 PDF 파일명 생성
    wxString defaultFilename = wxString::FromUTF8("거래내역서.pdf");
    std::string currentFilePath = controller->GetCurrentFilePath();
    if (!currentFilePath.empty())
    {
        wxFileName fn(wxString::FromUTF8(currentFilePath));
        defaultFilename = fn.GetName() + ".pdf";  // 확장자를 제거하고 .pdf 추가
    }

    wxFileDialog saveDialog(this,
        wxString::FromUTF8("거래 내역서 내보내기"),
        wxEmptyString,
        defaultFilename,
        wxString::FromUTF8("PDF 파일 (*.pdf)|*.pdf"),
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveDialog.ShowModal() == wxID_CANCEL) return;

    wxString filePath = saveDialog.GetPath();
    // .pdf 확장자 확인
    if (!filePath.EndsWith(".pdf"))
    {
        filePath += ".pdf";
    }

    if (controller->ExportTransactionListToPDF(filePath.ToUTF8().data()))
    {
        wxMessageBox(wxString::FromUTF8("거래 내역서가 성공적으로 생성되었습니다."),
            wxString::FromUTF8("완료"), wxOK | wxICON_INFORMATION);
    }
    else
    {
        wxMessageBox(wxString::FromUTF8("거래 내역서 생성에 실패했습니다."),
            wxString::FromUTF8("오류"), wxOK | wxICON_ERROR);
    }
}

void wxMLMainFrame::OnExit(wxCommandEvent& event)
{
    Close(false);
}

void wxMLMainFrame::OnClose(wxCloseEvent& event)
{
    if (!checkUnsavedChanges())
    {
        event.Veto();
        return;
    }
    event.Skip();
}

void wxMLMainFrame::updateTitle()
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    wxString title = wxString::FromUTF8("MissionLedger");

    if (controller)
    {
        const std::string& filePath = controller->GetCurrentFilePath();
        if (!filePath.empty())
        {
            wxFileName fileName(wxString::FromUTF8(filePath.c_str()));
            title = fileName.GetFullName() + wxString::FromUTF8(" - ") + title;
        }
        else
        {
            title = wxString::FromUTF8("새 파일 - ") + title;
        }

        if (controller->HasUnsavedChanges())
        {
            title = wxString::FromUTF8("* ") + title;
        }
    }
    else
    {
        // Controller가 아직 초기화되지 않은 경우 (프로그램 시작 시)
        title = wxString::FromUTF8("새 파일 - ") + title;
    }

    SetTitle(title);
}

bool wxMLMainFrame::checkUnsavedChanges()
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller || !controller->HasUnsavedChanges())
    {
        return true;
    }

    const int result = wxMessageBox(
        wxString::FromUTF8("저장되지 않은 변경 사항이 있습니다.\n저장하시겠습니까?"),
        wxString::FromUTF8("저장 확인"),
        wxYES_NO | wxCANCEL | wxICON_QUESTION);

    if (result == wxCANCEL)
    {
        return false;
    }

    if (result == wxYES)
    {
        wxCommandEvent dummyEvent;
        OnSaveFile(dummyEvent);
        return !controller->HasUnsavedChanges();
    }

    return true;
}

// 필터 패널 생성
void wxMLMainFrame::createFilterPanel(wxPanel* parent, wxBoxSizer* sizer)
{
    wxPanel* filterPanel = new wxPanel(parent);
    filterPanel->SetBackgroundColour(wxColour(250, 250, 250));
    wxBoxSizer* filterSizer = new wxBoxSizer(wxVERTICAL);

    // 제목
    wxStaticText* filterTitle = new wxStaticText(filterPanel, wxID_ANY, wxString::FromUTF8("필터"));
    wxFont titleFont = filterTitle->GetFont();
    titleFont.SetPointSize(10);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    filterTitle->SetFont(titleFont);
    filterSizer->Add(filterTitle, 0, wxALL, 5);

    // 날짜 범위
    wxBoxSizer* dateSizer = new wxBoxSizer(wxHORIZONTAL);
    dateSizer->Add(new wxStaticText(filterPanel, wxID_ANY, wxString::FromUTF8("기간:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

    filterStartDateText = new wxTextCtrl(filterPanel, wxID_ANY, "", wxDefaultPosition, wxSize(100, -1));
    filterStartDateText->SetHint("YYYY-MM-DD");
    filterStartDateButton = new wxButton(filterPanel, wxID_ANY, wxString::FromUTF8("📅"), wxDefaultPosition, wxSize(30, -1));
    filterStartDateButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { showCalendarDialog(filterStartDateText); });

    filterEndDateText = new wxTextCtrl(filterPanel, wxID_ANY, "", wxDefaultPosition, wxSize(100, -1));
    filterEndDateText->SetHint("YYYY-MM-DD");
    filterEndDateButton = new wxButton(filterPanel, wxID_ANY, wxString::FromUTF8("📅"), wxDefaultPosition, wxSize(30, -1));
    filterEndDateButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { showCalendarDialog(filterEndDateText); });

    dateSizer->Add(filterStartDateText, 1, wxRIGHT, 2);
    dateSizer->Add(filterStartDateButton, 0, wxRIGHT, 5);
    dateSizer->Add(new wxStaticText(filterPanel, wxID_ANY, "~"), 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 2);
    dateSizer->Add(filterEndDateText, 1, wxLEFT | wxRIGHT, 2);
    dateSizer->Add(filterEndDateButton, 0, wxLEFT, 5);

    filterSizer->Add(dateSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    // 거래 유형
    wxBoxSizer* typeSizer = new wxBoxSizer(wxHORIZONTAL);
    typeSizer->Add(new wxStaticText(filterPanel, wxID_ANY, wxString::FromUTF8("유형:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

    wxArrayString typeChoices;
    typeChoices.Add(wxString::FromUTF8("전체"));
    typeChoices.Add(wxString::FromUTF8("수입"));
    typeChoices.Add(wxString::FromUTF8("지출"));
    filterTypeChoice = new wxChoice(filterPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, typeChoices);
    filterTypeChoice->SetSelection(0);

    typeSizer->Add(filterTypeChoice, 1, 0, 0);
    filterSizer->Add(typeSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    // 카테고리
    wxBoxSizer* categorySizer = new wxBoxSizer(wxHORIZONTAL);
    categorySizer->Add(new wxStaticText(filterPanel, wxID_ANY, wxString::FromUTF8("카테고리:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

    filterCategoryCombo = new wxComboBox(filterPanel, wxID_ANY);
    filterCategoryCombo->Append(wxString::FromUTF8("전체"));
    filterCategoryCombo->SetSelection(0);

    categorySizer->Add(filterCategoryCombo, 1, 0, 0);
    filterSizer->Add(categorySizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    // 검색
    wxStaticText* searchLabel = new wxStaticText(filterPanel, wxID_ANY, wxString::FromUTF8("검색:"));
    filterSizer->Add(searchLabel, 0, wxLEFT | wxRIGHT, 5);

    filterSearchText = new wxTextCtrl(filterPanel, wxID_ANY);
    filterSizer->Add(filterSearchText, 0, wxEXPAND | wxLEFT | wxRIGHT, 5);

    // 검색 범위 체크박스
    wxBoxSizer* searchFieldSizer = new wxBoxSizer(wxHORIZONTAL);
    filterSearchInItem = new wxCheckBox(filterPanel, wxID_ANY, wxString::FromUTF8("항목"));
    filterSearchInDescription = new wxCheckBox(filterPanel, wxID_ANY, wxString::FromUTF8("설명"));
    filterSearchInReceipt = new wxCheckBox(filterPanel, wxID_ANY, wxString::FromUTF8("영수증"));

    filterSearchInItem->SetValue(false);
    filterSearchInDescription->SetValue(false);
    filterSearchInReceipt->SetValue(false);

    searchFieldSizer->Add(filterSearchInItem, 0, wxRIGHT, 5);
    searchFieldSizer->Add(filterSearchInDescription, 0, wxRIGHT, 5);
    searchFieldSizer->Add(filterSearchInReceipt, 0, 0, 0);

    filterSizer->Add(searchFieldSizer, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);

    // 버튼
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    applyFilterButton = new wxButton(filterPanel, wxID_ANY, wxString::FromUTF8("적용"));
    clearFilterButton = new wxButton(filterPanel, wxID_ANY, wxString::FromUTF8("초기화"));

    buttonSizer->Add(applyFilterButton, 1, wxRIGHT, 3);
    buttonSizer->Add(clearFilterButton, 1, wxLEFT, 3);

    filterSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);

    filterPanel->SetSizer(filterSizer);
    sizer->Add(filterPanel, 0, wxEXPAND | wxALL, 5);

    // 이벤트 바인딩
    applyFilterButton->Bind(wxEVT_BUTTON, &wxMLMainFrame::OnApplyFilter, this);
    clearFilterButton->Bind(wxEVT_BUTTON, &wxMLMainFrame::OnClearFilter, this);
}

// 필터 적용
void wxMLMainFrame::OnApplyFilter(wxCommandEvent& event)
{
    FilterActive = true;
    applyCurrentFilter();
    updateSummaryPanel();
}

// 필터 초기화
void wxMLMainFrame::OnClearFilter(wxCommandEvent& event)
{
    FilterActive = false;
    filterSearchText->Clear();
    filterSearchInItem->SetValue(false);
    filterSearchInDescription->SetValue(false);
    filterSearchInReceipt->SetValue(false);
    filterTypeChoice->SetSelection(0);
    filterCategoryCombo->SetSelection(0);

    // 정렬 상태도 초기화
    currentSortColumn = -1;
    currentSortAscending = true;
    listCtrl->ShowSortIndicator(wxNOT_FOUND, true);

    applyCurrentFilter();
    updateSummaryPanel();
}

// 카테고리 필터 업데이트 (거래 추가/로드 시 호출)
void wxMLMainFrame::updateCategoryFilter()
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller) return;

    // 현재 선택 저장
    wxString currentSelection = filterCategoryCombo->GetValue();

    // 카테고리 목록 갱신
    filterCategoryCombo->Clear();
    filterCategoryCombo->Append(wxString::FromUTF8("전체"));

    // Controller에서 카테고리 목록 가져오기 (View는 표시만 담당)
    auto categories = controller->GetAllCategories();

    for (const auto& cat : categories)
    {
        filterCategoryCombo->Append(wxString::FromUTF8(cat.c_str()));
    }

    // 이전 선택 복원 또는 "전체" 선택
    const int index = filterCategoryCombo->FindString(currentSelection);
    filterCategoryCombo->SetSelection(index != wxNOT_FOUND ? index : 0);
}

// 현재 필터 적용 (전체 재로드 방식)
void wxMLMainFrame::applyCurrentFilter()
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller) return;

    // 화면 갱신 중지 (깜빡임 방지)
    listCtrl->Freeze();

    // 리스트 완전 클리어
    listCtrl->DeleteAllItems();

    // 필터 조건 생성
    FMLFilterCriteria criteria = buildCurrentFilterCriteria();

    // 필터링된 데이터 가져오기
    auto transactions = controller->GetFilteredTransactionData(criteria);

    // 새 데이터 추가
    for (const auto& trans : transactions)
    {
        DisplayTransaction(trans);
    }

    // 정렬 적용 (정렬 상태가 있으면)
    if (currentSortColumn != -1)
    {
        listCtrl->SortItems(CompareTransactions, reinterpret_cast<wxIntPtr>(this));
        listCtrl->ShowSortIndicator(currentSortColumn, currentSortAscending);
    }

    // 화면 갱신 재개
    listCtrl->Thaw();
}


// 리스트에서 거래 ID로 항목 찾기
long wxMLMainFrame::findListItemByTransactionId(int transactionId)
{
    const int itemCount = listCtrl->GetItemCount();
    for (long i = 0; i < itemCount; i++)
    {
        if (listCtrl->GetItemData(i) == static_cast<long>(transactionId))
        {
            return i;
        }
    }
    return -1;
}

// 리스트에서 거래 ID로 항목 제거
void wxMLMainFrame::removeListItemByTransactionId(int transactionId)
{
    const long index = findListItemByTransactionId(transactionId);
    if (index != -1)
    {
        listCtrl->DeleteItem(index);
    }
}

// 컬럼 헤더 클릭 이벤트 핸들러
void wxMLMainFrame::OnColumnHeaderClick(wxListEvent& event)
{
    int column = event.GetColumn();

    // 정렬 방향 토글
    if (currentSortColumn == column)
    {
        if (currentSortAscending)
        {
            currentSortAscending = false;  // 내림차순
        }
        else
        {
            currentSortColumn = -1;  // 정렬 해제 (ID 순)
            currentSortAscending = true;
        }
    }
    else
    {
        currentSortColumn = column;
        currentSortAscending = true;  // 오름차순
    }

    // 정렬 수행
    applySorting();
}

// 정렬 수행
void wxMLMainFrame::applySorting()
{
    if (currentSortColumn == -1)
    {
        // 정렬 해제: ID 순으로 재표시
        applyCurrentFilter();  // 필터 재적용 (ID 순)

        // 정렬 아이콘 제거 (col = -1로 모든 아이콘 제거)
        listCtrl->ShowSortIndicator(wxNOT_FOUND, true);
    }
    else
    {
        // 정렬 수행
        listCtrl->SortItems(CompareTransactions, reinterpret_cast<wxIntPtr>(this));

        // 정렬 아이콘 표시
        listCtrl->ShowSortIndicator(currentSortColumn, currentSortAscending);
    }
}

// 거래 비교 함수 (정렬용)
int wxCALLBACK wxMLMainFrame::CompareTransactions(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData)
{
    auto* frame = reinterpret_cast<wxMLMainFrame*>(sortData);
    int column = frame->currentSortColumn;
    bool ascending = frame->currentSortAscending;

    // Controller에서 데이터 조회
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller) return 0;

    auto data1 = controller->GetTransactionData(static_cast<int>(item1));
    auto data2 = controller->GetTransactionData(static_cast<int>(item2));

    // ID가 -1이면 존재하지 않는 거래 (에러)
    if (data1.TransactionId == -1 || data2.TransactionId == -1)
    {
        return 0;
    }

    int result = 0;

    switch (column)
    {
    case 0: // 유형 (수입/지출)
        result = static_cast<int>(data1.Type) - static_cast<int>(data2.Type);
        break;

    case 1: // 카테고리 (한글)
        result = data1.Category.compare(data2.Category);
        break;

    case 2: // 항목 (한글)
        result = data1.Item.compare(data2.Item);
        break;

    case 3: // 금액 (숫자)
        if (data1.Amount < data2.Amount)
            result = -1;
        else if (data1.Amount > data2.Amount)
            result = 1;
        else
            result = 0;
        break;

    case 4: // 영수증번호 (숫자/문자열 혼합 지원)
        {
            const bool isNum1 = !data1.ReceiptNumber.empty() &&
                                std::all_of(data1.ReceiptNumber.begin(),
                                           data1.ReceiptNumber.end(), ::isdigit);
            const bool isNum2 = !data2.ReceiptNumber.empty() &&
                                std::all_of(data2.ReceiptNumber.begin(),
                                           data2.ReceiptNumber.end(), ::isdigit);

            if (isNum1 && isNum2) {
                const int64_t receiptNum1 = std::stoll(data1.ReceiptNumber);
                const int64_t receiptNum2 = std::stoll(data2.ReceiptNumber);
                result = (receiptNum1 < receiptNum2) ? -1 : (receiptNum1 > receiptNum2) ? 1 : 0;
            } else {
                result = data1.ReceiptNumber.compare(data2.ReceiptNumber);
            }
            break;
        }

    case 5: // 날짜 (문자열 YYYY-MM-DD)
        result = data1.DateTime.compare(data2.DateTime);
        break;

    default:
        result = 0;
        break;
    }

    // 오름차순/내림차순 적용
    return ascending ? result : -result;
}

// Summary 패널 생성
void wxMLMainFrame::createSummaryPanel(wxPanel* parent, wxBoxSizer* sizer)
{
    summaryPanel = new wxPanel(parent);
    summaryPanel->SetBackgroundColour(wxColour(245, 245, 245));

    wxBoxSizer* summarySizer = new wxBoxSizer(wxHORIZONTAL);

    // 수입 라벨
    wxStaticText* incomeLabel = new wxStaticText(summaryPanel, wxID_ANY, wxString::FromUTF8("수입: "));
    wxFont labelFont = incomeLabel->GetFont();
    labelFont.SetWeight(wxFONTWEIGHT_BOLD);
    incomeLabel->SetFont(labelFont);
    summarySizer->Add(incomeLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);

    summaryIncomeText = new wxStaticText(summaryPanel, wxID_ANY, "0");
    summaryIncomeText->SetForegroundColour(wxColour(0, 128, 0)); // 녹색
    wxFont amountFont = summaryIncomeText->GetFont();
    amountFont.SetWeight(wxFONTWEIGHT_BOLD);
    summaryIncomeText->SetFont(amountFont);
    summarySizer->Add(summaryIncomeText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);

    // 지출 라벨
    wxStaticText* expenseLabel = new wxStaticText(summaryPanel, wxID_ANY, wxString::FromUTF8("지출: "));
    expenseLabel->SetFont(labelFont);
    summarySizer->Add(expenseLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 60);

    summaryExpenseText = new wxStaticText(summaryPanel, wxID_ANY, "0");
    summaryExpenseText->SetForegroundColour(wxColour(200, 0, 0)); // 빨강
    summaryExpenseText->SetFont(amountFont);
    summarySizer->Add(summaryExpenseText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);

    // 잔액 라벨
    wxStaticText* balanceLabel = new wxStaticText(summaryPanel, wxID_ANY, wxString::FromUTF8("잔액: "));
    balanceLabel->SetFont(labelFont);
    summarySizer->Add(balanceLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 60);

    summaryBalanceText = new wxStaticText(summaryPanel, wxID_ANY, "0");
    summaryBalanceText->SetFont(amountFont);
    summarySizer->Add(summaryBalanceText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    summaryPanel->SetSizer(summarySizer);
    sizer->Add(summaryPanel, 0, wxEXPAND | wxALL, 5);
}

// Summary 데이터 갱신
void wxMLMainFrame::updateSummaryPanel()
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller) return;

    FMLTransactionSummary summary;

    if (FilterActive)
    {
        FMLFilterCriteria criteria = buildCurrentFilterCriteria();
        summary = controller->GetFilteredTransactionSummary(criteria);
    }
    else
    {
        summary = controller->GetTransactionSummary();
    }

    displaySummary(summary);
}

// Summary 데이터 UI에 표시
void wxMLMainFrame::displaySummary(const FMLTransactionSummary& summary)
{
    summaryIncomeText->SetLabel(wxString::FromUTF8(FMLFormat::FormatAmount(summary.TotalIncome)));
    summaryExpenseText->SetLabel(wxString::FromUTF8(FMLFormat::FormatAmount(summary.TotalExpense)));

    wxString balanceText = wxString::FromUTF8(FMLFormat::FormatAmount(summary.Balance));
    summaryBalanceText->SetLabel(balanceText);

    // 잔액 색상 설정 (양수: 파랑, 음수: 빨강)
    if (summary.Balance >= 0)
    {
        summaryBalanceText->SetForegroundColour(wxColour(0, 0, 200)); // 파랑
    }
    else
    {
        summaryBalanceText->SetForegroundColour(wxColour(200, 0, 0)); // 빨강
    }
}

// 천 단위 구분 포맷팅

// 현재 필터 조건 생성
FMLFilterCriteria wxMLMainFrame::buildCurrentFilterCriteria()
{
    FMLFilterCriteria criteria;

    // 거래 유형 필터
    const int typeSelection = filterTypeChoice->GetSelection();
    if (typeSelection == 1) // 수입
    {
        criteria.UseTypeFilter = true;
        criteria.TypeFilter = E_MLTransactionType::Income;
    }
    else if (typeSelection == 2) // 지출
    {
        criteria.UseTypeFilter = true;
        criteria.TypeFilter = E_MLTransactionType::Expense;
    }

    // 날짜 범위 필터 (FilterActive일 때만)
    if (FilterActive)
    {
        const std::string startDate = filterStartDateText->GetValue().ToStdString();
        const std::string endDate = filterEndDateText->GetValue().ToStdString();
        
        criteria.UseDateRangeFilter = !startDate.empty() && !endDate.empty();
        criteria.StartDate = startDate;
        criteria.EndDate = endDate;
    }

    // 카테고리 필터
    const int categorySelection = filterCategoryCombo->GetSelection();
    if (categorySelection > 0) // "전체"가 아닌 경우
    {
        criteria.UseCategoryFilter = true;
        criteria.CategoryFilter = filterCategoryCombo->GetValue().ToUTF8().data();
    }

    // 검색어 필터
    const std::string searchText = filterSearchText->GetValue().ToUTF8().data();
    if (!searchText.empty())
    {
        criteria.UseTextSearch = true;
        criteria.SearchText = searchText;
        criteria.SearchInItem = filterSearchInItem->GetValue();
        criteria.SearchInDescription = filterSearchInDescription->GetValue();
        criteria.SearchInReceipt = filterSearchInReceipt->GetValue();
    }

    return criteria;
}

// Helper methods
void wxMLMainFrame::showCalendarDialog(wxTextCtrl* targetTextCtrl)
{
    wxDateTime currentDate = parseDate(targetTextCtrl->GetValue());

    wxDialog dialog(this, wxID_ANY, wxString::FromUTF8("날짜 선택"), wxDefaultPosition, wxDefaultSize);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxCalendarCtrl* calendar = new wxCalendarCtrl(&dialog, wxID_ANY, currentDate);
    sizer->Add(calendar, 1, wxEXPAND | wxALL, 10);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* okButton = new wxButton(&dialog, wxID_OK, wxString::FromUTF8("확인"));
    wxButton* cancelButton = new wxButton(&dialog, wxID_CANCEL, wxString::FromUTF8("취소"));
    buttonSizer->Add(okButton, 0, wxRIGHT, 5);
    buttonSizer->Add(cancelButton, 0);
    sizer->Add(buttonSizer, 0, wxALIGN_RIGHT | wxALL, 10);

    dialog.SetSizer(sizer);
    dialog.Fit();

    if (dialog.ShowModal() == wxID_OK)
    {
        wxDateTime selectedDate = calendar->GetDate();
        targetTextCtrl->SetValue(formatDate(selectedDate));
    }
}

bool wxMLMainFrame::validateDateFormat(const wxString& dateStr)
{
    if (dateStr.length() != 10) return false;
    if (dateStr[4] != '-' || dateStr[7] != '-') return false;

    for (int i = 0; i < 10; i++)
    {
        if (i == 4 || i == 7) continue;
        if (!wxIsdigit(dateStr[i])) return false;
    }

    return true;
}

wxString wxMLMainFrame::formatDate(const wxDateTime& date)
{
    return date.FormatISODate(); // YYYY-MM-DD
}

wxDateTime wxMLMainFrame::parseDate(const wxString& dateStr)
{
    if (dateStr.IsEmpty() || !validateDateFormat(dateStr))
    {
        return wxDateTime::Now();
    }

    wxDateTime date;
    if (date.ParseISODate(dateStr))
    {
        return date;
    }

    return wxDateTime::Now();
}

// 환산 금액 계산 및 표시
void wxMLMainFrame::updateConvertedAmount()
{
    if (!exchangeRateCheckBox->GetValue())
    {
        return;
    }

    wxString originalAmountStr = originalAmountText->GetValue();
    wxString exchangeRateStr = exchangeRateText->GetValue();

    if (originalAmountStr.IsEmpty() || exchangeRateStr.IsEmpty())
    {
        convertedAmountLabel->SetLabel(wxString::FromUTF8("= 원화 환산: 0원"));
        return;
    }

    double originalAmount = 0.0;
    double exchangeRate = 0.0;

    if (!originalAmountStr.ToDouble(&originalAmount) || !exchangeRateStr.ToDouble(&exchangeRate))
    {
        convertedAmountLabel->SetLabel(wxString::FromUTF8("= 원화 환산: (입력 오류)"));
        return;
    }

    int64_t convertedAmount = static_cast<int64_t>(originalAmount * exchangeRate);
    wxString amountStr = wxString::FromUTF8(FMLFormat::FormatAmount(convertedAmount));
    convertedAmountLabel->SetLabel(wxString::FromUTF8("= 원화 환산: ") + amountStr + wxString::FromUTF8("원"));

    // 금액 필드에 자동 설정 (참고용, 실제 저장 시에는 계산된 값 사용)
    amountText->SetValue(wxString::Format("%lld", convertedAmount));
}

// 입력 필드에서 거래 데이터 수집 및 검증
bool wxMLMainFrame::collectTransactionDataFromInput(FMLTransactionData& outData)
{
    // 거래 유형
    outData.Type = incomeRadio->GetValue() ? E_MLTransactionType::Income : E_MLTransactionType::Expense;

    // 카테고리, 항목, 설명, 영수증 번호
    outData.Category = categoryText->GetValue().ToUTF8().data();
    outData.Item = itemText->GetValue().ToUTF8().data();
    outData.Description = descriptionText->GetValue().ToUTF8().data();
    outData.ReceiptNumber = receiptText->GetValue().ToUTF8().data();

    // 금액 파싱
    long long amount = 0;

    // 환율 적용 여부 확인
    if (exchangeRateCheckBox->GetValue()) {
        // 환율 적용: 외화 금액과 환율로 계산
        wxString originalAmountStr = originalAmountText->GetValue();
        wxString exchangeRateStr = exchangeRateText->GetValue();

        if (originalAmountStr.IsEmpty() || exchangeRateStr.IsEmpty()) {
            wxMessageBox(wxString::FromUTF8("외화 금액과 환율을 입력하세요."),
                        wxString::FromUTF8("입력 오류"), wxOK | wxICON_ERROR);
            return false;
        }

        double originalAmount = 0.0;
        double exchangeRate = 0.0;

        if (!originalAmountStr.ToDouble(&originalAmount) || !exchangeRateStr.ToDouble(&exchangeRate)) {
            wxMessageBox(wxString::FromUTF8("올바른 외화 금액과 환율을 입력하세요."),
                        wxString::FromUTF8("입력 오류"), wxOK | wxICON_ERROR);
            return false;
        }

        amount = static_cast<int64_t>(originalAmount * exchangeRate);

        // 환율 데이터 설정
        outData.UseExchangeRate = true;
        outData.Currency = currencyCombo->GetValue().ToUTF8().data();
        outData.OriginalAmount = originalAmount;
        outData.ExchangeRate = exchangeRate;
    } else {
        // 일반 금액 입력
        wxString amountStr = amountText->GetValue();
        if (!amountStr.ToLongLong(&amount)) {
            wxMessageBox(wxString::FromUTF8("올바른 금액을 입력하세요."),
                        wxString::FromUTF8("입력 오류"), wxOK | wxICON_ERROR);
            return false;
        }

        // 환율 미적용
        outData.UseExchangeRate = false;
    }

    outData.Amount = amount;

    // 날짜 가져오기
    outData.DateTime = dateText->GetValue().ToStdString();

    return true;
}

// ========== 예산 관련 메서드 구현 ==========

// 예산 탭 UI 생성
wxPanel* wxMLMainFrame::createBudgetTab()
{
    wxPanel* budgetTab = new wxPanel(notebook);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // ========== 입력 패널 ==========
    wxPanel* inputPanel = new wxPanel(budgetTab);
    wxBoxSizer* inputSizer = new wxBoxSizer(wxVERTICAL);

    // 구분 (수입/지출)
    wxBoxSizer* typeSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* typeLabel = new wxStaticText(inputPanel, wxID_ANY, wxString::FromUTF8("구분:"));
    budgetIncomeRadio = new wxRadioButton(inputPanel, wxID_ANY, wxString::FromUTF8("수입"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    budgetExpenseRadio = new wxRadioButton(inputPanel, wxID_ANY, wxString::FromUTF8("지출"));
    budgetExpenseRadio->SetValue(true);
    typeSizer->Add(typeLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    typeSizer->Add(budgetIncomeRadio, 0, wxRIGHT, 10);
    typeSizer->Add(budgetExpenseRadio, 0, 0, 0);
    inputSizer->Add(typeSizer, 0, wxALL, 5);

    // 카테고리
    wxBoxSizer* categorySizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* categoryLabel = new wxStaticText(inputPanel, wxID_ANY, wxString::FromUTF8("카테고리:"));
    budgetCategoryText = new wxTextCtrl(inputPanel, wxID_ANY, "", wxDefaultPosition, wxSize(200, -1));
    categorySizer->Add(categoryLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    categorySizer->Add(budgetCategoryText, 1, wxEXPAND, 0);
    inputSizer->Add(categorySizer, 0, wxEXPAND | wxALL, 5);

    // 항목
    wxBoxSizer* itemSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* itemLabel = new wxStaticText(inputPanel, wxID_ANY, wxString::FromUTF8("항목:"));
    budgetItemText = new wxTextCtrl(inputPanel, wxID_ANY, "", wxDefaultPosition, wxSize(200, -1));
    itemSizer->Add(itemLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    itemSizer->Add(budgetItemText, 1, wxEXPAND, 0);
    inputSizer->Add(itemSizer, 0, wxEXPAND | wxALL, 5);

    // 예산금액
    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* amountLabel = new wxStaticText(inputPanel, wxID_ANY, wxString::FromUTF8("예산금액:"));
    budgetAmountText = new wxTextCtrl(inputPanel, wxID_ANY, "", wxDefaultPosition, wxSize(200, -1));
    amountSizer->Add(amountLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    amountSizer->Add(budgetAmountText, 1, wxEXPAND, 0);
    inputSizer->Add(amountSizer, 0, wxEXPAND | wxALL, 5);

    // 비고
    wxBoxSizer* notesSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* notesLabel = new wxStaticText(inputPanel, wxID_ANY, wxString::FromUTF8("비고:"));
    budgetNotesText = new wxTextCtrl(inputPanel, wxID_ANY, "", wxDefaultPosition, wxSize(200, -1));
    notesSizer->Add(notesLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    notesSizer->Add(budgetNotesText, 1, wxEXPAND, 0);
    inputSizer->Add(notesSizer, 0, wxEXPAND | wxALL, 5);

    inputPanel->SetSizer(inputSizer);
    mainSizer->Add(inputPanel, 0, wxEXPAND | wxALL, 10);

    // ========== 버튼 패널 ==========
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    addBudgetButton = new wxButton(budgetTab, wxID_ANY, wxString::FromUTF8("추가"));
    updateBudgetButton = new wxButton(budgetTab, wxID_ANY, wxString::FromUTF8("수정"));
    deleteBudgetButton = new wxButton(budgetTab, wxID_ANY, wxString::FromUTF8("삭제"));
    updateBudgetButton->Enable(false);
    deleteBudgetButton->Enable(false);
    buttonSizer->Add(addBudgetButton, 0, wxRIGHT, 5);
    buttonSizer->Add(updateBudgetButton, 0, wxRIGHT, 5);
    buttonSizer->Add(deleteBudgetButton, 0, 0, 0);
    mainSizer->Add(buttonSizer, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // ========== 2단 패널 (카테고리 + 항목) ==========
    wxBoxSizer* listSizer = new wxBoxSizer(wxHORIZONTAL);

    // 왼쪽: 카테고리 목록
    wxBoxSizer* categoryListSizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* categoryListLabel = new wxStaticText(budgetTab, wxID_ANY, wxString::FromUTF8("카테고리"));
    categoryListBox = new wxListBox(budgetTab, wxID_ANY, wxDefaultPosition, wxSize(150, -1));
    categoryListSizer->Add(categoryListLabel, 0, wxBOTTOM, 5);
    categoryListSizer->Add(categoryListBox, 1, wxEXPAND, 0);
    listSizer->Add(categoryListSizer, 0, wxEXPAND | wxRIGHT, 10);

    // 오른쪽: 항목 목록
    wxBoxSizer* itemListSizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* itemListLabel = new wxStaticText(budgetTab, wxID_ANY, wxString::FromUTF8("항목 목록"));
    budgetItemListCtrl = new wxListCtrl(budgetTab, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    budgetItemListCtrl->InsertColumn(0, wxString::FromUTF8("항목"), wxLIST_FORMAT_LEFT, 200);
    budgetItemListCtrl->InsertColumn(1, wxString::FromUTF8("예산"), wxLIST_FORMAT_RIGHT, 150);
    itemListSizer->Add(itemListLabel, 0, wxBOTTOM, 5);
    itemListSizer->Add(budgetItemListCtrl, 1, wxEXPAND, 0);
    listSizer->Add(itemListSizer, 1, wxEXPAND, 0);

    mainSizer->Add(listSizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // ========== 요약 패널 ==========
    budgetSummaryPanel = new wxPanel(budgetTab);
    budgetSummaryPanel->SetBackgroundColour(wxColour(245, 245, 245));
    wxBoxSizer* summarySizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* budgetLabel = new wxStaticText(budgetSummaryPanel, wxID_ANY, wxString::FromUTF8("총 예산: "));
    wxFont labelFont = budgetLabel->GetFont();
    labelFont.SetWeight(wxFONTWEIGHT_BOLD);
    budgetLabel->SetFont(labelFont);
    summarySizer->Add(budgetLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 10);

    budgetSummaryTotalBudgetText = new wxStaticText(budgetSummaryPanel, wxID_ANY, "0");
    wxFont amountFont = budgetSummaryTotalBudgetText->GetFont();
    amountFont.SetWeight(wxFONTWEIGHT_BOLD);
    budgetSummaryTotalBudgetText->SetFont(amountFont);
    summarySizer->Add(budgetSummaryTotalBudgetText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    budgetSummaryPanel->SetSizer(summarySizer);
    mainSizer->Add(budgetSummaryPanel, 0, wxEXPAND | wxALL, 5);

    budgetTab->SetSizer(mainSizer);

    // ========== 이벤트 바인딩 ==========
    addBudgetButton->Bind(wxEVT_BUTTON, &wxMLMainFrame::OnAddBudget, this);
    updateBudgetButton->Bind(wxEVT_BUTTON, &wxMLMainFrame::OnEditBudget, this);
    deleteBudgetButton->Bind(wxEVT_BUTTON, &wxMLMainFrame::OnDeleteBudget, this);
    categoryListBox->Bind(wxEVT_LISTBOX, &wxMLMainFrame::onCategorySelected, this);
    budgetItemListCtrl->Bind(wxEVT_LIST_ITEM_SELECTED, &wxMLMainFrame::OnBudgetListItemSelected, this);
    budgetItemListCtrl->Bind(wxEVT_LIST_ITEM_DESELECTED, &wxMLMainFrame::OnBudgetListItemDeselected, this);

    return budgetTab;
}

// ========== Budget 헬퍼 메서드 구현 ==========

// 예산 입력 필드 초기화
void wxMLMainFrame::clearBudgetInputFields()
{
    budgetExpenseRadio->SetValue(true);
    budgetCategoryText->Clear();
    budgetItemText->Clear();
    budgetAmountText->Clear();
    budgetNotesText->Clear();
}

// 선택된 예산 데이터를 입력 필드에 로드
void wxMLMainFrame::loadBudgetToInput(int budgetId)
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller) return;

    FMLBudgetData data = controller->GetBudget(budgetId);

    // 거래 유형
    if (data.Type == E_MLTransactionType::Income) {
        budgetIncomeRadio->SetValue(true);
    } else {
        budgetExpenseRadio->SetValue(true);
    }

    // 텍스트 필드
    budgetCategoryText->SetValue(wxString::FromUTF8(data.Category.c_str()));
    budgetItemText->SetValue(wxString::FromUTF8(data.Item.c_str()));
    budgetAmountText->SetValue(wxString::Format("%lld", data.BudgetAmount));
    budgetNotesText->SetValue(wxString::FromUTF8(data.Notes.c_str()));
}

// 입력 필드에서 예산 데이터 수집 및 검증
bool wxMLMainFrame::collectBudgetDataFromInput(FMLBudgetData& outData)
{
    // 거래 유형
    outData.Type = budgetIncomeRadio->GetValue() ? E_MLTransactionType::Income : E_MLTransactionType::Expense;

    // 카테고리, 항목, 비고
    outData.Category = budgetCategoryText->GetValue().ToUTF8().data();
    outData.Item = budgetItemText->GetValue().ToUTF8().data();
    outData.Notes = budgetNotesText->GetValue().ToUTF8().data();

    // 카테고리 필수 체크
    if (outData.Category.empty()) {
        wxMessageBox(wxString::FromUTF8("카테고리를 입력하세요."), wxString::FromUTF8("입력 오류"), wxOK | wxICON_WARNING);
        budgetCategoryText->SetFocus();
        return false;
    }

    // 예산 금액 파싱
    wxString amountStr = budgetAmountText->GetValue();
    if (amountStr.IsEmpty()) {
        wxMessageBox(wxString::FromUTF8("예산 금액을 입력하세요."), wxString::FromUTF8("입력 오류"), wxOK | wxICON_WARNING);
        budgetAmountText->SetFocus();
        return false;
    }

    long long budgetAmount = 0;
    if (!amountStr.ToLongLong(&budgetAmount) || budgetAmount < 0) {
        wxMessageBox(wxString::FromUTF8("유효한 예산 금액을 입력하세요."), wxString::FromUTF8("입력 오류"), wxOK | wxICON_WARNING);
        budgetAmountText->SetFocus();
        return false;
    }

    outData.BudgetAmount = budgetAmount;
    outData.BudgetId = SelectedBudgetId;  // 수정 시 사용

    return true;
}

// 카테고리 목록 업데이트
void wxMLMainFrame::updateCategoryList()
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller) return;

    categoryListBox->Freeze();
    categoryListBox->Clear();

    // 모든 예산 가져오기
    auto budgets = controller->GetAllBudgets();

    // 카테고리 추출 (중복 제거)
    std::set<std::string> categories;
    for (const auto& budget : budgets) {
        categories.insert(budget.Category);
    }

    // 카테고리 목록에 추가
    for (const auto& cat : categories) {
        categoryListBox->Append(wxString::FromUTF8(cat.c_str()));
    }

    categoryListBox->Thaw();

    // 요약 업데이트
    FMLBudgetSummary summary = controller->GetBudgetSummary();
    displayBudgetSummary(summary);
}

// 선택된 카테고리의 항목 목록 업데이트
void wxMLMainFrame::updateBudgetItemList()
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller) return;

    budgetItemListCtrl->Freeze();
    budgetItemListCtrl->DeleteAllItems();

    if (SelectedBudgetCategory.empty()) {
        budgetItemListCtrl->Thaw();
        return;
    }

    // 선택된 카테고리의 예산 항목들 가져오기
    auto budgets = controller->GetAllBudgets();

    for (const auto& budget : budgets) {
        if (budget.Category == SelectedBudgetCategory) {
            long index = budgetItemListCtrl->InsertItem(budgetItemListCtrl->GetItemCount(),
                                                         wxString::FromUTF8(budget.Item.c_str()));

            budgetItemListCtrl->SetItem(index, 1, wxString::FromUTF8(FMLFormat::FormatAmount(budget.BudgetAmount)));

            // BudgetId를 ItemData로 저장
            budgetItemListCtrl->SetItemData(index, budget.BudgetId);
        }
    }

    budgetItemListCtrl->Thaw();
}

// 예산 요약 표시 (재구현)
void wxMLMainFrame::displayBudgetSummary(const FMLBudgetSummary& summary)
{
    wxString totalBudget = wxString::FromUTF8(FMLFormat::FormatAmount(summary.TotalBudget));
    budgetSummaryTotalBudgetText->SetLabel(totalBudget);
}

// 예산 버튼 상태 업데이트 (재구현)
void wxMLMainFrame::updateBudgetButtonStates()
{
    bool hasSelection = (SelectedBudgetId != -1);
    updateBudgetButton->Enable(hasSelection);
    deleteBudgetButton->Enable(hasSelection);
}

// 카테고리 선택 이벤트
void wxMLMainFrame::onCategorySelected(wxCommandEvent& event)
{
    int selection = categoryListBox->GetSelection();
    if (selection != wxNOT_FOUND) {
        SelectedBudgetCategory = categoryListBox->GetString(selection).ToUTF8().data();
        updateBudgetItemList();
    } else {
        SelectedBudgetCategory.clear();
    }

    // 항목 선택 해제
    SelectedBudgetId = -1;
    updateBudgetButtonStates();
}

// ========== IMLView 메서드 구현 ==========

void wxMLMainFrame::AddBudget(const FMLBudgetData& data)
{
    clearBudgetInputFields();
    updateCategoryList();
    updateTitle();
}

void wxMLMainFrame::DisplayBudget(const FMLBudgetData& data)
{
    // 개별 예산 표시는 updateBudgetItemList에서 처리
    updateBudgetItemList();
}

void wxMLMainFrame::DisplayBudgets(const std::vector<FMLBudgetData>& data)
{
    updateCategoryList();
}

// ========== Observer 메서드 구현 ==========

void wxMLMainFrame::OnBudgetAdded(const FMLBudgetData& budgetData)
{
    updateCategoryList();
    updateTitle();
}

void wxMLMainFrame::OnBudgetDeleted(const int budgetId)
{
    if (SelectedBudgetId == budgetId) {
        SelectedBudgetId = -1;
        clearBudgetInputFields();
    }
    updateCategoryList();
    updateTitle();
}

void wxMLMainFrame::OnBudgetUpdated(const FMLBudgetData& budgetData)
{
    updateBudgetItemList();
    updateTitle();
}

void wxMLMainFrame::OnBudgetCleared()
{
    clearBudgetInputFields();
    SelectedBudgetCategory.clear();
    SelectedBudgetId = -1;
    updateCategoryList();
}

// ========== 이벤트 핸들러 구현 ==========

void wxMLMainFrame::OnAddBudget(wxCommandEvent& event)
{
    FMLBudgetData data;
    if (!collectBudgetDataFromInput(data)) {
        return;
    }

    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (controller) {
        if (controller->AddBudget(data)) {
            clearBudgetInputFields();
        } else {
            wxMessageBox(wxString::FromUTF8("예산 추가에 실패했습니다."),
                         wxString::FromUTF8("오류"), wxOK | wxICON_ERROR);
        }
    }
}

void wxMLMainFrame::OnEditBudget(wxCommandEvent& event)
{
    if (SelectedBudgetId == -1) return;

    FMLBudgetData data;
    if (!collectBudgetDataFromInput(data)) {
        return;
    }

    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (controller) {
        if (controller->UpdateBudget(data)) {
            clearBudgetInputFields();
            SelectedBudgetId = -1;
            updateBudgetButtonStates();
        } else {
            wxMessageBox(wxString::FromUTF8("예산 수정에 실패했습니다."),
                         wxString::FromUTF8("오류"), wxOK | wxICON_ERROR);
        }
    }
}

void wxMLMainFrame::OnDeleteBudget(wxCommandEvent& event)
{
    if (SelectedBudgetId == -1) return;

    int result = wxMessageBox(
        wxString::FromUTF8("선택한 예산을 삭제하시겠습니까?"),
        wxString::FromUTF8("삭제 확인"),
        wxYES_NO | wxICON_QUESTION
    );

    if (result != wxYES) return;

    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (controller) {
        if (controller->DeleteBudget(SelectedBudgetId)) {
            clearBudgetInputFields();
            SelectedBudgetId = -1;
            updateBudgetButtonStates();
        } else {
            wxMessageBox(wxString::FromUTF8("예산 삭제에 실패했습니다."),
                         wxString::FromUTF8("오류"), wxOK | wxICON_ERROR);
        }
    }
}

void wxMLMainFrame::OnBudgetListItemSelected(wxListEvent& event)
{
    const long selectedIndex = event.GetIndex();
    SelectedBudgetId = static_cast<int>(budgetItemListCtrl->GetItemData(selectedIndex));
    loadBudgetToInput(SelectedBudgetId);
    updateBudgetButtonStates();
}

void wxMLMainFrame::OnBudgetListItemDeselected(wxListEvent& event)
{
    SelectedBudgetId = -1;
    clearBudgetInputFields();
    updateBudgetButtonStates();
}
