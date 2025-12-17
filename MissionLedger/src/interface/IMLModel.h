#pragma once

struct FMLTransactionData;

class IMLModel
{
public:
    virtual void AddTransaction(const FMLTransactionData& transactionData) = 0;
    virtual bool RemoveTransaction(const int transactionId) = 0;
    virtual int GetTransactionId() = 0;
    virtual bool Save() = 0;
    virtual bool Load() = 0;
};
