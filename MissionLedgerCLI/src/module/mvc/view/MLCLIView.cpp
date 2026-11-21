#include "MLCLIView.h"
#include "MLDefine.h"
#include "module/common/holder/MLMVCHolder.h"
#include "interface/IMLController.h"
#include "module/common/format/MLFormat.h"
#include "module/common/currency/MLCurrency.h"
#include <iomanip>
#include <sstream>
#include <limits>
#include <cstring>


FMLCLIView::FMLCLIView()
{
}

FMLCLIView::~FMLCLIView()
{
}

// 대화형 모드 entry point
void FMLCLIView::Run()
{
    printHelp();

    std::string command;
    while (true)
    {
        std::cout << "> ";
        if (!(std::cin >> command))
        {
            // 입력 끝(EOF, 파이프/리다이렉트 종료) 또는 입력 오류: 무한 루프 방지
            std::cout << std::endl;
            break;
        }

        if (command == "exit" || command == "quit")
        {
            std::cout << "종료합니다." << std::endl;
            break;
        }
        else if (command == "help" || command == "?")
        {
            printHelp();
        }
        else if (command == "new")
        {
            handleNewFile();
        }
        else if (command == "open")
        {
            handleOpenFile();
        }
        else if (command == "save")
        {
            handleSaveFile();
        }
        else if (command == "add")
        {
            handleAddTransaction();
        }
        else if (command == "list")
        {
            handleListTransactions();
        }
        else if (command == "budget-add")
        {
            handleAddBudget();
        }
        else if (command == "budget-list")
        {
            handleListBudgets();
        }
        else
        {
            std::cout << "알 수 없는 명령어입니다. 'help'를 입력하세요." << std::endl;
        }
    }
}

// 명령어 처리 메서드
void FMLCLIView::printHelp()
{
    std::cout << "\n=== MissionLedger CLI ===" << std::endl;
    std::cout << "명령어:" << std::endl;
    std::cout << "  help         - 도움말 표시" << std::endl;
    std::cout << "  new [파일명] - 새 파일 생성" << std::endl;
    std::cout << "  open [파일명] - 파일 열기" << std::endl;
    std::cout << "  save         - 현재 파일 저장" << std::endl;
    std::cout << "  add          - 트랜잭션 추가 (대화형)" << std::endl;
    std::cout << "  list         - 모든 트랜잭션 표시" << std::endl;
    std::cout << "  budget-add   - 예산 추가 (대화형)" << std::endl;
    std::cout << "  budget-list  - 모든 예산 표시" << std::endl;
    std::cout << "  exit         - 종료" << std::endl;
    std::cout << "========================\n" << std::endl;
}

void FMLCLIView::handleNewFile()
{
    std::cout << "새 파일 생성" << std::endl;
    auto controller = FMLMVCHolder::GetInstance().GetController();
    controller->NewFile();
}

void FMLCLIView::handleOpenFile()
{
    std::string filename;
    std::cin >> filename;
    std::cout << "파일 열기: " << filename << std::endl;
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller->OpenFile(filename))
    {
        std::cout << "오류: 파일을 열 수 없습니다." << std::endl;
    }
}

void FMLCLIView::handleSaveFile()
{
    std::cout << "파일 저장 중..." << std::endl;
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (controller->SaveFile())
    {
        std::cout << "저장 완료" << std::endl;
    }
    else
    {
        std::cout << "오류: 파일을 저장할 수 없습니다." << std::endl;
    }
}

