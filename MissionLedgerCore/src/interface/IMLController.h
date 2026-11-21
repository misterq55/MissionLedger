#pragma once
#include <string>
#include <vector>

struct FMLTransactionData;
struct FMLFilterCriteria;
struct FMLTransactionSummary;
struct FMLBudgetData;
struct FMLBudgetSummary;
struct FMLPDFExportOptions;

class IMLController
{
public:
    virtual ~IMLController() = default;

    // Transaction operations
    virtual bool AddTransaction(const FMLTransactionData& transactionData) = 0;
    virtual bool UpdateTransaction(const FMLTransactionData& transactionData) = 0;
    virtual bool DeleteTransaction(const int transactionId) = 0;

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
    virtual bool ExportSettlementToPDF(const std::string& filePath, const FMLPDFExportOptions& options) = 0;
    virtual bool ExportTransactionListToPDF(const std::string& filePath, const FMLPDFExportOptions& options) = 0;

    // File operations
    virtual void NewFile() = 0;
    virtual bool OpenFile(const std::string& filePath) = 0;
    virtual bool SaveFile() = 0;
    virtual bool SaveFileAs(const std::string& filePath) = 0;
    virtual std::string GetCurrentFilePath() const = 0;
    virtual bool HasUnsavedChanges() const = 0;
    // 외부 변경 감지: 파일을 연(또는 저장한) 뒤 다른 프로그램(CLI 등)이 파일을 바꿨는지, 그리고 다시 불러오기
    virtual bool HasExternalChanges() const = 0;
    virtual bool ReloadFile() = 0;

    // Budget operations
    virtual bool AddBudget(const FMLBudgetData& budgetData) = 0;
    virtual bool UpdateBudget(const FMLBudgetData& budgetData) = 0;
    virtual bool DeleteBudget(const int budgetId) = 0;
    virtual std::vector<FMLBudgetData> GetAllBudgets() const = 0;
    virtual FMLBudgetData GetBudget(const int budgetId) const = 0;

    // Budget Summary
    virtual FMLBudgetSummary GetBudgetSummary() const = 0;
    virtual FMLBudgetSummary GetFilteredBudgetSummary(const FMLFilterCriteria& criteria) const = 0;
};
