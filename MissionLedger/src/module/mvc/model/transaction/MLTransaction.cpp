#include "MLTransaction.h"
#include <sstream>
#include <iomanip>

CMLTransaction::CMLTransaction()
    : Id(0)
    , Type(E_MLTransactionType::Income)
    , Category("")
    , Item("")
    , Description("")
    , Amount(0.0)
    , DateTime(std::chrono::system_clock::now())
    , ReceiptNumber("")
{
}

CMLTransaction::CMLTransaction(int id, E_MLTransactionType type, const std::string& category,
                               const std::string& item, const std::string& description,
                               double amount, const std::string& receiptNumber)
    : Id(id)
    , Type(type)
    , Category(category)
    , Item(item)
    , Description(description)
    , Amount(amount)
    , DateTime(std::chrono::system_clock::now())
    , ReceiptNumber(receiptNumber)
{
}

// Getters
int CMLTransaction::GetId() const
{
    return Id;
}

E_MLTransactionType CMLTransaction::GetType() const
{
    return Type;
}

double CMLTransaction::GetAmount() const
{
    return Amount;
}

const std::string& CMLTransaction::GetCategory() const
{
    return Category;
}

const std::string& CMLTransaction::GetItem() const
{
    return Item;
}

const std::string& CMLTransaction::GetDescription() const
{
    return Description;
}

const std::string& CMLTransaction::GetReceiptNumber() const
{
    return ReceiptNumber;
}

const std::chrono::system_clock::time_point& CMLTransaction::GetDateTime() const
{
    return DateTime;
}

// Setters
void CMLTransaction::SetId(int id)
{
    Id = id;
}

void CMLTransaction::SetType(E_MLTransactionType type)
{
    Type = type;
}

void CMLTransaction::SetAmount(double amount)
{
    Amount = amount;
}

void CMLTransaction::SetCategory(const std::string& category)
{
    Category = category;
}

void CMLTransaction::SetItem(const std::string& item)
{
    Item = item;
}

void CMLTransaction::SetDescription(const std::string& description)
{
    Description = description;
}

void CMLTransaction::SetReceiptNumber(const std::string& receiptNumber)
{
    ReceiptNumber = receiptNumber;
}

void CMLTransaction::SetDateTime(const std::chrono::system_clock::time_point& dateTime)
{
    DateTime = dateTime;
}

// Utility methods
std::string CMLTransaction::GetDateTimeString() const
{
    auto time_t = std::chrono::system_clock::to_time_t(DateTime);
    struct tm timeInfo;

#ifdef _WIN32
    localtime_s(&timeInfo, &time_t);
#else
    localtime_r(&time_t, &timeInfo);
#endif

    std::stringstream ss;
    ss << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string CMLTransaction::GetTypeString() const
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

bool CMLTransaction::IsIncome() const
{
    return Type == E_MLTransactionType::Income;
}

bool CMLTransaction::IsExpense() const
{
    return Type == E_MLTransactionType::Expense;
}
