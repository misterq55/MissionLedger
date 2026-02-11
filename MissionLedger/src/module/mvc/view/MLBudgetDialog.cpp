#include "MLBudgetDialog.h"
#include <wx/statline.h>

// 예산 추가 모드 생성자
wxMLBudgetDialog::wxMLBudgetDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, wxString::FromUTF8("예산 추가"), wxDefaultPosition, wxSize(400, 300))
    , IsEditMode(false)
{
    createControls();
    bindEvents();
}

// 예산 수정 모드 생성자
wxMLBudgetDialog::wxMLBudgetDialog(wxWindow* parent, const FMLBudgetData& budgetData)
    : wxDialog(parent, wxID_ANY, wxString::FromUTF8("예산 수정"), wxDefaultPosition, wxSize(400, 300))
    , IsEditMode(true)
    , OriginalCategory(budgetData.Category)
{
    createControls();
    bindEvents();

    // 기존 데이터 로드
    categoryText->SetValue(wxString::FromUTF8(budgetData.Category.c_str()));
    incomeAmountText->SetValue(wxString::Format("%lld", budgetData.BudgetAmount));

    // 수정 모드에서는 카테고리 변경 불가
    categoryText->Enable(false);
}

void wxMLBudgetDialog::createControls()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // 카테고리 입력
    wxStaticText* categoryLabel = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("카테고리:"));
    mainSizer->Add(categoryLabel, 0, wxALL, 10);

    categoryText = new wxTextCtrl(this, wxID_ANY);
    mainSizer->Add(categoryText, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // 수입 예산 입력
    wxStaticText* incomeLabel = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("수입 예산:"));
    mainSizer->Add(incomeLabel, 0, wxLEFT | wxRIGHT, 10);

    incomeAmountText = new wxTextCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_DIGITS));
    mainSizer->Add(incomeAmountText, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // 지출 예산 입력
    wxStaticText* expenseLabel = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("지출 예산:"));
    mainSizer->Add(expenseLabel, 0, wxLEFT | wxRIGHT, 10);

    expenseAmountText = new wxTextCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_DIGITS));
    mainSizer->Add(expenseAmountText, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // 구분선
    mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    mainSizer->AddSpacer(10);

    // 버튼 (확인, 취소)
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->AddStretchSpacer();

    wxButton* okButton = new wxButton(this, wxID_OK, wxString::FromUTF8("확인"));
    wxButton* cancelButton = new wxButton(this, wxID_CANCEL, wxString::FromUTF8("취소"));

    buttonSizer->Add(okButton, 0, wxRIGHT, 5);
    buttonSizer->Add(cancelButton, 0, 0, 0);

    mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 10);

    SetSizer(mainSizer);
}

void wxMLBudgetDialog::bindEvents()
{
    Bind(wxEVT_BUTTON, &wxMLBudgetDialog::OnOK, this, wxID_OK);
    Bind(wxEVT_BUTTON, &wxMLBudgetDialog::OnCancel, this, wxID_CANCEL);
}

void wxMLBudgetDialog::OnOK(wxCommandEvent& event)
{
    if (!validateInput())
    {
        return;  // 검증 실패 시 다이얼로그 유지
    }

    EndModal(wxID_OK);
}

void wxMLBudgetDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

bool wxMLBudgetDialog::validateInput()
{
    // 카테고리 검증
    wxString category = categoryText->GetValue().Trim();
    if (category.IsEmpty())
    {
        wxMessageBox(wxString::FromUTF8("카테고리를 입력하세요."),
                     wxString::FromUTF8("입력 오류"), wxOK | wxICON_ERROR);
        categoryText->SetFocus();
        return false;
    }

    // 금액 검증 (수입)
    long long incomeAmount = 0;
    wxString incomeStr = incomeAmountText->GetValue();
    if (!incomeStr.ToLongLong(&incomeAmount) || incomeAmount < 0)
    {
        wxMessageBox(wxString::FromUTF8("올바른 수입 예산을 입력하세요."),
                     wxString::FromUTF8("입력 오류"), wxOK | wxICON_ERROR);
        incomeAmountText->SetFocus();
        return false;
    }

    // 금액 검증 (지출)
    long long expenseAmount = 0;
    wxString expenseStr = expenseAmountText->GetValue();
    if (!expenseStr.ToLongLong(&expenseAmount) || expenseAmount < 0)
    {
        wxMessageBox(wxString::FromUTF8("올바른 지출 예산을 입력하세요."),
                     wxString::FromUTF8("입력 오류"), wxOK | wxICON_ERROR);
        expenseAmountText->SetFocus();
        return false;
    }

    return true;
}

FMLBudgetData wxMLBudgetDialog::GetBudgetData() const
{
    FMLBudgetData data;
    data.Category = categoryText->GetValue().Trim().ToUTF8().data();

    long long incomeAmount = 0;
    incomeAmountText->GetValue().ToLongLong(&incomeAmount);
    data.BudgetAmount = incomeAmount;
    
    return data;
}
