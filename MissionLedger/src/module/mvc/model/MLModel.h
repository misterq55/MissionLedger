#pragma once
#include <map>
#include <memory>

#include "interface/IMLModel.h"

class FMLTransaction;
class FMLModel : public IMLModel
{
public:
    FMLModel();
    virtual ~FMLModel();

public:
    // Transaction CRUD operations
    virtual void AddTransaction(const FMLTransactionData& transactionData) override;
    virtual bool UpdateTransaction(const FMLTransactionData& transactionData) override;
    virtual bool RemoveTransaction(const int transactionId) override;

    // Data retrieval - DTO 기반
    virtual FMLTransactionData GetTransactionData(const int transactionId) override;
    virtual std::vector<FMLTransactionData> GetAllTransactionData() override;

    // Data retrieval - Entity 기반
    virtual std::shared_ptr<FMLTransaction> GetTransaction(const int transactionId) override;
    virtual std::map<int, std::shared_ptr<FMLTransaction>> GetAllTransactions() override;

    // Business logic
    virtual float GetCategoryTotal(const std::string& category) override;
    virtual float GetAllTotal() override;
    virtual int GetNextTransactionId() override;

    // Persistence
    virtual bool Save() override;
    virtual bool Load() override;
    virtual void ExportToExcel() override;

private:
    FMLTransactionData convertToTransactionData(const std::shared_ptr<FMLTransaction>& transaction);

private:
    std::map<int, std::shared_ptr<FMLTransaction>> Transactions;
    int TransactionIdIndex = 0;
};
