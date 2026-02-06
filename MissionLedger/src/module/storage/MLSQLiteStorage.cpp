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
    const char* sql = R"(
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
    int result = sqlite3_exec(Database, sql, nullptr, nullptr, &errorMsg);

    if (result != SQLITE_OK)
    {
        sqlite3_free(errorMsg);
        return false;
    }

    return true;
}

bool FMLSQLiteStorage::SaveTransaction(const FMLTransaction& transaction)
{
    if (!IsInitialized) return false;

    const char* sql = R"(
        INSERT OR REPLACE INTO transactions
        (id, type, category, item, description, amount, datetime, receipt_number,
         use_exchange_rate, currency, original_amount, exchange_rate)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(Database, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_int(stmt, 1, transaction.GetId());
    sqlite3_bind_int(stmt, 2, static_cast<int>(transaction.GetType()));
    sqlite3_bind_text(stmt, 3, transaction.GetCategory().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, transaction.GetItem().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, transaction.GetDescription().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 6, transaction.GetAmount());
    sqlite3_bind_text(stmt, 7, transaction.GetDateTime().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, transaction.GetReceiptNumber().c_str(), -1, SQLITE_TRANSIENT);

    // 환율 관련 필드
    sqlite3_bind_int(stmt, 9, transaction.GetUseExchangeRate() ? 1 : 0);
    sqlite3_bind_text(stmt, 10, transaction.GetCurrency().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 11, transaction.GetOriginalAmount());
    sqlite3_bind_double(stmt, 12, transaction.GetExchangeRate());

    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return result == SQLITE_DONE;
}

bool FMLSQLiteStorage::SaveAllTransactions(const std::vector<std::shared_ptr<FMLTransaction>>& transactions)
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
    for (const auto& transaction : transactions)
    {
        if (!SaveTransaction(*transaction))
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

bool FMLSQLiteStorage::LoadAllTransactions(std::vector<std::shared_ptr<FMLTransaction>>& outTransactions)
{
    if (!IsInitialized) return false;

    outTransactions.clear();

    const char* sql = R"(
        SELECT id, type, category, item, description, amount, datetime, receipt_number,
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
        // DTO 생성
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

        // Entity 생성 및 DTO 적용
        auto transaction = std::make_shared<FMLTransaction>();
        transaction->ApplyData(data);

        outTransactions.push_back(transaction);
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

bool FMLSQLiteStorage::UpdateTransaction(const FMLTransaction& transaction)
{
    // SaveTransaction이 INSERT OR REPLACE를 사용하므로 동일하게 동작
    return SaveTransaction(transaction);
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
