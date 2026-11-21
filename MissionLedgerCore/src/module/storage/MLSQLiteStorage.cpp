#include "MLSQLiteStorage.h"
#include "module/mvc/model/transaction/MLTransaction.h"
#include "sqlite3.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <filesystem>
#include <system_error>

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

    // 다른 프로그램(GUI/CLI)이 쓰는 중이면 바로 실패하지 않고 잠시 기다린다
    sqlite3_busy_timeout(Database, 5000);

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
            exchange_rate REAL DEFAULT 1.0,
            budget_id INTEGER DEFAULT -1
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
            budget_id INTEGER PRIMARY KEY AUTOINCREMENT,
            type INTEGER NOT NULL,
            category TEXT NOT NULL,
            item TEXT,
            budget_amount INTEGER DEFAULT 0,
            notes TEXT
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
        (id, type, category, item, description, amount, datetime, receipt_number,
         use_exchange_rate, currency, original_amount, exchange_rate, budget_id)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
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

    // 예산 연결 필드
    sqlite3_bind_int(stmt, 13, data.BudgetId);

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
        SELECT id, type, category, item, description, amount, datetime, receipt_number,
               use_exchange_rate, currency, original_amount, exchange_rate, budget_id
        FROM transactions ORDER BY id;
    )";

    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(Database, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        return false;
    }

    int rowCount = 0;
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

        // 예산 연결 필드
        data.BudgetId = sqlite3_column_int(stmt, 12);

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

bool FMLSQLiteStorage::SaveBudget(const FMLBudgetData& budget)
{
    if (!IsInitialized) return false;

    const char* sql = R"(
        INSERT OR REPLACE INTO budgets (budget_id, type, category, item, budget_amount, notes)
        VALUES (?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(Database, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) return false;

    // BudgetId가 -1이면 NULL (AUTOINCREMENT), 아니면 기존 ID 사용
    if (budget.BudgetId == -1) {
        sqlite3_bind_null(stmt, 1);
    } else {
        sqlite3_bind_int(stmt, 1, budget.BudgetId);
    }
    sqlite3_bind_int(stmt, 2, static_cast<int>(budget.Type));
    sqlite3_bind_text(stmt, 3, budget.Category.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, budget.Item.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 5, budget.BudgetAmount);
    sqlite3_bind_text(stmt, 6, budget.Notes.c_str(), -1, SQLITE_TRANSIENT);

    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return result == SQLITE_DONE;
}

bool FMLSQLiteStorage::LoadAllBudgets(std::vector<FMLBudgetData>& outBudgets)
{
    if (!IsInitialized) return false;

    outBudgets.clear();

    const char* sql = "SELECT budget_id, type, category, item, budget_amount, notes FROM budgets ORDER BY budget_id;";

    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(Database, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) return false;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        FMLBudgetData budget;
        budget.BudgetId = sqlite3_column_int(stmt, 0);
        budget.Type = static_cast<E_MLTransactionType>(sqlite3_column_int(stmt, 1));
        budget.Category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        const char* item = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        budget.Item = item ? item : "";

        budget.BudgetAmount = sqlite3_column_int64(stmt, 4);

        const char* notes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        budget.Notes = notes ? notes : "";

        budget.ActualAmount = 0;  // DB에 저장 안 됨, 나중에 계산

        outBudgets.push_back(budget);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool FMLSQLiteStorage::DeleteBudget(const int budgetId)
{
    if (!IsInitialized) return false;

    const char* sql = "DELETE FROM budgets WHERE budget_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(Database, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, budgetId);

    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return result == SQLITE_DONE;
}

bool FMLSQLiteStorage::exec(const char* sql)
{
    char* errorMsg = nullptr;
    const int result = sqlite3_exec(Database, sql, nullptr, nullptr, &errorMsg);
    sqlite3_free(errorMsg);
    return result == SQLITE_OK;
}

void FMLSQLiteStorage::rollback()
{
    sqlite3_exec(Database, "ROLLBACK;", nullptr, nullptr, nullptr);
}

bool FMLSQLiteStorage::SaveAll(const std::vector<FMLTransactionData>& transactions, const std::vector<FMLBudgetData>& budgets)
{
    if (!IsInitialized) return false;

    // IMMEDIATE: 시작할 때 쓰기 잠금을 잡는다 (다른 쓰기가 끝나기를 busy_timeout만큼 기다림)
    if (!exec("BEGIN IMMEDIATE;")) return false;

    bool ok = exec("DELETE FROM transactions;");
    for (size_t i = 0; ok && i < transactions.size(); ++i)
        ok = SaveTransaction(transactions[i]);
    ok = ok && exec("DELETE FROM budgets;");
    for (size_t i = 0; ok && i < budgets.size(); ++i)
        ok = SaveBudget(budgets[i]);

    if (!ok || !exec("COMMIT;"))
    {
        rollback();
        return false;
    }
    return true;
}

bool FMLSQLiteStorage::LoadAll(std::vector<FMLTransactionData>& outTransactions, std::vector<FMLBudgetData>& outBudgets)
{
    if (!IsInitialized) return false;

    // 읽기 트랜잭션: 두 테이블을 같은 시점의 내용으로 읽는다
    if (!exec("BEGIN;")) return false;
    const bool ok = LoadAllTransactions(outTransactions) && LoadAllBudgets(outBudgets);
    if (!exec("COMMIT;"))
    {
        rollback();
        return false;
    }
    return ok;
}

std::string FMLSQLiteStorage::GetChangeToken()
{
    if (!IsInitialized) return std::string();

    // PRAGMA data_version: 이 연결이 아닌 다른 연결/프로세스가 커밋하면 바뀐다 (같은 연결 안에서만 비교 가능)
    long long dataVersion = -1;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(Database, "PRAGMA data_version;", -1, &stmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
            dataVersion = sqlite3_column_int64(stmt, 0);
        sqlite3_finalize(stmt);
    }

    // 파일이 통째로 바뀌는 경우 (클라우드 동기화 폴더 등): 연결은 옛 파일을 보고 있을 수 있어 경로의 시각/크기도 본다
    std::error_code ec;
    const std::filesystem::path path = std::filesystem::u8path(FilePath);
    const auto size = std::filesystem::file_size(path, ec);
    if (ec) return std::to_string(dataVersion) + "|missing";
    const auto time = std::filesystem::last_write_time(path, ec);
    if (ec) return std::to_string(dataVersion) + "|missing";

    // count()의 타입은 표준 라이브러리마다 다름 (MSVC: long long, macOS libc++: __int128). 같은 OS 안에서 비교만 하므로 long long으로
    const long long ticks = static_cast<long long>(time.time_since_epoch().count());
    return std::to_string(dataVersion) + "|" + std::to_string(ticks) + "|" + std::to_string(static_cast<unsigned long long>(size));
}

bool FMLSQLiteStorage::DeleteAllBudgets()
{
    if (!IsInitialized) return false;

    const char* sql = "DELETE FROM budgets;";

    char* errorMsg = nullptr;
    int result = sqlite3_exec(Database, sql, nullptr, nullptr, &errorMsg);

    if (result != SQLITE_OK)
    {
        sqlite3_free(errorMsg);
        return false;
    }

    return true;
}
