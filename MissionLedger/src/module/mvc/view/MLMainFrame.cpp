#include "MLMainFrame.h"
#include <wx/statline.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/calctrl.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <set>
#include "MLDefine.h"
#include "module/common/holder/MLMVCHolder.h"
#include "interface/IMLController.h"

// 메뉴 ID 정의
enum
{
    ID_NEW_FILE = wxID_NEW,
    ID_OPEN_FILE = wxID_OPEN,
    ID_SAVE_FILE = wxID_SAVE,
    ID_SAVE_FILE_AS = wxID_SAVEAS,
    ID_EXIT = wxID_EXIT
};

wxMLMainFrame::wxMLMainFrame()
    : wxFrame(nullptr, wxID_ANY, wxString::FromUTF8("MissionLedger - 거래 관리"), wxDefaultPosition, wxSize(1200, 800))
{
    // 메인 패널
    wxPanel* mainPanel = new wxPanel(this);

    // 수평 박스 사이저 (왼쪽 입력 패널 + 오른쪽 리스트)
    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

    // === 왼쪽 입력 패널 ===
    wxPanel* inputPanel = new wxPanel(mainPanel);
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
    wxPanel* rightPanel = new wxPanel(mainPanel);
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

    // 메인 사이저에 패널 추가
    mainSizer->Add(inputPanel, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(rightPanel, 2, wxEXPAND | wxALL, 5);

    mainPanel->SetSizer(mainSizer);

    // 버튼 이벤트 바인딩
    addButton->Bind(wxEVT_BUTTON, &wxMLMainFrame::OnAddTransaction, this);
    updateButton->Bind(wxEVT_BUTTON, &wxMLMainFrame::OnUpdateTransaction, this);
    deleteButton->Bind(wxEVT_BUTTON, &wxMLMainFrame::OnDeleteTransaction, this);

    // 리스트 선택 이벤트 바인딩
    listCtrl->Bind(wxEVT_LIST_ITEM_SELECTED, &wxMLMainFrame::OnListItemSelected, this);
    listCtrl->Bind(wxEVT_LIST_ITEM_DESELECTED, &wxMLMainFrame::OnListItemDeselected, this);

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
    listCtrl->SetItem(index, 3, formatAmountWithComma(data.Amount));
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
    // 입력 데이터 수집
    FMLTransactionData data;
    data.Type = incomeRadio->GetValue() ? E_MLTransactionType::Income : E_MLTransactionType::Expense;
    data.Category = categoryText->GetValue().ToUTF8().data();
    data.Item = itemText->GetValue().ToUTF8().data();
    data.Description = descriptionText->GetValue().ToUTF8().data();
    data.ReceiptNumber = receiptText->GetValue().ToUTF8().data();

    // 금액 파싱
    wxString amountStr = amountText->GetValue();
    long long amount = 0;
    if (!amountStr.ToLongLong(&amount)) {
        wxMessageBox(wxString::FromUTF8("올바른 금액을 입력하세요."), wxString::FromUTF8("입력 오류"), wxOK | wxICON_ERROR);
        return;
    }
    data.Amount = amount;

    // 날짜 가져오기
    data.DateTime = dateText->GetValue().ToStdString();

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
    dateText->SetValue(formatDate(wxDateTime::Now()));
}

// IMLModelObserver 인터페이스 구현
void wxMLMainFrame::OnTransactionAdded(const FMLTransactionData& transactionData)
{
    updateCategoryFilter();

    if (FilterActive)
    {
        // 필터 활성화 상태: 증분 업데이트로 필터 재적용
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

void wxMLMainFrame::OnTransactionRemoved(int transactionId)
{
    updateCategoryFilter();

    if (FilterActive)
    {
        // 필터 활성화 상태: 증분 업데이트로 필터 재적용
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
        // 필터 활성화 상태: 증분 업데이트로 필터 재적용
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
    // 데이터 로드 시 증분 업데이트 (FilterActive 상태 유지)
    updateCategoryFilter();
    applyCurrentFilter();
    updateSummaryPanel();
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

    // 입력 데이터 수집
    FMLTransactionData data;
    data.TransactionId = SelectedTransactionId;
    data.Type = incomeRadio->GetValue() ? E_MLTransactionType::Income : E_MLTransactionType::Expense;
    data.Category = categoryText->GetValue().ToUTF8().data();
    data.Item = itemText->GetValue().ToUTF8().data();
    data.Description = descriptionText->GetValue().ToUTF8().data();
    data.ReceiptNumber = receiptText->GetValue().ToUTF8().data();

    // 금액 파싱
    wxString amountStr = amountText->GetValue();
    long long amount = 0;
    if (!amountStr.ToLongLong(&amount)) {
        wxMessageBox(wxString::FromUTF8("올바른 금액을 입력하세요."), wxString::FromUTF8("입력 오류"), wxOK | wxICON_ERROR);
        return;
    }
    data.Amount = amount;

    // 날짜 가져오기
    data.DateTime = dateText->GetValue().ToStdString();

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
        if (controller->RemoveTransaction(SelectedTransactionId)) {
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
    fileMenu->Append(ID_EXIT, wxString::FromUTF8("종료(&X)\tAlt+F4"));

    menuBar->Append(fileMenu, wxString::FromUTF8("파일(&F)"));

    SetMenuBar(menuBar);

    // 메뉴 이벤트 바인딩
    Bind(wxEVT_MENU, &wxMLMainFrame::OnNewFile, this, ID_NEW_FILE);
    Bind(wxEVT_MENU, &wxMLMainFrame::OnOpenFile, this, ID_OPEN_FILE);
    Bind(wxEVT_MENU, &wxMLMainFrame::OnSaveFile, this, ID_SAVE_FILE);
    Bind(wxEVT_MENU, &wxMLMainFrame::OnSaveFileAs, this, ID_SAVE_FILE_AS);
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
            wxFileName fileName(filePath);
            title = fileName.GetFullName() + " - " + title;
        }
        else
        {
            title = wxString::FromUTF8("새 파일 - ") + title;
        }

        if (controller->HasUnsavedChanges())
        {
            title = "* " + title;
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
    applyCurrentFilter();  // 증분 업데이트 사용
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

    auto allTransactions = controller->GetAllTransactionData();
    std::set<std::string> categories;

    for (const auto& trans : allTransactions)
    {
        if (!trans.Category.empty())
        {
            categories.insert(trans.Category);
        }
    }

    for (const auto& cat : categories)
    {
        filterCategoryCombo->Append(wxString::FromUTF8(cat.c_str()));
    }

    // 이전 선택 복원 또는 "전체" 선택
    const int index = filterCategoryCombo->FindString(currentSelection);
    filterCategoryCombo->SetSelection(index != wxNOT_FOUND ? index : 0);
}

// 현재 필터 적용 (증분 업데이트 방식)
void wxMLMainFrame::applyCurrentFilter()
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller) return;

    // 현재 리스트에 표시된 ID 목록 가져오기 (Single Source of Truth)
    std::set<int> previousIds = getCurrentListIds();

    // 필터 조건 생성
    FMLFilterCriteria criteria = buildCurrentFilterCriteria();
    
    // 새로운 필터 결과 가져오기
    auto transactions = controller->GetFilteredTransactionData(criteria);

    // 새로운 ID 목록 생성
    std::set<int> currentIds;
    std::map<int, FMLTransactionData> transactionMap;
    for (const auto& trans : transactions)
    {
        currentIds.insert(trans.TransactionId);
        transactionMap[trans.TransactionId] = trans;
    }

    // 증분 업데이트: 제거된 항목 삭제
    std::vector<int> idsToRemove;
    for (const int& prevId : previousIds)
    {
        if (currentIds.find(prevId) == currentIds.end())
        {
            idsToRemove.push_back(prevId);
        }
    }
    for (const int& idToRemove : idsToRemove)
    {
        removeListItemByTransactionId(idToRemove);
    }

    // 증분 업데이트: 추가된 항목 삽입
    for (const int& currentId : currentIds)
    {
        if (previousIds.find(currentId) == previousIds.end())
        {
            // 새로 추가된 항목
            DisplayTransaction(transactionMap[currentId]);
        }
    }
}

// 현재 리스트에 표시된 모든 거래 ID 가져오기 (Single Source of Truth)
std::set<int> wxMLMainFrame::getCurrentListIds()
{
    std::set<int> ids;
    const int itemCount = listCtrl->GetItemCount();
    for (long i = 0; i < itemCount; i++)
    {
        ids.insert(static_cast<int>(listCtrl->GetItemData(i)));
    }
    return ids;
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
    summaryIncomeText->SetLabel(formatAmountWithComma(summary.TotalIncome));
    summaryExpenseText->SetLabel(formatAmountWithComma(summary.TotalExpense));

    wxString balanceText = formatAmountWithComma(summary.Balance);
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
wxString wxMLMainFrame::formatAmountWithComma(int64_t amount)
{
    bool negative = (amount < 0);
    int64_t absAmount = negative ? -amount : amount;

    std::string numStr = std::to_string(absAmount);
    std::string result;

    int count = 0;
    for (auto it = numStr.rbegin(); it != numStr.rend(); ++it)
    {
        if (count > 0 && count % 3 == 0)
        {
            result = ',' + result;
        }
        result = *it + result;
        count++;
    }

    if (negative)
    {
        result = "-" + result;
    }

    return wxString::FromUTF8(result.c_str());
}

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
        criteria.UseDateRangeFilter = true;
        criteria.StartDate = filterStartDateText->GetValue().ToStdString();
        criteria.EndDate = filterEndDateText->GetValue().ToStdString();
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

