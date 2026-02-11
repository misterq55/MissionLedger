#pragma once
#include <map>
#include <string>
#include <vector>
#include <memory>

struct FMLTransactionData;
struct FMLFilterCriteria;
struct FMLTransactionSummary;
struct FMLBudgetData;
struct FMLBudgetSummary;
class FMLTransaction;
class IMLModelObserver;

class IMLModel
{
public:
    virtual ~IMLModel() = default;
    
    // Observer
    virtual void AddObserver(std::shared_ptr<IMLModelObserver> modelObserver) = 0;
    
    // Transaction CRUD operations
    virtual bool AddTransaction(const FMLTransactionData& transactionData) = 0;
    virtual bool UpdateTransaction(const FMLTransactionData& transactionData) = 0;
    virtual bool DeleteTransaction(const int transactionId) = 0;

    // Data retrieval - DTO 기반 (View/Controller용)
    virtual FMLTransactionData GetTransactionData(const int transactionId) const = 0;
    virtual std::vector<FMLTransactionData> GetAllTransactionData() const = 0;
    virtual std::vector<FMLTransactionData> GetFilteredTransactionData(const FMLFilterCriteria& criteria) const = 0;

    // Budget CRUD operations (항목별 예산)
    virtual bool AddBudget(const FMLBudgetData& budgetData) = 0;
    virtual bool UpdateBudget(const FMLBudgetData& budgetData) = 0;
    virtual bool DeleteBudget(const int budgetId) = 0;
    virtual std::vector<FMLBudgetData> GetAllBudgets() const = 0;
    virtual FMLBudgetData GetBudget(const int budgetId) const = 0;

    // Budget Summary (계층 구조: 카테고리 > 항목)
    virtual FMLBudgetSummary GetBudgetSummary() const = 0;
    virtual FMLBudgetSummary GetFilteredBudgetSummary(const FMLFilterCriteria& criteria) const = 0;
    
    // Business logic
    virtual FMLTransactionSummary GetTransactionSummary() const = 0;
    virtual FMLTransactionSummary GetFilteredTransactionSummary(const FMLFilterCriteria& criteria) const = 0;
    virtual std::vector<std::string> GetAllCategories() const = 0;

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
