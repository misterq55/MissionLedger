#pragma once

struct FMLTransactionData;

class IMLView
{
public:
    virtual ~IMLView() = default;
    virtual void AddTransaction(const FMLTransactionData& data) = 0;
    virtual void DisplayTransaction(const FMLTransactionData& data) = 0;
    virtual void DisplayTransactions() = 0;
};