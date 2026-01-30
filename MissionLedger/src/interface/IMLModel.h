#pragma once
#include <map>
#include <string>
#include <vector>
#include <memory>

struct FMLTransactionData;
struct FMLFilterCriteria;
struct FMLTransactionSummary;
class FMLTransaction;
class IMLModelObserver;

class IMLModel
{
public:
    virtual ~IMLModel() = default;
    
    // Observer
    virtual void AddObserver(std::shared_ptr<IMLModelObserver> modelObserver) = 0;
    
    // Transaction CRUD operations
    virtual void AddTransaction(const FMLTransactionData& transactionData) = 0;
    virtual bool UpdateTransaction(const FMLTransactionData& transactionData) = 0;
    virtual bool RemoveTransaction(const int transactionId) = 0;

    // Data retrieval - DTO 기반 (View/Controller용)
    virtual FMLTransactionData GetTransactionData(const int transactionId) = 0;
    virtual std::vector<FMLTransactionData> GetAllTransactionData() = 0;
    virtual std::vector<FMLTransactionData> GetFilteredTransactionData(const FMLFilterCriteria& criteria) = 0;

    // Data retrieval - Entity 기반 (내부 로직용, optional)
    virtual std::shared_ptr<FMLTransaction> GetTransaction(const int transactionId) = 0;
    virtual std::map<int, std::shared_ptr<FMLTransaction>> GetAllTransactions() = 0;

    // Business logic
    virtual FMLTransactionSummary CalculateTransactionSummary() = 0;
    virtual FMLTransactionSummary CalculateFilteredTransactionSummary(const FMLFilterCriteria& criteria) = 0;

    // Persistence
    virtual bool Save() = 0;
    virtual bool Load() = 0;
    virtual void ExportToExcel() = 0;

    // File Operations
    virtual bool OpenFile(const std::string& filePath) = 0;
    virtual bool SaveFile() = 0;
    virtual bool SaveFileAs(const std::string& filePath) = 0;
    virtual void NewFile() = 0;
    virtual const std::string& GetCurrentFilePath() const = 0;
    virtual bool HasUnsavedChanges() const = 0;
};
