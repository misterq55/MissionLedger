#include "MLModel.h"
#include "module/mvc/model/transaction/MLTransaction.h"

void CMLModel::AddTransaction(const SMLTransactionData& transactionData)
{
    Transactions.emplace(TransactionIdIndex,
                         std::make_shared<CMLTransaction>(TransactionIdIndex, transactionData.Type,
                                                          transactionData.Category, transactionData.Item,
                                                          transactionData.Description, transactionData.Amount,
                                                          transactionData.ReceiptNumber));
    TransactionIdIndex++;
}

bool CMLModel::RemoveTransaction(const int transactionId)
{
    Transactions.erase(TransactionIdIndex);
    return true;
}

int CMLModel::GetTransactionId()
{
    return 0;
}
