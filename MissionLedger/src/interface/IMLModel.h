#pragma once

struct SMLTransactionData;

class IMLModel
{
public:
    virtual void AddTransaction(const SMLTransactionData& transactionData) = 0;
    virtual bool RemoveTransaction(const int transactionId) = 0;
    virtual bool Save() = 0;
    virtual bool Load() = 0;
};
