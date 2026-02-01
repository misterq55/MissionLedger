#include "MLTransaction.h"

FMLTransaction::FMLTransaction()
    : Id(0)
      , Type(E_MLTransactionType::Income)
      , Category("")
      , Item("")
      , Description("")
      , Amount(0)
      , DateTime("")
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
      , DateTime(dateTime)
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

const std::string& FMLTransaction::GetDateTime() const
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

void FMLTransaction::SetDateTime(const std::string& dateTimeStr)
{
    DateTime = dateTimeStr;
}

void FMLTransaction::ApplyData(const FMLTransactionData& data)
{
    Type = data.Type;
    Category = data.Category;
    Item = data.Item;
    Description = data.Description;
    Amount = data.Amount;
    DateTime = data.DateTime;
    ReceiptNumber = data.ReceiptNumber;
}

// Utility methods
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
