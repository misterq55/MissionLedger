#pragma once
#include <memory>

class FMLTransaction;
struct FMLTransactionData;

class IMLModelObserver
{
public:
    virtual ~IMLModelObserver() = default;

    // Transaction events
    virtual void OnTransactionAdded(const FMLTransactionData& transactionData) = 0;
    virtual void OnTransactionRemoved(int transactionId) = 0;
    virtual void OnTransactionUpdated(const FMLTransactionData& transactionData) = 0;

    // Data events
    virtual void OnTransactionsCleared() = 0;
    virtual void OnDataLoaded() = 0;
    virtual void OnDataSaved() = 0;
};