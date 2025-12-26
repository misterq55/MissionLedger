#pragma once
#include <map>
#include <memory>

#include "interface/IMLModel.h"

class FMLTransaction;
class FMLModel : public IMLModel
{
public:
    FMLModel();
    virtual ~FMLModel();
    
public:
    virtual void AddTransaction(const FMLTransactionData& transactionData) override;
    virtual bool RemoveTransaction(const int transactionId) override;
    virtual int GetTransactionId() override;
    virtual bool Save() override;
    virtual bool Load() override;
    
private:
    std::map<int, std::shared_ptr<FMLTransaction>> Transactions;
    int TransactionIdIndex = 0;
};
