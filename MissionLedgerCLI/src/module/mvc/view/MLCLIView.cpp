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