void FMLCLIView::handleAddTransaction()
{
    FMLTransactionData data;
    std::string typeStr;

    std::cout << "\n=== 트랜잭션 추가 ===" << std::endl;

    // 유형 입력
    std::cout << "유형 (income/expense): ";
    std::cin >> typeStr;
    if (typeStr == "income" || typeStr == "i")
    {
        data.Type = E_MLTransactionType::Income;
    }
    else if (typeStr == "expense" || typeStr == "e")
    {
        data.Type = E_MLTransactionType::Expense;
    }
    else
    {
        std::cout << "잘못된 유형입니다." << std::endl;
        return;
    }

    // 나머지 입력
    std::cin.ignore(); // 버퍼 비우기

    std::cout << "카테고리: ";
    std::getline(std::cin, data.Category);

    std::cout << "항목: ";
    std::getline(std::cin, data.Item);

    std::cout << "설명: ";
    std::getline(std::cin, data.Description);

    std::cout << "금액: ";
    if (!(std::cin >> data.Amount))
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "잘못된 입력입니다. 숫자를 입력해주세요." << std::endl;
        return;
    }

    std::cin.ignore(); // 버퍼 비우기

    std::cout << "영수증 번호 (선택): ";
    std::getline(std::cin, data.ReceiptNumber);

    // 환율 적용 여부
    std::cout << "환율 적용 (y/n, 기본: n): ";
    std::string useExchangeStr;
    std::getline(std::cin, useExchangeStr);

    if (useExchangeStr == "y" || useExchangeStr == "Y")
    {
        data.UseExchangeRate = true;

        std::cout << "통화 코드 (예: USD, EUR, JPY): ";
        std::getline(std::cin, data.Currency);

        std::cout << "외화 금액: ";
        if (!(std::cin >> data.OriginalAmount))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "잘못된 입력입니다." << std::endl;
            return;
        }

        std::cout << "환율 (원/외화): ";
        if (!(std::cin >> data.ExchangeRate))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "잘못된 입력입니다." << std::endl;
            return;
        }

        // 금액 자동 계산 (원 단위 반올림)
        if (!FMLCurrency::ConvertToKRW(data.OriginalAmount, data.ExchangeRate, data.Amount))
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "원화 환산 금액이 범위를 벗어났습니다." << std::endl;
            return;
        }
        std::cout << "→ 원화 환산 금액: " << FMLFormat::FormatAmount(data.Amount) << "원" << std::endl;

        std::cin.ignore(); // 버퍼 비우기
    }
    else
    {
        data.UseExchangeRate = false;
    }

    // 날짜는 자동으로 현재 시간으로 설정됨 (빈 문자열)
    data.DateTime = "";

    AddTransaction(data);

    std::cout << "트랜잭션이 추가되었습니다." << std::endl;
}

void FMLCLIView::handleListTransactions()
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    auto transactions = controller->GetAllTransactionData();
    DisplayTransactions(transactions);
}

void FMLCLIView::handleAddBudget()
{
    FMLBudgetData data;
    std::string typeStr;

    std::cout << "\n=== 예산 추가 ===" << std::endl;

    // 유형 입력
    std::cout << "유형 (income/expense): ";
    std::cin >> typeStr;
    if (typeStr == "income" || typeStr == "i")
    {
        data.Type = E_MLTransactionType::Income;
    }
    else if (typeStr == "expense" || typeStr == "e")
    {
        data.Type = E_MLTransactionType::Expense;
    }
    else
    {
        std::cout << "잘못된 유형입니다." << std::endl;
        return;
    }

    // 나머지 입력
    std::cin.ignore(); // 버퍼 비우기

    std::cout << "카테고리: ";
    std::getline(std::cin, data.Category);

    std::cout << "항목 (선택): ";
    std::getline(std::cin, data.Item);

    std::cout << "예산 금액: ";
    if (!(std::cin >> data.BudgetAmount))
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "잘못된 입력입니다. 숫자를 입력해주세요." << std::endl;
        return;
    }

    // 실제 지출액은 0으로 초기화
    data.ActualAmount = 0;

    AddBudget(data);

    std::cout << "예산이 추가되었습니다." << std::endl;
}

void FMLCLIView::handleListBudgets()
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    auto budgets = controller->GetAllBudgets();
    DisplayBudgets(budgets);
}

// IMLView implementation
void FMLCLIView::AddTransaction(const FMLTransactionData& data)
{
    // Controller를 통해 추가
    auto controller = FMLMVCHolder::GetInstance().GetController();
    controller->AddTransaction(data);
}

void FMLCLIView::DisplayTransaction(const FMLTransactionData& data)
{
    printSeparator();
    printTransaction(data, true);
    printSeparator();
}

