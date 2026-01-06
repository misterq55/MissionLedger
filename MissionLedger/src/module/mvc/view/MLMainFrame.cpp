#include "MLMainFrame.h"
#include <wx/statline.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include "MLDefine.h"
#include "module/common/holder/MLMVCHolder.h"
#include "interface/IMLController.h"
#include "interface/IMLModel.h"

wxMLMainFrame::wxMLMainFrame()
    : wxFrame(nullptr, wxID_ANY, wxString::FromUTF8("MissionLedger - 거래 관리"), wxDefaultPosition, wxSize(900, 600))
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
    datePicker = new wxDatePickerCtrl(inputPanel, wxID_ANY, wxDefaultDateTime,
        wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT | wxDP_DROPDOWN);
    inputSizer->Add(datePicker, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    inputSizer->AddSpacer(15);

    // 추가 버튼
    wxButton* addButton = new wxButton(inputPanel, wxID_ANY, wxString::FromUTF8("거래 추가"));
    inputSizer->Add(addButton, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    inputPanel->SetSizer(inputSizer);

    // === 오른쪽 리스트 컨트롤 ===
    listCtrl = new wxListCtrl(mainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);

    // 컬럼 추가
    listCtrl->InsertColumn(0, wxString::FromUTF8("유형"), wxLIST_FORMAT_LEFT, 60);
    listCtrl->InsertColumn(1, wxString::FromUTF8("카테고리"), wxLIST_FORMAT_LEFT, 120);
    listCtrl->InsertColumn(2, wxString::FromUTF8("항목"), wxLIST_FORMAT_LEFT, 120);
    listCtrl->InsertColumn(3, wxString::FromUTF8("금액"), wxLIST_FORMAT_RIGHT, 100);
    listCtrl->InsertColumn(4, wxString::FromUTF8("영수증번호"), wxLIST_FORMAT_LEFT, 100);
    listCtrl->InsertColumn(5, wxString::FromUTF8("날짜/시간"), wxLIST_FORMAT_LEFT, 150);

    // 메인 사이저에 패널 추가
    mainSizer->Add(inputPanel, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(listCtrl, 2, wxEXPAND | wxALL, 5);

    mainPanel->SetSizer(mainSizer);

    // 이벤트 바인딩
    addButton->Bind(wxEVT_BUTTON, &wxMLMainFrame::OnAddTransaction, this);
}

void wxMLMainFrame::AddTransaction(const FMLTransactionData& data)
{
    // Controller를 통해 Model에 추가
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (controller) {
        controller->AddTransaction(data);

        // Observer 패턴으로 자동 UI 업데이트됨 (OnTransactionAdded 호출)

        // 입력 필드 초기화
        ClearInputFields();

        // wxMessageBox(wxString::FromUTF8("거래가 추가되었습니다."), wxString::FromUTF8("성공"), wxOK | wxICON_INFORMATION);
    }
}

void wxMLMainFrame::DisplayTransaction(const FMLTransactionData& data)
{
    // 리스트에 새 항목 추가
    long index = listCtrl->InsertItem(listCtrl->GetItemCount(),
        data.Type == E_MLTransactionType::Income ? wxString::FromUTF8("수입") : wxString::FromUTF8("지출"));
    listCtrl->SetItem(index, 1, wxString::FromUTF8(data.Category.c_str()));
    listCtrl->SetItem(index, 2, wxString::FromUTF8(data.Item.c_str()));
    listCtrl->SetItem(index, 3, wxString::Format("%ld", static_cast<long>(data.Amount)));
    listCtrl->SetItem(index, 4, wxString::FromUTF8(data.ReceiptNumber.c_str()));
    listCtrl->SetItem(index, 5, wxString::FromUTF8(data.DateTime.c_str()));
    listCtrl->SetItemData(index, data.TransactionId);
}

void wxMLMainFrame::DisplayTransactions()
{
}

void wxMLMainFrame::OnAddTransaction(wxCommandEvent& event)
{
    // 입력 데이터 수집
    FMLTransactionData data;
    data.Type = incomeRadio->GetValue() ? E_MLTransactionType::Income : E_MLTransactionType::Expense;
    data.Category = categoryText->GetValue().ToStdString();
    data.Item = itemText->GetValue().ToStdString();
    data.Description = descriptionText->GetValue().ToStdString();
    data.ReceiptNumber = receiptText->GetValue().ToStdString();

    // 금액 파싱
    wxString amountStr = amountText->GetValue();
    double amount = 0.0;
    if (!amountStr.ToDouble(&amount)) {
        wxMessageBox(wxString::FromUTF8("올바른 금액을 입력하세요."), wxString::FromUTF8("입력 오류"), wxOK | wxICON_ERROR);
        return;
    }
    data.Amount = amount;

    // 날짜 가져오기
    wxDateTime selectedDate = datePicker->GetValue();
    data.DateTime = selectedDate.Format("%Y-%m-%d").ToStdString();

    AddTransaction(data);
}

void wxMLMainFrame::RefreshTransactionList()
{
    listCtrl->DeleteAllItems();

    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (controller) {
        auto transactions = controller->GetAllTransactionData();

        for (const auto& trans : transactions) {
            long index = listCtrl->InsertItem(listCtrl->GetItemCount(),
                trans.Type == E_MLTransactionType::Income ? wxString::FromUTF8("수입") : wxString::FromUTF8("지출"));
            listCtrl->SetItem(index, 1, wxString::FromUTF8(trans.Category.c_str()));
            listCtrl->SetItem(index, 2, wxString::FromUTF8(trans.Item.c_str()));
            listCtrl->SetItem(index, 3, wxString::Format("%.2f", trans.Amount));
            listCtrl->SetItem(index, 4, wxString::FromUTF8(trans.ReceiptNumber.c_str()));
            listCtrl->SetItem(index, 5, wxString::FromUTF8(trans.DateTime.c_str()));

            // 행에 거래 ID 저장 (내부적으로 사용)
            listCtrl->SetItemData(index, trans.TransactionId);
        }
    }
}

void wxMLMainFrame::ClearInputFields()
{
    incomeRadio->SetValue(true);
    categoryText->Clear();
    itemText->Clear();
    descriptionText->Clear();
    amountText->Clear();
    receiptText->Clear();
    datePicker->SetValue(wxDateTime::Now());
}

// IMLModelObserver 인터페이스 구현
void wxMLMainFrame::OnTransactionAdded(const FMLTransactionData& transactionData)
{
    DisplayTransaction(transactionData);
}

void wxMLMainFrame::OnTransactionRemoved(int transactionId)
{
    // 리스트에서 해당 항목 찾아서 제거
    for (long i = 0; i < listCtrl->GetItemCount(); i++) {
        if (listCtrl->GetItemData(i) == transactionId) {
            listCtrl->DeleteItem(i);
            break;
        }
    }
}

void wxMLMainFrame::OnTransactionUpdated(const FMLTransactionData& transactionData)
{
    // 리스트에서 해당 항목 찾아서 업데이트
    for (long i = 0; i < listCtrl->GetItemCount(); i++) {
        if (listCtrl->GetItemData(i) == transactionData.TransactionId) {
            listCtrl->SetItem(i, 0, transactionData.Type == E_MLTransactionType::Income ? wxString::FromUTF8("수입") : wxString::FromUTF8("지출"));
            listCtrl->SetItem(i, 1, wxString::FromUTF8(transactionData.Category.c_str()));
            listCtrl->SetItem(i, 2, wxString::FromUTF8(transactionData.Item.c_str()));
            listCtrl->SetItem(i, 3, wxString::Format("%.2f", transactionData.Amount));
            listCtrl->SetItem(i, 4, wxString::FromUTF8(transactionData.ReceiptNumber.c_str()));
            listCtrl->SetItem(i, 5, wxString::FromUTF8(transactionData.DateTime.c_str()));
            break;
        }
    }
}

void wxMLMainFrame::OnTransactionsCleared()
{
    // 모든 항목 제거
    listCtrl->DeleteAllItems();
}

void wxMLMainFrame::OnDataLoaded()
{
    // 데이터 로드 시 전체 리스트 새로고침
    RefreshTransactionList();
}

void wxMLMainFrame::OnDataSaved()
{
    // 저장 완료 메시지 (선택사항)
    // wxMessageBox("데이터가 저장되었습니다.", "저장 완료", wxOK | wxICON_INFORMATION);
}
