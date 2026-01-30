#pragma once

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/datectrl.h>
#include <wx/valtext.h>
#include <set>
#include <map>
#include "interface/IMLView.h"
#include "interface/IMLModelObserver.h"

/**
 * @brief MissionLedger 메인 윈도우 프레임
 *
 * wxWidgets의 wxFrame을 상속하며, IMLView와 IMLModelObserver를 직접 구현하여
 * MVC 패턴의 View 역할과 Model의 Observer 역할을 동시에 수행합니다.
 */
class wxMLMainFrame : public wxFrame, public IMLView, public IMLModelObserver
{
public:
    wxMLMainFrame();
    virtual ~wxMLMainFrame() = default;

public:
    // IMLView 인터페이스 구현
    void AddTransaction(const FMLTransactionData& data) override;
    void DisplayTransaction(const FMLTransactionData& data) override;
    void DisplayTransactions() override;
    
    // IMLModelObserver 인터페이스 구현
    void OnTransactionAdded(const FMLTransactionData& transactionData) override;
    void OnTransactionRemoved(int transactionId) override;
    void OnTransactionUpdated(const FMLTransactionData& transactionData) override;
    void OnTransactionsCleared() override;
    void OnDataLoaded() override;
    void OnDataSaved() override;

private:
    // 이벤트 핸들러 - 거래
    void OnAddTransaction(wxCommandEvent& event);
    void OnUpdateTransaction(wxCommandEvent& event);
    void OnDeleteTransaction(wxCommandEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnListItemDeselected(wxListEvent& event);

    // 이벤트 핸들러 - 파일 메뉴
    void OnNewFile(wxCommandEvent& event);
    void OnOpenFile(wxCommandEvent& event);
    void OnSaveFile(wxCommandEvent& event);
    void OnSaveFileAs(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    // 이벤트 핸들러 - 필터
    void OnApplyFilter(wxCommandEvent& event);
    void OnClearFilter(wxCommandEvent& event);

    // UI 헬퍼 메서드
    void refreshTransactionList();
    void clearInputFields();
    void loadTransactionToInput(int transactionId);
    void updateButtonStates();
    void updateTitle();
    bool checkUnsavedChanges();

    // 필터 헬퍼 메서드
    void createFilterPanel(wxPanel* parent, wxBoxSizer* sizer);
    void updateCategoryFilter();
    void applyCurrentFilter();

    // 리스트 증분 업데이트 헬퍼
    std::set<int> getCurrentListIds();
    long findListItemByTransactionId(int transactionId);
    void removeListItemByTransactionId(int transactionId);

private:
    void createMenuBar();

private:
    // UI 컨트롤들 - 입력 패널
    wxRadioButton* incomeRadio;
    wxRadioButton* expenseRadio;
    wxTextCtrl* categoryText;
    wxTextCtrl* itemText;
    wxTextCtrl* descriptionText;
    wxTextCtrl* amountText;
    wxTextCtrl* receiptText;
    wxDatePickerCtrl* datePicker;
    wxListCtrl* listCtrl;
    wxButton* addButton;
    wxButton* updateButton;
    wxButton* deleteButton;

    // UI 컨트롤들 - 필터 패널
    wxTextCtrl* filterSearchText;
    wxCheckBox* filterSearchInItem;
    wxCheckBox* filterSearchInDescription;
    wxCheckBox* filterSearchInReceipt;
    wxDatePickerCtrl* filterStartDate;
    wxDatePickerCtrl* filterEndDate;
    wxChoice* filterTypeChoice;
    wxComboBox* filterCategoryCombo;
    wxButton* applyFilterButton;
    wxButton* clearFilterButton;
    
private:
    int SelectedTransactionId = -1;
    bool FilterActive = false;
};
