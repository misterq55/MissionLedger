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
// 예산 관리 (항목별 예산)
// ========================================

// 항목별 예산 데이터 (Item Budget Data)
// 예: Category="항공", Item="항공료 선결제", BudgetAmount=3,000,000
struct FMLItemBudgetData {
    std::string Category;        // 카테고리 (예: "항공", "생활")
    std::string Item;            // 항목 (예: "항공료 선결제", "숙박비")
    int64_t BudgetAmount = 0;    // 예산 금액 (가용 금액)
};

// 항목 1개의 예산 vs 실제 요약 (Item Budget Summary)
struct FMLItemBudgetSummary {
    std::string Item;              // 항목명
    int64_t BudgetAmount = 0;      // 예산 금액
    int64_t ActualExpense = 0;     // 실제 지출 (Type=Expense인 Transaction 합계)
    int64_t ActualIncome = 0;      // 실제 수입 (Type=Income인 Transaction 합계)
    int64_t NetAmount = 0;         // 순액 (수입 - 지출)
    int64_t Variance = 0;          // 차이 (실제 - 예산, 지출 기준)
    int TransactionCount = 0;      // 실제 거래 건수
    double Ratio = 0.0;            // 비율 (실제 / 예산 * 100)
};

// 카테고리 1개의 예산 vs 실제 요약 (Category Budget Summary)
struct FMLCategoryBudgetSummary {
    std::string Category;                                  // 카테고리명
    std::map<std::string, FMLItemBudgetSummary> Items;     // 항목별 상세 (Item → Summary)

    // 카테고리 합계
    int64_t TotalBudget = 0;          // 모든 항목 예산 합계
    int64_t TotalActualExpense = 0;   // 실제 총 지출
    int64_t TotalActualIncome = 0;    // 실제 총 수입
    int64_t TotalNetAmount = 0;       // 순액 (수입 - 지출)
    int64_t TotalVariance = 0;        // 차이 (실제 - 예산)
    int TransactionCount = 0;         // 거래 건수
    double Ratio = 0.0;               // 비율
};

// 전체 예산 요약 (Overall Budget Summary - Hierarchical Structure)
struct FMLBudgetSummary {
    // 카테고리별 상세 데이터 (Category → Category Summary)
    std::map<std::string, FMLCategoryBudgetSummary> Categories;

    // 전체 합계
    int64_t TotalBudget = 0;          // 모든 항목 예산 합계
    int64_t TotalActualExpense = 0;   // 실제 총 지출
    int64_t TotalActualIncome = 0;    // 실제 총 수입
    int64_t TotalNetAmount = 0;       // 순액
    int64_t TotalVariance = 0;        // 차이
    int CategoryCount = 0;            // 카테고리 수
    int ItemCount = 0;                // 항목 수
    double Ratio = 0.0;               // 비율
};