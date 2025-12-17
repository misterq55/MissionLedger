#pragma once
#include <map>
#include <memory>

#include "interface/IMLModel.h"

class FMLTransaction;
class FMLModel : public IMLModel
{
public:
    virtual void AddTransaction(const FMLTransactionData& transactionData) override;
    virtual bool RemoveTransaction(const int transactionId) override;
    virtual int GetTransactionId() override;
    
private:
    std::map<int, std::shared_ptr<FMLTransaction>> Transactions;
    int TransactionIdIndex = 0;
};
