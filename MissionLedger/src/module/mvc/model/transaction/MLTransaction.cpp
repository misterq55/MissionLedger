#include "MLTransaction.h"
#include <sstream>
#include <iomanip>

FMLTransaction::FMLTransaction()
    : Id(0)
      , Type(E_MLTransactionType::Income)
      , Category("")
      , Item("")
      , Description("")
      , Amount(0)
      , DateTime(std::chrono::system_clock::now())
      , ReceiptNumber("")
{
}

FMLTransaction::FMLTransaction(int id, E_MLTransactionType type, const std::string& category,
                               const std::string& item, const std::string& description,
                               int64_t amount, const std::string& dateTime, const std::string& receiptNumber)
    : Id(id)
      , Type(type)
      , Category(category)
      , Item(item)
      , Description(description)
      , Amount(amount)
      , DateTime(parseDateTime(dateTime))
      , ReceiptNumber(receiptNumber)
{
}

// Getters
int FMLTransaction::GetId() const
{
    return Id;
}

E_MLTransactionType FMLTransaction::GetType() const
{
    return Type;
}

int64_t FMLTransaction::GetAmount() const
{
    return Amount;
}

const std::string& FMLTransaction::GetCategory() const
{
    return Category;
}

const std::string& FMLTransaction::GetItem() const
{
    return Item;
}

const std::string& FMLTransaction::GetDescription() const
{
    return Description;
}

const std::string& FMLTransaction::GetReceiptNumber() const
{
    return ReceiptNumber;
}

const std::chrono::system_clock::time_point& FMLTransaction::GetDateTime() const
{
    return DateTime;
}

// Setters
void FMLTransaction::SetId(int id)
{
    Id = id;
}

void FMLTransaction::SetType(E_MLTransactionType type)
{
    Type = type;
}

void FMLTransaction::SetAmount(int64_t amount)
{
    Amount = amount;
}

void FMLTransaction::SetCategory(const std::string& category)
{
    Category = category;
}

void FMLTransaction::SetItem(const std::string& item)
{
    Item = item;
}

void FMLTransaction::SetDescription(const std::string& description)
{
    Description = description;
}

void FMLTransaction::SetReceiptNumber(const std::string& receiptNumber)
{
    ReceiptNumber = receiptNumber;
}

void FMLTransaction::SetDateTime(const std::chrono::system_clock::time_point& dateTime)
{
    DateTime = dateTime;
}

void FMLTransaction::SetDateTime(const std::string& dateTimeStr)
{
    DateTime = parseDateTime(dateTimeStr);
}

void FMLTransaction::ApplyData(const FMLTransactionData& data)
{
    Type = data.Type;
    Category = data.Category;
    Item = data.Item;
    Description = data.Description;
    Amount = data.Amount;
    DateTime = parseDateTime(data.DateTime);
    ReceiptNumber = data.ReceiptNumber;
}

// Private helper
std::chrono::system_clock::time_point FMLTransaction::parseDateTime(const std::string& dateTimeStr)
{
    if (dateTimeStr.empty())
    {
        return std::chrono::system_clock::now();
    }

    std::tm tm = {};
    std::istringstream ss(dateTimeStr);

    // 날짜만 파싱 (YYYY-MM-DD)
    ss >> std::get_time(&tm, "%Y-%m-%d");

    if (ss.fail())
    {
        // 파싱 실패 시 현재 날짜 반환
        return std::chrono::system_clock::now();
    }

    // 시간은 00:00:00으로 고정
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;

    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

// Utility methods
std::string FMLTransaction::GetDateTimeString() const
{
    auto time_t = std::chrono::system_clock::to_time_t(DateTime);
    struct tm timeInfo;

#ifdef _WIN32
    localtime_s(&timeInfo, &time_t);
#else
    localtime_r(&time_t, &timeInfo);
#endif

    std::stringstream ss;
    ss << std::put_time(&timeInfo, "%Y-%m-%d");
    return ss.str();
}

std::string FMLTransaction::GetTypeString() const
{
    switch (Type)
    {
    case E_MLTransactionType::Income:
        return "수입";
    case E_MLTransactionType::Expense:
        return "지출";
    default:
        return "알 수 없음";
    }
}

bool FMLTransaction::IsIncome() const
{
    return Type == E_MLTransactionType::Income;
}

bool FMLTransaction::IsExpense() const
{
    return Type == E_MLTransactionType::Expense;
}
