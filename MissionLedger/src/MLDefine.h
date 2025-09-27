#pragma once
#include <string>

enum class E_MLTransactionType
{
    Income,    // 수입/입금
    Expense    // 지출/출금
};

struct SMLTransactionData {
    E_MLTransactionType Type;
    std::string Category;
    std::string Item;
    std::string Description;
    double Amount;
    std::string ReceiptNumber = "";
};