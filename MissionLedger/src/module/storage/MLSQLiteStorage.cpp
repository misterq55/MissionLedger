#include "MLSQLiteStorage.h"
#include "module/mvc/model/transaction/MLTransaction.h"
#include "third_party/sqlite/sqlite3.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <iostream>

FMLSQLiteStorage::FMLSQLiteStorage()
{
}

FMLSQLiteStorage::~FMLSQLiteStorage()
{
    Close();
}

bool FMLSQLiteStorage::Initialize(const std::string& filePath)
{
    if (IsInitialized)
    {
        Close();
    }

    FilePath = filePath;

    int result = sqlite3_open(filePath.c_str(), &Database);
    if (result != SQLITE_OK)
    {
        sqlite3_close(Database);
        Database = nullptr;
        return false;
    }

    if (!createTable())
    {
        Close();
        return false;
    }

    IsInitialized = true;
    return true;
}

bool FMLSQLiteStorage::Close()
{
    if (Database)
    {
        sqlite3_close(Database);
        Database = nullptr;
    }
    IsInitialized = false;
    return true;
}

bool FMLSQLiteStorage::createTable()
{
    // transactions 테이블 생성
    const char* transactionsSql = R"(
        CREATE TABLE IF NOT EXISTS transactions (
            id INTEGER PRIMARY KEY,
            type INTEGER NOT NULL,
            category TEXT NOT NULL,
            item TEXT NOT NULL,
            description TEXT,
            amount INTEGER NOT NULL,
            datetime TEXT NOT NULL,
            receipt_number TEXT,
            use_exchange_rate INTEGER DEFAULT 0,
            currency TEXT DEFAULT 'KRW',
            original_amount REAL DEFAULT 0.0,
            exchange_rate REAL DEFAULT 1.0
        );
    )";

    char* errorMsg = nullptr;
    int result = sqlite3_exec(Database, transactionsSql, nullptr, nullptr, &errorMsg);

    if (result != SQLITE_OK)
    {
        sqlite3_free(errorMsg);
        return false;
    }

    // budgets 테이블 생성
    const char* budgetsSql = R"(
        CREATE TABLE IF NOT EXISTS budgets (
            category TEXT NOT NULL,
            item TEXT NOT NULL,
            budget_amount INTEGER DEFAULT 0,
            PRIMARY KEY (budgetId)
        );
    )";

    result = sqlite3_exec(Database, budgetsSql, nullptr, nullptr, &errorMsg);

    if (result != SQLITE_OK)
    {
        sqlite3_free(errorMsg);
        return false;
    }

    return true;
}

