#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/statline.h>
#include "module/common/holder/MLMVCHolder.h"
#include "module/mvc/model/MLModel.h"
#include "module/mvc/controller/MLController.h"
#include "MLDefine.h"
#include "module/mvc/view/MLGuiView.h"

class wxMLMainFrame : public wxFrame {
public:
  wxMLMainFrame() : wxFrame(nullptr, wxID_ANY, "MissionLedger - 거래 관리", wxDefaultPosition, wxSize(900, 600)) {
    // 메인 패널
    wxPanel* mainPanel = new wxPanel(this);

    // 수평 박스 사이저 (왼쪽 입력 패널 + 오른쪽 리스트)
    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

    // === 왼쪽 입력 패널 ===
    wxPanel* inputPanel = new wxPanel(mainPanel);
    inputPanel->SetBackgroundColour(wxColour(240, 240, 240));
    wxBoxSizer* inputSizer = new wxBoxSizer(wxVERTICAL);

    // 제목
    wxStaticText* titleText = new wxStaticText(inputPanel, wxID_ANY, "거래 입력");
    wxFont titleFont = titleText->GetFont();
    titleFont.SetPointSize(12);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleText->SetFont(titleFont);
    inputSizer->Add(titleText, 0, wxALL | wxALIGN_CENTER, 10);

    // 구분선
    inputSizer->Add(new wxStaticLine(inputPanel), 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    inputSizer->AddSpacer(10);

    // 거래 유형 (라디오 버튼)
    wxStaticText* typeLabel = new wxStaticText(inputPanel, wxID_ANY, "거래 유형:");
    inputSizer->Add(typeLabel, 0, wxLEFT | wxRIGHT, 10);

    incomeRadio = new wxRadioButton(inputPanel, wxID_ANY, "수입", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    expenseRadio = new wxRadioButton(inputPanel, wxID_ANY, "지출");
    incomeRadio->SetValue(true);
    inputSizer->Add(incomeRadio, 0, wxLEFT, 20);
    inputSizer->Add(expenseRadio, 0, wxLEFT, 20);
    inputSizer->AddSpacer(10);

    // 카테고리
    wxStaticText* categoryLabel = new wxStaticText(inputPanel, wxID_ANY, "카테고리:");
    inputSizer->Add(categoryLabel, 0, wxLEFT | wxRIGHT, 10);
    categoryText = new wxTextCtrl(inputPanel, wxID_ANY);
    inputSizer->Add(categoryText, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    inputSizer->AddSpacer(5);

    // 항목
    wxStaticText* itemLabel = new wxStaticText(inputPanel, wxID_ANY, "항목:");
    inputSizer->Add(itemLabel, 0, wxLEFT | wxRIGHT, 10);
    itemText = new wxTextCtrl(inputPanel, wxID_ANY);
    inputSizer->Add(itemText, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    inputSizer->AddSpacer(5);

    // 설명
    wxStaticText* descLabel = new wxStaticText(inputPanel, wxID_ANY, "설명:");
    inputSizer->Add(descLabel, 0, wxLEFT | wxRIGHT, 10);
    descriptionText = new wxTextCtrl(inputPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    inputSizer->Add(descriptionText, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);
    inputSizer->AddSpacer(5);

    // 금액
    wxStaticText* amountLabel = new wxStaticText(inputPanel, wxID_ANY, "금액:");
    inputSizer->Add(amountLabel, 0, wxLEFT | wxRIGHT, 10);
    amountText = new wxTextCtrl(inputPanel, wxID_ANY);
    inputSizer->Add(amountText, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    inputSizer->AddSpacer(5);

    // 영수증 번호
    wxStaticText* receiptLabel = new wxStaticText(inputPanel, wxID_ANY, "영수증 번호:");
    inputSizer->Add(receiptLabel, 0, wxLEFT | wxRIGHT, 10);
    receiptText = new wxTextCtrl(inputPanel, wxID_ANY);
    inputSizer->Add(receiptText, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    inputSizer->AddSpacer(15);

    // 추가 버튼
    wxButton* addButton = new wxButton(inputPanel, wxID_ANY, "거래 추가");
    inputSizer->Add(addButton, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    inputPanel->SetSizer(inputSizer);

    // === 오른쪽 리스트 컨트롤 ===
    listCtrl = new wxListCtrl(mainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);

    // 컬럼 추가
    listCtrl->InsertColumn(0, "ID", wxLIST_FORMAT_LEFT, 50);
    listCtrl->InsertColumn(1, "유형", wxLIST_FORMAT_LEFT, 60);
    listCtrl->InsertColumn(2, "카테고리", wxLIST_FORMAT_LEFT, 100);
    listCtrl->InsertColumn(3, "항목", wxLIST_FORMAT_LEFT, 100);
    listCtrl->InsertColumn(4, "금액", wxLIST_FORMAT_RIGHT, 100);
    listCtrl->InsertColumn(5, "영수증번호", wxLIST_FORMAT_LEFT, 100);
    listCtrl->InsertColumn(6, "날짜/시간", wxLIST_FORMAT_LEFT, 150);

    // 메인 사이저에 패널 추가
    mainSizer->Add(inputPanel, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(listCtrl, 2, wxEXPAND | wxALL, 5);

    mainPanel->SetSizer(mainSizer);

    // 이벤트 바인딩
    addButton->Bind(wxEVT_BUTTON, &wxMLMainFrame::OnAddTransaction, this);
  }

private:
  // UI 컨트롤들
  wxRadioButton* incomeRadio;
  wxRadioButton* expenseRadio;
  wxTextCtrl* categoryText;
  wxTextCtrl* itemText;
  wxTextCtrl* descriptionText;
  wxTextCtrl* amountText;
  wxTextCtrl* receiptText;
  wxListCtrl* listCtrl;

  // 거래 추가 이벤트 핸들러
  void OnAddTransaction(wxCommandEvent& event) {
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
      wxMessageBox("올바른 금액을 입력하세요.", "입력 오류", wxOK | wxICON_ERROR);
      return;
    }
    data.Amount = amount;

    // Controller를 통해 Model에 추가
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (controller) {
      controller->AddTransaction(data);

      // UI 업데이트 (임시 - 나중에 Observer 패턴으로 변경)
      RefreshTransactionList();

      // 입력 필드 초기화
      ClearInputFields();

      wxMessageBox("거래가 추가되었습니다.", "성공", wxOK | wxICON_INFORMATION);
    }
  }

  // 거래 목록 새로고침
  void RefreshTransactionList() {
    listCtrl->DeleteAllItems();

    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model) {
      auto transactions = model->GetAllTransactionData();

      for (const auto& trans : transactions) {
        long index = listCtrl->InsertItem(listCtrl->GetItemCount(), wxString::Format("%d", trans.TransactionId));
        listCtrl->SetItem(index, 1, trans.Type == E_MLTransactionType::Income ? "수입" : "지출");
        listCtrl->SetItem(index, 2, trans.Category);
        listCtrl->SetItem(index, 3, trans.Item);
        listCtrl->SetItem(index, 4, wxString::Format("%.2f", trans.Amount));
        listCtrl->SetItem(index, 5, trans.ReceiptNumber);
        listCtrl->SetItem(index, 6, trans.DateTime);

        // 행에 거래 ID 저장 (나중에 사용)
        listCtrl->SetItemData(index, trans.TransactionId);
      }
    }
  }

  // 입력 필드 초기화
  void ClearInputFields() {
    incomeRadio->SetValue(true);
    categoryText->Clear();
    itemText->Clear();
    descriptionText->Clear();
    amountText->Clear();
    receiptText->Clear();
  }
};

class MyApp : public wxApp {
public:
  virtual bool OnInit() {
    auto& mvcHolder = FMLMVCHolder::GetInstance();
    std::shared_ptr<FMLModel> model = std::make_shared<FMLModel>();
    std::shared_ptr<FMLController> controller = std::make_shared<FMLController>();
    std::shared_ptr<FMLGuiView> view = std::make_shared<FMLGuiView>();
    
    mvcHolder.SetModel(model);
    mvcHolder.SetController(controller);
    mvcHolder.SetView(view);
    
    model->AddObserver(view);
    
    wxMLMainFrame* frame = new wxMLMainFrame();
    frame->Show();
    return true;
  }
};

wxIMPLEMENT_APP(MyApp);
