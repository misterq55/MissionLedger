#pragma once

#include "interface/IMLStorageProvider.h"
#include <string>

struct sqlite3;

/// <summary>
/// SQLite 기반 저장소 구현
/// </summary>
class FMLSQLiteStorage : public IMLStorageProvider
{
public:
    FMLSQLiteStorage();
    virtual ~FMLSQLiteStorage();

    // IMLStorageProvider 인터페이스 구현 (DTO 기반)
    bool Initialize(const std::string& filePath) override;
    bool Close() override;
    bool SaveTransaction(const FMLTransactionData& data) override;
    bool SaveAllTransactions(const std::vector<FMLTransactionData>& transactions) override;
    bool LoadAllTransactions(std::vector<FMLTransactionData>& outTransactions) override;
    bool DeleteTransaction(int transactionId) override;
    bool UpdateTransaction(const FMLTransactionData& data) override;
    int GetLastTransactionId() override;
    E_MLStorageType GetStorageType() const override;
    bool IsReady() const override;

    // 예산 관련 메서드
    bool SaveBudget(const FMLItemBudgetData& budget) override;
    bool LoadAllBudgets(std::vector<FMLItemBudgetData>& outBudgets) override;
    bool DeleteBudget(const int budgetId) override;

private:
    /// <summary>
    /// 트랜잭션 테이블 생성
    /// </summary>
    bool createTable();

private:
    sqlite3* Database = nullptr;
    std::string FilePath;
    bool IsInitialized = false;
};
