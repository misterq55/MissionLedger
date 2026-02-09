#pragma once

#include <wx/wx.h>
#include <wx/valtext.h>
#include "MLDefine.h"

/**
 * @brief 예산 입력 다이얼로그
 *
 * 카테고리별 예산을 추가하거나 수정하는 다이얼로그입니다.
 * 추가 모드와 수정 모드를 모두 지원합니다.
 */
class wxMLBudgetDialog : public wxDialog
{
public:
    /**
     * @brief 예산 추가 모드 생성자
     * @param parent 부모 윈도우
     */
    wxMLBudgetDialog(wxWindow* parent);

    /**
     * @brief 예산 수정 모드 생성자
     * @param parent 부모 윈도우
     * @param budgetData 수정할 예산 데이터
     */
    wxMLBudgetDialog(wxWindow* parent, const FMLCategoryBudgetData& budgetData);

    virtual ~wxMLBudgetDialog() = default;

    /**
     * @brief 사용자가 입력한 예산 데이터 가져오기
     * @return 입력된 예산 데이터
     */
    FMLCategoryBudgetData GetBudgetData() const;

private:
    void createControls();
    void bindEvents();
    bool validateInput();

    // 이벤트 핸들러
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

private:
    // UI 컨트롤
    wxTextCtrl* categoryText;
    wxTextCtrl* incomeAmountText;
    wxTextCtrl* expenseAmountText;

    // 모드
    bool IsEditMode;
    std::string OriginalCategory;  // 수정 모드에서 원래 카테고리 이름
};
