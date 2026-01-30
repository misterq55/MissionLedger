#pragma once
#include <map>
#include <memory>
#include <string>

#include "interface/IMLModel.h"

class FMLTransaction;
class IMLStorageProvider;

class FMLModel : public IMLModel
{
public:
    FMLModel();
    virtual ~FMLModel() override;

public:
    // Observer
    virtual void AddObserver(std::shared_ptr<IMLModelObserver> modelObserver) override;

    // Transaction CRUD operations
    virtual void AddTransaction(const FMLTransactionData& transactionData) override;
    virtual bool UpdateTransaction(const FMLTransactionData& transactionData) override;
    virtual bool RemoveTransaction(const int transactionId) override;

    // Data retrieval - DTO 기반
    virtual FMLTransactionData GetTransactionData(const int transactionId) override;
    virtual std::vector<FMLTransactionData> GetAllTransactionData() override;
    virtual std::vector<FMLTransactionData> GetFilteredTransactionData(const FMLFilterCriteria& criteria) override;

    // Data retrieval - Entity 기반
    virtual std::shared_ptr<FMLTransaction> GetTransaction(const int transactionId) override;
    virtual std::map<int, std::shared_ptr<FMLTransaction>> GetAllTransactions() override;

    // Business logic
    virtual float GetCategoryTotal(const std::string& category) override;
    virtual float GetAllTotal() override;
    virtual int GetNextTransactionId() override;

    virtual FMLTransactionSummary CalculateTransactionSummary() override;
    virtual FMLTransactionSummary CalculateFilteredTransactionSummary(const FMLFilterCriteria& criteria) override;

    // Persistence
    virtual bool Save() override;
    virtual bool Load() override;
    virtual void ExportToExcel() override;

    // File Operations
    virtual bool OpenFile(const std::string& filePath) override;
    virtual bool SaveFile() override;
    virtual bool SaveFileAs(const std::string& filePath) override;
    virtual void NewFile() override;
    virtual const std::string& GetCurrentFilePath() const override;
    virtual bool HasUnsavedChanges() const override;

    // Storage Provider (구현 전용)
    void SetStorageProvider(std::shared_ptr<IMLStorageProvider> storageProvider);

private:
    FMLTransactionData convertToTransactionData(const std::shared_ptr<FMLTransaction>& transaction);
    void clearAllTransactions();
    FMLTransactionSummary calculateTransactionSummary(const std::vector<FMLTransactionData>& transactionData);

private:
    std::shared_ptr<IMLModelObserver> ModelObserver;
    std::shared_ptr<IMLStorageProvider> StorageProvider;
    std::map<int, std::shared_ptr<FMLTransaction>> Transactions;
    std::string CurrentFilePath;
    int TransactionIdIndex = 0;
    bool UnsavedChanges = false;
};
