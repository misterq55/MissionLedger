#include "MLModel.h"
#include "module/mvc/model/transaction/MLTransaction.h"

void FMLModel::AddTransaction(const FMLTransactionData& transactionData)
{
    Transactions.emplace(TransactionIdIndex,
                         std::make_shared<FMLTransaction>(TransactionIdIndex, transactionData.Type,
                                                          transactionData.Category, transactionData.Item,
                                                          transactionData.Description, transactionData.Amount,
                                                          transactionData.ReceiptNumber));
    TransactionIdIndex++;
}

bool FMLModel::RemoveTransaction(const int transactionId)
{
    Transactions.erase(TransactionIdIndex);
    return true;
}

int FMLModel::GetTransactionId()
{
    return 0;
}
