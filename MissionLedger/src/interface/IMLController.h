#pragma once
#include <string>
#include <vector>

struct FMLTransactionData;
struct FMLFilterCriteria;
struct FMLTransactionSummary;
struct FMLCategoryBudgetData;
struct FMLBudgetSummary;

class IMLController
{
public:
    virtual ~IMLController() = default;

    // Transaction operations
    virtual void AddTransaction(const FMLTransactionData& transactionData) = 0;
    virtual bool UpdateTransaction(const FMLTransactionData& transactionData) = 0;
    virtual bool RemoveTransaction(const int transactionId) = 0;

    // Data retrieval for View
    virtual FMLTransactionData GetTransactionData(const int transactionId) = 0;
    virtual std::vector<FMLTransactionData> GetAllTransactionData() = 0;
    virtual std::vector<FMLTransactionData> GetFilteredTransactionData(const FMLFilterCriteria& criteria) = 0;
    virtual std::vector<std::string> GetAllCategories() const = 0;

    // Transaction Summary
    virtual FMLTransactionSummary GetTransactionSummary() = 0;
    virtual FMLTransactionSummary GetFilteredTransactionSummary(const FMLFilterCriteria& criteria) = 0;

    // Persistence
    virtual bool SaveData() = 0;
    virtual bool LoadData() = 0;

    // File operations
    virtual void NewFile() = 0;
    virtual bool OpenFile(const std::string& filePath) = 0;
    virtual bool SaveFile() = 0;
    virtual bool SaveFileAs(const std::string& filePath) = 0;
    virtual std::string GetCurrentFilePath() const = 0;
    virtual bool HasUnsavedChanges() const = 0;

    // Budget operations
    virtual bool AddBudget(const FMLCategoryBudgetData& budgetData) = 0;
    virtual bool UpdateBudget(const FMLCategoryBudgetData& budgetData) = 0;
    virtual bool DeleteBudget(const std::string& category) = 0;
    virtual std::vector<FMLCategoryBudgetData> GetAllBudgets() const = 0;
    virtual FMLCategoryBudgetData GetBudget(const std::string& category) const = 0;

    // Budget Summary
    virtual FMLBudgetSummary GetBudgetSummary() const = 0;
    virtual FMLBudgetSummary GetFilteredBudgetSummary(const FMLFilterCriteria& criteria) const = 0;
};
