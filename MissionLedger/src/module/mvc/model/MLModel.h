#pragma once
#include <map>
#include <memory>

#include "interface/IMLModel.h"

class CMLTransaction;
class MLModel : public IMLModel
{
public:
    virtual void AddTransaction(const SMLTransactionData& transactionData) override;
    virtual bool RemoveTransaction(const int transactionId) override;
    virtual int GetTransactionId() override;
    
private:
    std::map<int, std::shared_ptr<CMLTransaction>> Transactions;
};
