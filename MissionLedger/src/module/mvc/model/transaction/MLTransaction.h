#pragma once
#include "MLDefine.h"
#include <string>

class FMLTransaction
{
public:
    FMLTransaction();
    FMLTransaction(int id, E_MLTransactionType type, const std::string& category,
                   const std::string& item, const std::string& description,
                   int64_t amount, const std::string& dateTime, const std::string& receiptNumber = "");

    // Getters
    int GetId() const;
    E_MLTransactionType GetType() const;
    const std::string& GetCategory() const; // 구분
    const std::string& GetItem() const; // 항목
    const std::string& GetDescription() const; // 내용
    int64_t GetAmount() const; // (수입/지출) 금액
    const std::string& GetDateTime() const; // 년월일 (YYYY-MM-DD)
    const std::string& GetReceiptNumber() const; // 영수증 No

    // Setters
    void SetId(int id);
    void SetType(E_MLTransactionType type);
    void SetCategory(const std::string& category);
    void SetItem(const std::string& item);
    void SetDescription(const std::string& description);
    void SetAmount(int64_t amount);
    void SetDateTime(const std::string& dateTimeStr);
    void SetReceiptNumber(const std::string& receiptNumber);

    // Apply data from DTO
    void ApplyData(const FMLTransactionData& data);

    // Utility methods
    std::string GetTypeString() const;
    bool IsIncome() const;
    bool IsExpense() const;

private:
    int Id;
    E_MLTransactionType Type;
    std::string Category; // 구분
    std::string Item; // 항목
    std::string Description; // 내용
    int64_t Amount; // (수입/지출) 금액
    std::string DateTime; // 년월일 (YYYY-MM-DD)
    std::string ReceiptNumber; // 영수증 No
};
