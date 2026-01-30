#pragma once
#include <string>
#include <cstdint>

enum class E_MLTransactionType
{
    Income,    // 수입/입금
    Expense    // 지출/출금
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
};

struct FMLFilterCriteria {
    bool UseTypeFilter = false;
    E_MLTransactionType TypeFilter;

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