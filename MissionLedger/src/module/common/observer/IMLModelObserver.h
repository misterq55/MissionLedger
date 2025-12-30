#pragma once
#include <memory>

class FMLTransaction;

class IMLModelObserver
{
public:
    virtual ~IMLModelObserver() = default;

    // Transaction events
    virtual void OnTransactionAdded(std::shared_ptr<FMLTransaction> transaction) = 0;
    virtual void OnTransactionRemoved(int transactionId) = 0;
    virtual void OnTransactionUpdated(std::shared_ptr<FMLTransaction> transaction) = 0;

    // Data events
    virtual void OnTransactionsCleared() = 0;
    virtual void OnDataLoaded() = 0;
    virtual void OnDataSaved() = 0;
};