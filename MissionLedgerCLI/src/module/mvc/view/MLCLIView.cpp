#include "MLCLIView.h"
#include "MLDefine.h"
#include <iomanip>
#include <sstream>

FMLCLIView::FMLCLIView()
{
    std::cout << "[CLI View] 초기화 완료" << std::endl;
}

FMLCLIView::~FMLCLIView()
{
    std::cout << "[CLI View] 종료" << std::endl;
}

// IMLView implementation
void FMLCLIView::AddTransaction(const FMLTransactionData& data)
{
    // CLI에서는 직접 입력받지 않고, main에서 컨트롤러를 통해 추가합니다.
    // 이 메서드는 사용되지 않을 수 있습니다.
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
    std::cout << std::left
        << std::setw(6) << "ID"
        << std::setw(8) << "유형"
        << std::setw(15) << "카테고리"
        << std::setw(20) << "항목"
        << std::setw(15) << "금액"
        << std::setw(20) << "날짜/시간"
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

    std::cout << "총 수입: " << formatAmount(totalIncome) << std::endl;
    std::cout << "총 지출: " << formatAmount(totalExpense) << std::endl;
    std::cout << "잔액: " << formatAmount(totalIncome - totalExpense) << std::endl;
    printSeparator();
}

// IMLModelObserver implementation
void FMLCLIView::OnTransactionAdded(const FMLTransactionData& transactionData)
{
    std::cout << "[이벤트] 트랜잭션 추가됨 - ID: " << transactionData.TransactionId << std::endl;
}

void FMLCLIView::OnTransactionRemoved(int transactionId)
{
    std::cout << "[이벤트] 트랜잭션 삭제됨 - ID: " << transactionId << std::endl;
}

void FMLCLIView::OnTransactionUpdated(const FMLTransactionData& transactionData)
{
    std::cout << "[이벤트] 트랜잭션 업데이트됨 - ID: " << transactionData.TransactionId << std::endl;
}

void FMLCLIView::OnTransactionsCleared()
{
    std::cout << "[이벤트] 모든 트랜잭션 삭제됨" << std::endl;
}

void FMLCLIView::OnDataLoaded()
{
    std::cout << "[이벤트] 데이터 로드 완료" << std::endl;
}

void FMLCLIView::OnDataSaved()
{
    std::cout << "[이벤트] 데이터 저장 완료" << std::endl;
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
        std::cout << "금액: " << formatAmount(data.Amount) << std::endl;
        std::cout << "영수증 번호: " << data.ReceiptNumber << std::endl;
        std::cout << "날짜/시간: " << data.DateTime << std::endl;
    }
    else
    {
        std::cout << std::left
            << std::setw(6) << data.TransactionId
            << std::setw(8) << getTypeString(data.Type)
            << std::setw(15) << data.Category.substr(0, 14)
            << std::setw(20) << data.Item.substr(0, 19)
            << std::setw(15) << formatAmount(data.Amount)
            << std::setw(20) << data.DateTime.substr(0, 19)
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

std::string FMLCLIView::formatAmount(int64_t amount)
{
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << amount << "원";
    return ss.str();
}
