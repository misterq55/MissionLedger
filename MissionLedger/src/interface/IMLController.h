#pragma once

struct FMLTransactionData;

class IMLController
{
public:
    virtual void AddTransaction(const FMLTransactionData& transactionData) = 0;
    virtual bool RemoveTransaction(const int transactionId) = 0;
};
