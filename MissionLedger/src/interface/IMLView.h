#pragma once
#include <vector>

struct FMLTransactionData;
struct FMLTransactionSummary;
struct FMLFilterCriteria;
struct FMLItemBudgetData;

class IMLView
{
public:
    virtual ~IMLView() = default;

    // Transactions
    virtual void AddTransaction(const FMLTransactionData& data) = 0;
    virtual void DisplayTransaction(const FMLTransactionData& data) = 0;
    virtual void DisplayTransactions(const std::vector<FMLTransactionData>& data) = 0;

    // Budget
    // virtual void AddBudget(const FMLItemBudgetData& data) = 0;
    // virtual void DisplayBudget(const FMLItemBudgetData& data) = 0;
    // virtual void DisplayBudgets(const std::vector<FMLItemBudgetData>& data) = 0;
};