void FMLCLIView::DisplayTransactions(const std::vector<FMLTransactionData>& data)
{
    printSeparator();
    std::cout << "총 " << data.size() << "개의 트랜잭션" << std::endl;
    printSeparator();

    if (data.empty())
    {
        std::cout << "트랜잭션이 없습니다." << std::endl;
        printSeparator();
        return;
    }

    // 헤더 출력
    std::cout
        << FMLFormat::FitColumn("ID", 6)
        << FMLFormat::FitColumn("유형", 8)
        << FMLFormat::FitColumn("카테고리", 15)
        << FMLFormat::FitColumn("항목", 20)
        << FMLFormat::FitColumn("금액", 15)
        << FMLFormat::FitColumn("날짜/시간", 20)
        << std::endl;
    printSeparator();

    // 데이터 출력
    for (const auto& transaction : data)
    {
        printTransaction(transaction, false);
    }

    printSeparator();

    // 요약 정보 계산
    int64_t totalIncome = 0;
    int64_t totalExpense = 0;
    for (const auto& transaction : data)
    {
        if (transaction.Type == E_MLTransactionType::Income)
        {
            totalIncome += transaction.Amount;
        }
        else if (transaction.Type == E_MLTransactionType::Expense)
        {
            totalExpense += transaction.Amount;
        }
    }

    std::cout << "총 수입: " << FMLFormat::FormatAmount(totalIncome) + "원" << std::endl;
    std::cout << "총 지출: " << FMLFormat::FormatAmount(totalExpense) + "원" << std::endl;
    std::cout << "잔액: " << FMLFormat::FormatAmount(totalIncome - totalExpense) + "원" << std::endl;
    printSeparator();
}

// IMLModelObserver implementation
void FMLCLIView::OnTransactionAdded(const FMLTransactionData& transactionData)
{
    LastAddedTransactionId = transactionData.TransactionId;
    if (!SilentMode)
    {
        std::cout << "[이벤트] 트랜잭션 추가됨 - ID: " << transactionData.TransactionId << std::endl;
    }
}

void FMLCLIView::OnTransactionDeleted(const int transactionId)
{
    if (!SilentMode)
    {
        std::cout << "[이벤트] 트랜잭션 삭제됨 - ID: " << transactionId << std::endl;
    }
}

void FMLCLIView::OnTransactionUpdated(const FMLTransactionData& transactionData)
{
    if (!SilentMode)
    {
        std::cout << "[이벤트] 트랜잭션 업데이트됨 - ID: " << transactionData.TransactionId << std::endl;
    }
}

void FMLCLIView::OnTransactionsCleared()
{
    if (!SilentMode)
    {
        std::cout << "[이벤트] 모든 트랜잭션 삭제됨" << std::endl;
    }
}

void FMLCLIView::OnDataLoaded()
{
    if (!SilentMode)
    {
        std::cout << "[이벤트] 데이터 로드 완료" << std::endl;
    }
}

void FMLCLIView::OnDataSaved()
{
    if (!SilentMode)
    {
        std::cout << "[이벤트] 데이터 저장 완료" << std::endl;
    }
}

void FMLCLIView::OnBudgetAdded(const FMLBudgetData& budgetData)
{
    LastAddedBudgetId = budgetData.BudgetId;
    if (!SilentMode)
    {
        std::cout << "[이벤트] 예산 추가됨 - 카테고리: " << budgetData.Category << std::endl;
    }
}

void FMLCLIView::OnBudgetDeleted(const int budgetId)
{
    if (!SilentMode)
    {
        std::cout << "[이벤트] 예산 삭제됨 - ID: " << budgetId << std::endl;
    }
}

void FMLCLIView::OnBudgetUpdated(const FMLBudgetData& budgetData)
{
    if (!SilentMode)
    {
        std::cout << "[이벤트] 예산 업데이트됨 - 카테고리: " << budgetData.Category << std::endl;
    }
}

void FMLCLIView::OnBudgetCleared()
{
    if (!SilentMode)
    {
        std::cout << "[이벤트] 모든 예산 삭제됨" << std::endl;
    }
}

// Private helper methods
void FMLCLIView::printTransaction(const FMLTransactionData& data, bool showHeader)
{
    if (showHeader)
    {
        std::cout << "ID: " << data.TransactionId << std::endl;
        std::cout << "유형: " << getTypeString(data.Type) << std::endl;
        std::cout << "카테고리: " << data.Category << std::endl;
        std::cout << "항목: " << data.Item << std::endl;
        std::cout << "설명: " << data.Description << std::endl;
        std::cout << "금액: " << FMLFormat::FormatAmount(data.Amount) + "원" << std::endl;

        // 환율 정보 표시
        if (data.UseExchangeRate)
        {
            std::cout << "  ↳ 환율 적용: " << FMLFormat::FormatDecimal(data.OriginalAmount, 2) << " " << data.Currency
                      << " × " << FMLFormat::FormatDecimal(data.ExchangeRate, 4)
                      << " = " << FMLFormat::FormatAmount(data.Amount) << "원" << std::endl;
        }

        std::cout << "영수증 번호: " << data.ReceiptNumber << std::endl;
        std::cout << "날짜/시간: " << data.DateTime << std::endl;
    }
    else
    {
        // Widths in console columns (Hangul = 2), cut at character boundaries
        std::cout
            << FMLFormat::FitColumn(std::to_string(data.TransactionId), 6)
            << FMLFormat::FitColumn(getTypeString(data.Type), 8)
            << FMLFormat::FitColumn(data.Category, 15)
            << FMLFormat::FitColumn(data.Item, 20)
            << FMLFormat::FitColumn(FMLFormat::FormatAmount(data.Amount) + "원", 15)
            << FMLFormat::FitColumn(data.DateTime, 20)
            << std::endl;
    }
}

