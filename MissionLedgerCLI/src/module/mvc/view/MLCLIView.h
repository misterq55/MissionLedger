#pragma once
#include "interface/IMLView.h"
#include "interface/IMLModelObserver.h"
#include "MLDefine.h"
#include <iostream>
#include <vector>
#include <string>
#include <map>

/**
 * @brief CLI View implementation
 *
 * IMLView와 IMLModelObserver를 구현하는 콘솔 기반 View입니다.
 * 트랜잭션 데이터를 표준 출력으로 표시하고, Model의 변경 사항을
 * 콘솔에 로깅합니다.
 *
 * 사용자 입력(명령어)을 처리하고 Controller를 통해 작업을 수행합니다.
 *
 * 실행 모드:
 * - 대화형 모드: 인수 없이 실행 시 명령어 루프 실행
 * - 명령어 모드: 명령줄 인수로 직접 명령 실행
 *   예: MissionLedgerCLI.exe list
 *       MissionLedgerCLI.exe add --type income --amount 1000 --category "급여"
 *       MissionLedgerCLI.exe open data.ml
 */
class FMLCLIView : public IMLView, public IMLModelObserver
{
public:
    FMLCLIView();
    virtual ~FMLCLIView();

    // Main entry points
    void Run();                                    // 대화형 모드
    int RunWithArgs(int argc, char* argv[]);       // 명령어 모드 (반환: 0=성공, 1=실패)

    // IMLView implementation
    virtual void AddTransaction(const FMLTransactionData& data) override;
    virtual void DisplayTransaction(const FMLTransactionData& data) override;
    virtual void DisplayTransactions(const std::vector<FMLTransactionData>& data) override;
    virtual void AddBudget(const FMLBudgetData& data) override;
    virtual void DisplayBudget(const FMLBudgetData& data) override;
    virtual void DisplayBudgets(const std::vector<FMLBudgetData>& data) override;

    // IMLModelObserver implementation
    virtual void OnTransactionAdded(const FMLTransactionData& transactionData) override;
    virtual void OnTransactionDeleted(const int transactionId) override;
    virtual void OnTransactionUpdated(const FMLTransactionData& transactionData) override;
    virtual void OnTransactionsCleared() override;
    virtual void OnBudgetAdded(const FMLBudgetData& budgetData) override;
    virtual void OnBudgetDeleted(const int budgetId) override;
    virtual void OnBudgetUpdated(const FMLBudgetData& budgetData) override;
    virtual void OnBudgetCleared() override;
    virtual void OnDataLoaded() override;
    virtual void OnDataSaved() override;

private:
    // 명령어 모드 - 명령줄 인수 처리
    int cmdHelp();
    int cmdList(const std::string& filePath);
    int cmdAdd(const std::map<std::string, std::string>& options);
    int cmdOpen(const std::string& filePath);
    int cmdNew(const std::string& filePath);
    int cmdSave();
    int cmdBudgetList(const std::string& filePath);
    int cmdBudgetAdd(const std::map<std::string, std::string>& options);
    int cmdBudgetUpdate(const std::map<std::string, std::string>& options);
    int cmdBudgetDelete(const std::map<std::string, std::string>& options);

    // 명령줄 인수 파싱 헬퍼
    std::map<std::string, std::string> parseOptions(int argc, char* argv[], int startIndex);
    void printUsage();

    // 대화형 모드 - 명령어 처리
    void printHelp();
    void handleNewFile();
    void handleOpenFile();
    void handleSaveFile();
    void handleAddTransaction();
    void handleListTransactions();
    void handleAddBudget();
    void handleListBudgets();

    // 출력 헬퍼
    void printTransaction(const FMLTransactionData& data, bool showHeader = false);
    void printBudget(const FMLBudgetData& data, bool showHeader = false);
    void printSeparator();
    std::string getTypeString(E_MLTransactionType type);
    std::string getBudgetTypeString(E_MLTransactionType type);
    std::string formatAmount(int64_t amount);

    // 상태
    bool SilentMode = false;  // 명령어 모드에서 이벤트 출력 억제
};
