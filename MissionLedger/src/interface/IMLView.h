#pragma once
#include <vector>

struct FMLTransactionData;
struct FMLTransactionSummary;
struct FMLFilterCriteria;
struct FMLCategoryBudgetData;

class IMLView
{
public:
    virtual ~IMLView() = default;

    // Transactions
    virtual void AddTransaction(const FMLTransactionData& data) = 0;
    virtual void DisplayTransaction(const FMLTransactionData& data) = 0;
    virtual void DisplayTransactions(const std::vector<FMLTransactionData>& data) = 0;

    // Budgets - 현재 View는 Controller를 호출하는 방식이므로 필요 없음
    // TODO: 향후 필요 시 활성화
    // virtual void AddBudget(const FMLCategoryBudgetData& data) = 0;
    // virtual void DisplayBudget(const FMLCategoryBudgetData& data) = 0;
    // virtual void DisplayBudgets(const std::vector<FMLCategoryBudgetData>& data) = 0;
};