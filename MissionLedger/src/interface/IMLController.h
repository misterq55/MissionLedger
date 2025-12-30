#pragma once
#include <string>
#include <vector>

struct FMLTransactionData;

class IMLController
{
public:
    // Transaction operations
    virtual void AddTransaction(const FMLTransactionData& transactionData) = 0;
    virtual bool UpdateTransaction(const FMLTransactionData& transactionData) = 0;
    virtual bool RemoveTransaction(const int transactionId) = 0;

    // Data retrieval for View
    virtual FMLTransactionData GetTransactionData(const int transactionId) = 0;
    virtual std::vector<FMLTransactionData> GetAllTransactionData() = 0;

    // Business logic delegation
    virtual float GetCategoryTotal(const std::string& category) = 0;
    virtual float GetAllTotal() = 0;

    // Persistence
    virtual bool SaveData() = 0;
    virtual bool LoadData() = 0;
};
