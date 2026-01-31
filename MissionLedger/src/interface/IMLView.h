#pragma once
#include <vector>

struct FMLTransactionData;
struct FMLTransactionSummary;
struct FMLFilterCriteria;

class IMLView
{
public:
    virtual ~IMLView() = default;
    virtual void AddTransaction(const FMLTransactionData& data) = 0;
    virtual void DisplayTransaction(const FMLTransactionData& data) = 0;
    virtual void DisplayTransactions(const std::vector<FMLTransactionData>& data) = 0;
};