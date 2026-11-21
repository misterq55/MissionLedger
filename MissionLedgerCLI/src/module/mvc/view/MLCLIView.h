#pragma once
#include "interface/IMLView.h"
#include "interface/IMLModelObserver.h"
#include "MLDefine.h"
#include "MLCLIArgs.h"
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
 * - 대화형 모드: 인수 없이 실행하거나 shell <파일>
 * - 명령어 모드: MissionLedgerCLI <명령> <파일.ml> [옵션] (docs/CLI_DESIGN.md §3)
 *   예: MissionLedgerCLI list data.ml --type expense
 *       MissionLedgerCLI add data.ml --type income --category "후원금" --amount 1000
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
    // === 명령어 모드 (MLCLIViewCommands.cpp) ===
    // 명령 정의: 이름, 위치 인수, 옵션, 처리 함수. 파싱/검증/도움말이 모두 이 표에서 나온다
    struct FMLCLICommandSpec {
        std::string Name;            // "add", "budget add", "export settlement"
        std::string Positionals;     // 도움말용 표기 ("<파일>")
        size_t MinPositionals;
        size_t MaxPositionals;
        std::string Summary;
        std::vector<FMLCLIOptionSpec> Options;
        int (FMLCLIView::*Handler)(const FMLCLIArgs&);
        std::vector<std::string> Examples;
    };
    static const std::vector<FMLCLICommandSpec>& commandSpecs();
    static const FMLCLICommandSpec* findCommand(const std::string& name);

    // 도움말
    void printGeneralHelp(std::ostream& os) const;
    void printCommandHelp(std::ostream& os, const FMLCLICommandSpec& spec) const;
    static std::string usageLine(const FMLCLICommandSpec& spec);
    int usageError(const FMLCLICommandSpec* spec, const std::string& message) const;

    // 명령 처리 (반환: 0 성공, 1 실행 실패, 2 사용법 오류)
    int cmdHelp(const FMLCLIArgs& args);
    int cmdShell(const FMLCLIArgs& args);
    int cmdNew(const FMLCLIArgs& args);
    int cmdInfo(const FMLCLIArgs& args);
    int cmdList(const FMLCLIArgs& args);
    int cmdShow(const FMLCLIArgs& args);
    int cmdAdd(const FMLCLIArgs& args);
    int cmdUpdate(const FMLCLIArgs& args);
    int cmdDelete(const FMLCLIArgs& args);
    int cmdBudgetList(const FMLCLIArgs& args);
    int cmdBudgetAdd(const FMLCLIArgs& args);
    int cmdBudgetUpdate(const FMLCLIArgs& args);
    int cmdBudgetDelete(const FMLCLIArgs& args);
    int cmdExportSettlement(const FMLCLIArgs& args);
    int cmdExportTransactions(const FMLCLIArgs& args);

    // 명령 공통 헬퍼
    bool openFileForCommand(const std::string& filePath);
    int saveForCommand();
    int applyTransactionOptions(const FMLCLIArgs& args, FMLTransactionData& data, bool isAdd, std::string& error);
    int applyBudgetOptions(const FMLCLIArgs& args, FMLBudgetData& data, std::string& error);
    int confirmDelete(const FMLCLIArgs& args, const std::string& what);

    const FMLCLICommandSpec* CurrentCommand = nullptr;  // 사용법 오류 메시지용
    int LastAddedTransactionId = -1;
    int LastAddedBudgetId = -1;

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

    // 상태
    bool SilentMode = false;  // 명령어 모드에서 이벤트 출력 억제
};
