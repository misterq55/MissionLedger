#pragma once
#include <string>
#include <cstdint>
#include <map>

enum class E_MLTransactionType
{
    Income,    // 수입/입금
    Expense,    // 지출/출금
    _Max
};

struct FMLTransactionData {
    int TransactionId = -1;
    E_MLTransactionType Type;
    std::string Category;
    std::string Item;
    std::string Description;
    int64_t Amount;
    std::string ReceiptNumber;
    std::string DateTime;  // 입력 시 빈 문자열, 출력 시 포맷된 날짜/시간 문자열

    // 환율 관련 필드
    bool UseExchangeRate = false;       // 환율 적용 여부
    std::string Currency = "KRW";       // 통화 코드 (기본값: KRW)
    double OriginalAmount = 0.0;        // 원래 외화 금액
    double ExchangeRate = 1.0;          // 환율 (기본값: 1.0)

    // 예산 연결 필드
    int BudgetId = -1;                  // 연결된 예산 ID (-1이면 자동 매칭 또는 미연결)
};

struct FMLFilterCriteria {
    bool UseTypeFilter = false;
    E_MLTransactionType TypeFilter = E_MLTransactionType::_Max;

    bool UseDateRangeFilter = false;
    std::string StartDate;  // YYYY-MM-DD 형식
    std::string EndDate;    // YYYY-MM-DD 형식

    bool UseCategoryFilter = false;
    std::string CategoryFilter;

    bool UseAmountRangeFilter = false;
    int64_t MinAmount = 0;
    int64_t MaxAmount = 0;

    bool UseTextSearch = false;
    std::string SearchText;
    bool SearchInItem = false;
    bool SearchInDescription = false;
    bool SearchInReceipt = false;
};

struct FMLTransactionSummary {
    int64_t TotalIncome = 0;
    int64_t TotalExpense = 0;
    int64_t Balance = 0;
    int TransactionCount = 0;
};

// ========================================
// 예산 관리
// ========================================

// 예산 데이터 (Budget Data)
// 예: Category="항공", Item="항공료 선결제", BudgetAmount=3,000,000, ActualAmount=2,800,000
struct FMLBudgetData {
    int BudgetId = -1;
    E_MLTransactionType Type;      // Income/Expense
    std::string Category;          // 카테고리 (필수, 예: "항공", "생활", "회비")
    std::string Item;              // 항목 (선택, 예: "항공료 선결제", "숙박비", 비어있을 수 있음)
    int64_t BudgetAmount = 0;      // 예산 금액 (수동 입력)
    int64_t ActualAmount = 0;      // 실제 금액 (자동 계산, READ-ONLY, 연결된 Transaction들의 합계)
    std::string Notes;             // 비고
};

// 전체 예산 요약 (Overall Budget Summary - Hierarchical Structure)
struct FMLBudgetSummary {
    // 전체 합계
    int64_t TotalBudget = 0;          // 모든 항목 예산 합계
};

// 결산안 데이터
struct FMLSettlmentData {
    // 제목
    std::string Title = "결산 보고서";

    // 예산 데이터 - 카테고리별 합계 (하위 호환용)
    std::map<std::string, int64_t> BudgetIncomeCategories;
    std::map<std::string, int64_t> BudgetExpenseCategories;
    int64_t TotalIncome = 0;
    int64_t TotalExpense = 0;

    // 예산 데이터 - 중분류 (카테고리 → 항목 → 금액)
    std::map<std::string, std::map<std::string, int64_t>> BudgetIncomeItems;
    std::map<std::string, std::map<std::string, int64_t>> BudgetExpenseItems;

    // 실적 데이터 - 카테고리별 합계 (하위 호환용)
    std::map<std::string, int64_t> ActualIncomeCategories;
    std::map<std::string, int64_t> ActualExpenseCategories;
    int64_t TotalActualIncome = 0;
    int64_t TotalActualExpense = 0;

    // 실적 데이터 - 중분류 (카테고리 → 항목 → 금액)
    std::map<std::string, std::map<std::string, int64_t>> ActualIncomeItems;
    std::map<std::string, std::map<std::string, int64_t>> ActualExpenseItems;

    // 기타
    int64_t TotalBalance = 0;
    std::map<std::string, double> ExchangeRates;
};
