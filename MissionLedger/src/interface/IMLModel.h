#pragma once
#include <map>
#include <string>
#include <vector>
#include <memory>

struct FMLTransactionData;
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

    // Data retrieval - Entity 기반 (내부 로직용, optional)
    virtual std::shared_ptr<FMLTransaction> GetTransaction(const int transactionId) = 0;
    virtual std::map<int, std::shared_ptr<FMLTransaction>> GetAllTransactions() = 0;

    // Business logic
    virtual float GetCategoryTotal(const std::string& category) = 0;
    virtual float GetAllTotal() = 0;
    virtual int GetNextTransactionId() = 0;

    // Persistence
    virtual bool Save() = 0;
    virtual bool Load() = 0;
    virtual void ExportToExcel() = 0;
};