void FMLCLIView::printSeparator()
{
    std::cout << std::string(80, '-') << std::endl;
}

std::string FMLCLIView::getTypeString(E_MLTransactionType type)
{
    switch (type)
    {
    case E_MLTransactionType::Income:
        return "수입";
    case E_MLTransactionType::Expense:
        return "지출";
    default:
        return "알 수 없음";
    }
}

std::string FMLCLIView::getBudgetTypeString(E_MLTransactionType type)
{
    switch (type)
    {
    case E_MLTransactionType::Income:
        return "수입";
    case E_MLTransactionType::Expense:
        return "지출";
    default:
        return "알 수 없음";
    }
}

void FMLCLIView::printBudget(const FMLBudgetData& data, bool showHeader)
{
    if (showHeader)
    {
        std::cout << "ID: " << data.BudgetId << std::endl;
        std::cout << "유형: " << getBudgetTypeString(data.Type) << std::endl;
        std::cout << "카테고리: " << data.Category << std::endl;
        std::cout << "항목: " << data.Item << std::endl;
        std::cout << "예산 금액: " << FMLFormat::FormatAmount(data.BudgetAmount) + "원" << std::endl;
    }
    else
    {
        // Widths in console columns (Hangul = 2), cut at character boundaries
        std::cout
            << FMLFormat::FitColumn(std::to_string(data.BudgetId), 6)
            << FMLFormat::FitColumn(getBudgetTypeString(data.Type), 8)
            << FMLFormat::FitColumn(data.Category, 15)
            << FMLFormat::FitColumn(data.Item, 20)
            << FMLFormat::FitColumn(FMLFormat::FormatAmount(data.BudgetAmount) + "원", 15)
            << std::endl;
    }
}

// Budget IMLView implementation
void FMLCLIView::AddBudget(const FMLBudgetData& data)
{
    // Controller를 통해 추가
    auto controller = FMLMVCHolder::GetInstance().GetController();
    controller->AddBudget(data);
}

void FMLCLIView::DisplayBudget(const FMLBudgetData& data)
{
    printSeparator();
    printBudget(data, true);
    printSeparator();
}

void FMLCLIView::DisplayBudgets(const std::vector<FMLBudgetData>& data)
{
    printSeparator();
    std::cout << "총 " << data.size() << "개의 예산" << std::endl;
    printSeparator();

    if (data.empty())
    {
        std::cout << "예산이 없습니다." << std::endl;
        printSeparator();
        return;
    }

    // 헤더 출력
    std::cout
        << FMLFormat::FitColumn("ID", 6)
        << FMLFormat::FitColumn("유형", 8)
        << FMLFormat::FitColumn("카테고리", 15)
        << FMLFormat::FitColumn("항목", 20)
        << FMLFormat::FitColumn("예산액", 15)
        << std::endl;
    printSeparator();

    // 데이터 출력
    for (const auto& budget : data)
    {
        printBudget(budget, false);
    }

    printSeparator();

    // 요약 정보 계산
    int64_t totalIncomeBudget = 0;
    int64_t totalExpenseBudget = 0;

    for (const auto& budget : data)
    {
        if (budget.Type == E_MLTransactionType::Income)
        {
            totalIncomeBudget += budget.BudgetAmount;
        }
        else if (budget.Type == E_MLTransactionType::Expense)
        {
            totalExpenseBudget += budget.BudgetAmount;
        }
    }

    std::cout << "수입 예산: " << FMLFormat::FormatAmount(totalIncomeBudget) + "원" << std::endl;
    std::cout << "지출 예산: " << FMLFormat::FormatAmount(totalExpenseBudget) + "원" << std::endl;
    printSeparator();
}
