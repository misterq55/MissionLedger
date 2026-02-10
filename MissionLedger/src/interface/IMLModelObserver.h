#pragma once
#include <memory>
#include <string>

class FMLTransaction;
struct FMLTransactionData;
struct FMLItemBudgetData;

class IMLModelObserver
{
public:
    virtual ~IMLModelObserver() = default;

    // Transaction events
    virtual void OnTransactionAdded(const FMLTransactionData& transactionData) = 0;
    virtual void OnTransactionDeleted(const int transactionId) = 0;
    virtual void OnTransactionUpdated(const FMLTransactionData& transactionData) = 0;

    // Budget events
    virtual void OnBudgetAdded(const FMLItemBudgetData& budgetData) = 0;
    virtual void OnBudgetDeleted(const int budgetId) = 0;
    virtual void OnBudgetUpdated(const FMLItemBudgetData& budgetData) = 0;
    
    // Data events
    virtual void OnTransactionsCleared() = 0;
    virtual void OnBudgetCleared() = 0;
    virtual void OnDataLoaded() = 0;
    virtual void OnDataSaved() = 0;
};