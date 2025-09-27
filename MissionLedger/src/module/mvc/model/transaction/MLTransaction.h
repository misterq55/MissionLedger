﻿#pragma once
#include "MLDefine.h"
#include <string>
#include <chrono>

class CMLTransaction
{
public:
    CMLTransaction();
    CMLTransaction(int id, E_MLTransactionType type, const std::string& category,
                   const std::string& item, const std::string& description,
                   double amount, const std::string& receiptNumber = "");

    // Getters
    int GetId() const;
    E_MLTransactionType GetType() const;
    const std::string& GetCategory() const;    // 구분
    const std::string& GetItem() const;        // 항목
    const std::string& GetDescription() const; // 내용
    double GetAmount() const;                  // (수입/지출) 금액
    const std::chrono::system_clock::time_point& GetDateTime() const; // 년월일
    const std::string& GetReceiptNumber() const; // 영수증 No

    // Setters
    void SetId(int id);
    void SetType(E_MLTransactionType type);
    void SetCategory(const std::string& category);
    void SetItem(const std::string& item);
    void SetDescription(const std::string& description);
    void SetAmount(double amount);
    void SetDateTime(const std::chrono::system_clock::time_point& dateTime);
    void SetReceiptNumber(const std::string& receiptNumber);

    // Utility methods
    std::string GetDateTimeString() const;
    std::string GetTypeString() const;
    bool IsIncome() const;
    bool IsExpense() const;

private:
    int Id;
    E_MLTransactionType Type;
    std::string Category;        // 구분
    std::string Item;            // 항목
    std::string Description;     // 내용
    double Amount;               // (수입/지출) 금액
    std::chrono::system_clock::time_point DateTime; // 년월일
    std::string ReceiptNumber;   // 영수증 No
};
