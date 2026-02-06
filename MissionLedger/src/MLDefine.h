#pragma once
#include <string>
#include <cstdint>

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