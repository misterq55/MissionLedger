#pragma once
#include <string>

struct FMLTransactionData;
class FMLTransaction;

class IMLModel
{
public:
    virtual void AddTransaction(const FMLTransactionData& transactionData) = 0;
    virtual bool RemoveTransaction(const int transactionId) = 0;
    virtual int GetTransactionId() = 0;
    virtual std::shared_ptr<FMLTransaction> GetTransaction(const int transactionId) = 0;
    virtual std::map<int, std::shared_ptr<FMLTransaction>> GetAllTransactions() = 0;
    virtual float GetCategoryTotal(std::string category) = 0;
    virtual float GetAllTotal() = 0;
    virtual bool Save() = 0;
    virtual bool Load() = 0;
    virtual void ExportToExcel() = 0;
};
