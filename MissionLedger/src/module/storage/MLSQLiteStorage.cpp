#include "MLSQLiteStorage.h"
#include "module/mvc/model/transaction/MLTransaction.h"
#include "third_party/sqlite/sqlite3.h"
#include <sstream>
#include <iomanip>
#include <ctime>

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
            amount REAL NOT NULL,
            datetime TEXT NOT NULL,
            receipt_number TEXT
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
        (id, type, category, item, description, amount, datetime, receipt_number)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?);
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
    sqlite3_bind_double(stmt, 6, transaction.GetAmount());
    sqlite3_bind_text(stmt, 7, formatDateTime(transaction.GetDateTime()).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, transaction.GetReceiptNumber().c_str(), -1, SQLITE_TRANSIENT);

    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return result == SQLITE_DONE;
}

bool FMLSQLiteStorage::SaveAllTransactions(const std::vector<std::shared_ptr<FMLTransaction>>& transactions)
{
    if (!IsInitialized) return false;

    // 트랜잭션 시작
    sqlite3_exec(Database, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    // 기존 데이터 삭제
    sqlite3_exec(Database, "DELETE FROM transactions;", nullptr, nullptr, nullptr);

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
    sqlite3_exec(Database, "COMMIT;", nullptr, nullptr, nullptr);
    return true;
}

bool FMLSQLiteStorage::LoadAllTransactions(std::vector<std::shared_ptr<FMLTransaction>>& outTransactions)
{
    if (!IsInitialized) return false;

    outTransactions.clear();

    const char* sql = "SELECT id, type, category, item, description, amount, datetime, receipt_number FROM transactions ORDER BY id;";

    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(Database, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK)
    {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        E_MLTransactionType type = static_cast<E_MLTransactionType>(sqlite3_column_int(stmt, 1));
        std::string category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string item = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        const char* descPtr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        std::string description = descPtr ? descPtr : "";

        double amount = sqlite3_column_double(stmt, 5);
        std::string dateTimeStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

        const char* receiptPtr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        std::string receiptNumber = receiptPtr ? receiptPtr : "";

        auto transaction = std::make_shared<FMLTransaction>(id, type, category, item, description, amount, receiptNumber);
        transaction->SetDateTime(parseDateTime(dateTimeStr));

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

std::chrono::system_clock::time_point FMLSQLiteStorage::parseDateTime(const std::string& dateTimeStr)
{
    std::tm tm = {};
    std::istringstream ss(dateTimeStr);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    if (ss.fail())
    {
        // 날짜만 있는 경우 (YYYY-MM-DD)
        ss.clear();
        ss.str(dateTimeStr);
        ss >> std::get_time(&tm, "%Y-%m-%d");
    }

    std::time_t time = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(time);
}

std::string FMLSQLiteStorage::formatDateTime(const std::chrono::system_clock::time_point& timePoint)
{
    std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
    std::tm tm;
    localtime_s(&tm, &time);

    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
