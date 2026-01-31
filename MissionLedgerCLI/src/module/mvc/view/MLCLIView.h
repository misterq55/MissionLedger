#pragma once
#include "interface/IMLView.h"
#include "interface/IMLModelObserver.h"
#include "MLDefine.h"
#include <iostream>
#include <vector>

/**
 * @brief CLI View implementation
 *
 * IMLView와 IMLModelObserver를 구현하는 콘솔 기반 View입니다.
 * 트랜잭션 데이터를 표준 출력으로 표시하고, Model의 변경 사항을
 * 콘솔에 로깅합니다.
 */
class FMLCLIView : public IMLView, public IMLModelObserver
{
public:
    FMLCLIView();
    virtual ~FMLCLIView();

    // IMLView implementation
    virtual void AddTransaction(const FMLTransactionData& data) override;
    virtual void DisplayTransaction(const FMLTransactionData& data) override;
    virtual void DisplayTransactions(const std::vector<FMLTransactionData>& data) override;

    // IMLModelObserver implementation
    virtual void OnTransactionAdded(const FMLTransactionData& transactionData) override;
    virtual void OnTransactionRemoved(int transactionId) override;
    virtual void OnTransactionUpdated(const FMLTransactionData& transactionData) override;
    virtual void OnTransactionsCleared() override;
    virtual void OnDataLoaded() override;
    virtual void OnDataSaved() override;

private:
    void printTransaction(const FMLTransactionData& data, bool showHeader = false);
    void printSeparator();
    std::string getTypeString(E_MLTransactionType type);
    std::string formatAmount(int64_t amount);
};
