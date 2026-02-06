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
      , UseExchangeRate(false)
      , Currency("KRW")
      , OriginalAmount(0.0)
      , ExchangeRate(1.0)
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

// 환율 관련 Getters
bool FMLTransaction::GetUseExchangeRate() const
{
    return UseExchangeRate;
}

const std::string& FMLTransaction::GetCurrency() const
{
    return Currency;
}

double FMLTransaction::GetOriginalAmount() const
{
    return OriginalAmount;
}

double FMLTransaction::GetExchangeRate() const
{
    return ExchangeRate;
}

void FMLTransaction::ApplyData(const FMLTransactionData& data)
{
    Id = data.TransactionId;
    Type = data.Type;
    Category = data.Category;
    Item = data.Item;
    Description = data.Description;
    Amount = data.Amount;
    DateTime = data.DateTime;
    ReceiptNumber = data.ReceiptNumber;

    // 환율 관련 필드
    UseExchangeRate = data.UseExchangeRate;
    Currency = data.Currency;
    OriginalAmount = data.OriginalAmount;
    ExchangeRate = data.ExchangeRate;
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