bool FMLSQLiteStorage::SaveTransaction(const FMLTransactionData& data)
{
    if (!IsInitialized) return false;

    const char* sql = R"(
        INSERT OR REPLACE INTO transactions
        (id, type, budgetId, description, amount, datetime, receipt_number,
         use_exchange_rate, currency, original_amount, exchange_rate)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(Database, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_int(stmt, 1, data.TransactionId);
    sqlite3_bind_int(stmt, 2, static_cast<int>(data.Type));
    sqlite3_bind_text(stmt, 3, data.Category.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, data.Item.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, data.Description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 6, data.Amount);
    sqlite3_bind_text(stmt, 7, data.DateTime.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, data.ReceiptNumber.c_str(), -1, SQLITE_TRANSIENT);

    // 환율 관련 필드
    sqlite3_bind_int(stmt, 9, data.UseExchangeRate ? 1 : 0);
    sqlite3_bind_text(stmt, 10, data.Currency.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 11, data.OriginalAmount);
    sqlite3_bind_double(stmt, 12, data.ExchangeRate);

    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return result == SQLITE_DONE;
}

bool FMLSQLiteStorage::SaveAllTransactions(const std::vector<FMLTransactionData>& transactions)
{
    if (!IsInitialized) return false;

    // 트랜잭션 시작
    char* errorMsg = nullptr;
    int result = sqlite3_exec(Database, "BEGIN TRANSACTION;", nullptr, nullptr, &errorMsg);
    if (result != SQLITE_OK)
    {
        sqlite3_free(errorMsg);
        return false;
    }

    // 기존 데이터 삭제
    result = sqlite3_exec(Database, "DELETE FROM transactions;", nullptr, nullptr, &errorMsg);
    if (result != SQLITE_OK)
    {
        sqlite3_free(errorMsg);
        sqlite3_exec(Database, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    // 모든 거래 저장
    for (const auto& data : transactions)
    {
        if (!SaveTransaction(data))
        {
            sqlite3_exec(Database, "ROLLBACK;", nullptr, nullptr, nullptr);
            return false;
        }
    }

    // 커밋
    result = sqlite3_exec(Database, "COMMIT;", nullptr, nullptr, &errorMsg);
    if (result != SQLITE_OK)
    {
        sqlite3_free(errorMsg);
        return false;
    }

    return true;
}

bool FMLSQLiteStorage::LoadAllTransactions(std::vector<FMLTransactionData>& outTransactions)
{
    if (!IsInitialized) return false;

    outTransactions.clear();

    const char* sql = R"(
        SELECT id, type, budgetId, description, amount, datetime, receipt_number,
               use_exchange_rate, currency, original_amount, exchange_rate
        FROM transactions ORDER BY id;
    )";

    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(Database, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        FMLTransactionData data;

        data.TransactionId = sqlite3_column_int(stmt, 0);
        data.Type = static_cast<E_MLTransactionType>(sqlite3_column_int(stmt, 1));
        data.Category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        data.Item = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        const char* descPtr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        data.Description = descPtr ? descPtr : "";

        data.Amount = sqlite3_column_int64(stmt, 5);
        data.DateTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

        const char* receiptPtr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        data.ReceiptNumber = receiptPtr ? receiptPtr : "";

        // 환율 관련 필드
        data.UseExchangeRate = sqlite3_column_int(stmt, 8) != 0;

        const char* currencyPtr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        data.Currency = currencyPtr ? currencyPtr : "KRW";

        data.OriginalAmount = sqlite3_column_double(stmt, 10);
        data.ExchangeRate = sqlite3_column_double(stmt, 11);

        outTransactions.push_back(data);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool FMLSQLiteStorage::DeleteTransaction(int transactionId)
{
    if (!IsInitialized) return false;

    const char* sql = "DELETE FROM transactions WHERE id = ?;";

    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(Database, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_int(stmt, 1, transactionId);
    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return result == SQLITE_DONE;
}

bool FMLSQLiteStorage::UpdateTransaction(const FMLTransactionData& data)
{
    // SaveTransaction이 INSERT OR REPLACE를 사용하므로 동일하게 동작
    return SaveTransaction(data);
}

int FMLSQLiteStorage::GetLastTransactionId()
{
    if (!IsInitialized) return -1;

    const char* sql = "SELECT MAX(id) FROM transactions;";

    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(Database, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        return -1;
    }

    int lastId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (sqlite3_column_type(stmt, 0) != SQLITE_NULL)
        {
            lastId = sqlite3_column_int(stmt, 0);
        }
    }

    sqlite3_finalize(stmt);
    return lastId;
}

E_MLStorageType FMLSQLiteStorage::GetStorageType() const
{
    return E_MLStorageType::SQLite;
}

bool FMLSQLiteStorage::IsReady() const
{
    return IsInitialized && Database != nullptr;
}

// ========== 예산 관련 메서드 구현 ==========

bool FMLSQLiteStorage::SaveBudget(const FMLItemBudgetData& budget)
{
    if (!IsInitialized) return false;

    const char* sql = R"(
        INSERT OR REPLACE INTO budgets (budgetId, budget_amount)
        VALUES (?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(Database, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, budget.Category.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, budget.Item.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, budget.BudgetAmount);

    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return result == SQLITE_DONE;
}

bool FMLSQLiteStorage::LoadAllBudgets(std::vector<FMLItemBudgetData>& outBudgets)
{
    if (!IsInitialized) return false;

    outBudgets.clear();

    const char* sql = "SELECT budgetId, budget_amount FROM budgets ORDER BY budgetId;";

    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(Database, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) return false;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        FMLItemBudgetData budget;
        budget.Category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        budget.Item = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        budget.BudgetAmount = sqlite3_column_int64(stmt, 2);
        outBudgets.push_back(budget);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool FMLSQLiteStorage::DeleteBudget(const int budgetId)
{
    if (!IsInitialized) return false;

    const char* sql = "DELETE FROM budgets WHERE category = ? AND item = ?;";

    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(Database, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) return false;

    // sqlite3_bind_text(stmt, 1, category.c_str(), -1, SQLITE_TRANSIENT);
    // sqlite3_bind_text(stmt, 2, item.c_str(), -1, SQLITE_TRANSIENT);
    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return result == SQLITE_DONE;
}
