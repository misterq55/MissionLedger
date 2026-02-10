#pragma once
#include <map>
#include <memory>
#include <string>
#include <set>

#include "interface/IMLModel.h"

class FMLTransaction;
class FMLItemBudget;
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
    virtual bool AddTransaction(const FMLTransactionData& transactionData) override;
    virtual bool UpdateTransaction(const FMLTransactionData& transactionData) override;
    virtual bool DeleteTransaction(const int transactionId) override;

    // Data retrieval - DTO 기반
    virtual FMLTransactionData GetTransactionData(const int transactionId) const override;
    virtual std::vector<FMLTransactionData> GetAllTransactionData() const override;
    virtual std::vector<FMLTransactionData> GetFilteredTransactionData(const FMLFilterCriteria& criteria) const override;

    // Business logic
    virtual FMLTransactionSummary GetTransactionSummary() const override;
    virtual FMLTransactionSummary GetFilteredTransactionSummary(const FMLFilterCriteria& criteria) const override;
    virtual std::vector<std::string> GetAllCategories() const override;

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

    // Budget CRUD operations
    virtual bool AddBudget(const FMLItemBudgetData& budgetData) override;
    virtual bool UpdateBudget(const FMLItemBudgetData& budgetData) override;
    virtual bool DeleteBudget(const int budgetId) override;
    virtual std::vector<FMLItemBudgetData> GetAllBudgets() const override;
    virtual FMLItemBudgetData GetBudget(const int budgetId) const override;

    // Budget Summary
    virtual FMLBudgetSummary GetBudgetSummary() const override;
    virtual FMLBudgetSummary GetFilteredBudgetSummary(const FMLFilterCriteria& criteria) const override;

    // Storage Provider (구현 전용)
    void SetStorageProvider(std::shared_ptr<IMLStorageProvider> storageProvider);

private:
    void clearAllTransactions();
    FMLTransactionSummary calculateTransactionSummary(const std::vector<FMLTransactionData>& transactionData) const;

    // 카테고리 캐싱
    void rebuildCategoryCache() const;
    void invalidateCategoryCache();

private:
    std::shared_ptr<IMLModelObserver> ModelObserver;
    std::shared_ptr<IMLStorageProvider> StorageProvider;
    std::map<int, std::shared_ptr<FMLTransaction>> Transactions;
    std::map<int, std::shared_ptr<FMLItemBudget>> Budgets;

    std::string CurrentFilePath;
    int TransactionIdIndex = 0;
    int BudgetIdIndex = 0;
    bool UnsavedChanges = false;

    // 카테고리 캐시 (Lazy 업데이트)
    mutable std::set<std::string> CachedCategories;
    mutable bool CategoryCacheDirty = true;
};
