#pragma once

struct SMLTransactionData;

class IMLController
{
public:
    virtual void AddTransaction(const SMLTransactionData& transactionData) = 0;
    virtual bool RemoveTransaction(const int transactionId) = 0;
};
