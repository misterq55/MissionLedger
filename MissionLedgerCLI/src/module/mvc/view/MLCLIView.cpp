#include "MLCLIView.h"
#include "MLDefine.h"
#include "module/common/holder/MLMVCHolder.h"
#include "interface/IMLController.h"
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

// 명령어 모드 entry point
int FMLCLIView::RunWithArgs(int argc, char* argv[])
{
    SilentMode = true;

    if (argc < 2)
    {
        printUsage();
        return 1;
    }

    std::string command = argv[1];

    if (command == "help" || command == "--help" || command == "-h")
    {
        return cmdHelp();
    }
    else if (command == "list")
    {
        std::string filePath;
        if (argc >= 3)
        {
            filePath = argv[2];
        }
        return cmdList(filePath);
    }
    else if (command == "add")
    {
        auto options = parseOptions(argc, argv, 2);
        return cmdAdd(options);
    }
    else if (command == "open")
    {
        if (argc < 3)
        {
            std::cerr << "오류: 파일 경로가 필요합니다." << std::endl;
            std::cerr << "사용법: MissionLedgerCLI open <파일경로>" << std::endl;
            return 1;
        }
        return cmdOpen(argv[2]);
    }
    else if (command == "new")
    {
        std::string filePath;
        if (argc >= 3)
        {
            filePath = argv[2];
        }
        return cmdNew(filePath);
    }
    else
    {
        std::cerr << "알 수 없는 명령어: " << command << std::endl;
        printUsage();
        return 1;
    }
}

// 명령줄 인수 파싱
std::map<std::string, std::string> FMLCLIView::parseOptions(int argc, char* argv[], int startIndex)
{
    std::map<std::string, std::string> options;

    for (int i = startIndex; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg.rfind("--", 0) == 0 && i + 1 < argc)
        {
            std::string key = arg.substr(2);  // "--" 제거
            std::string value = argv[++i];
            options[key] = value;
        }
        else if (arg.rfind("-", 0) == 0 && arg.length() == 2 && i + 1 < argc)
        {
            std::string key = arg.substr(1);  // "-" 제거
            std::string value = argv[++i];
            options[key] = value;
        }
    }

    return options;
}

void FMLCLIView::printUsage()
{
    std::cout << "사용법: MissionLedgerCLI <명령어> [옵션]\n" << std::endl;
    std::cout << "명령어:" << std::endl;
    std::cout << "  help                    도움말 표시" << std::endl;
    std::cout << "  list [파일]             트랜잭션 목록 표시" << std::endl;
    std::cout << "  add [옵션]              트랜잭션 추가" << std::endl;
    std::cout << "  open <파일>             파일 열기 (대화형 모드 진입)" << std::endl;
    std::cout << "  new [파일]              새 파일 생성" << std::endl;
    std::cout << "\nadd 명령어 옵션:" << std::endl;
    std::cout << "  --file <파일>           대상 파일 (필수)" << std::endl;
    std::cout << "  --type <income|expense> 거래 유형 (필수)" << std::endl;
    std::cout << "  --amount <금액>         금액 (필수)" << std::endl;
    std::cout << "  --category <카테고리>   카테고리 (필수)" << std::endl;
    std::cout << "  --item <항목>           항목명" << std::endl;
    std::cout << "  --desc <설명>           설명" << std::endl;
    std::cout << "  --receipt <번호>        영수증 번호" << std::endl;
    std::cout << "  --date <날짜>           날짜 (YYYY-MM-DD 형식, 미지정시 현재 날짜)" << std::endl;
    std::cout << "\n환율 옵션 (선택, 모두 지정 시 환율 적용):" << std::endl;
    std::cout << "  --currency <통화>       통화 코드 (USD, EUR, JPY 등)" << std::endl;
    std::cout << "  --original <외화금액>   원래 외화 금액" << std::endl;
    std::cout << "  --rate <환율>           환율 (원/외화)" << std::endl;
    std::cout << "\n예시:" << std::endl;
    std::cout << "  MissionLedgerCLI list data.ml" << std::endl;
    std::cout << "  MissionLedgerCLI add --file data.ml --type income --amount 50000 --category \"급여\"" << std::endl;
    std::cout << "  MissionLedgerCLI add --file data.ml --type expense --amount 130000 --category \"숙박\" --currency USD --original 100 --rate 1300" << std::endl;
    std::cout << "  MissionLedgerCLI open data.ml" << std::endl;
    std::cout << "\n인수 없이 실행하면 대화형 모드로 진입합니다." << std::endl;
}

int FMLCLIView::cmdHelp()
{
    printUsage();
    return 0;
}

int FMLCLIView::cmdList(const std::string& filePath)
{
    auto controller = FMLMVCHolder::GetInstance().GetController();

    if (!filePath.empty())
    {
        if (!controller->OpenFile(filePath))
        {
            std::cerr << "오류: 파일을 열 수 없습니다: " << filePath << std::endl;
            return 1;
        }
    }

    auto transactions = controller->GetAllTransactionData();
    DisplayTransactions(transactions);
    return 0;
}

int FMLCLIView::cmdAdd(const std::map<std::string, std::string>& options)
{
    // 필수 옵션 확인
    auto fileIt = options.find("file");
    auto typeIt = options.find("type");
    auto amountIt = options.find("amount");
    auto categoryIt = options.find("category");

    if (fileIt == options.end())
    {
        std::cerr << "오류: --file 옵션이 필요합니다." << std::endl;
        return 1;
    }
    if (typeIt == options.end())
    {
        std::cerr << "오류: --type 옵션이 필요합니다. (income 또는 expense)" << std::endl;
        return 1;
    }
    if (amountIt == options.end())
    {
        std::cerr << "오류: --amount 옵션이 필요합니다." << std::endl;
        return 1;
    }
    if (categoryIt == options.end())
    {
        std::cerr << "오류: --category 옵션이 필요합니다." << std::endl;
        return 1;
    }

    auto controller = FMLMVCHolder::GetInstance().GetController();

    // 파일 열기
    if (!controller->OpenFile(fileIt->second))
    {
        std::cerr << "오류: 파일을 열 수 없습니다: " << fileIt->second << std::endl;
        return 1;
    }

    // 트랜잭션 데이터 구성
    FMLTransactionData data;

    // 유형 파싱
    std::string typeStr = typeIt->second;
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
        std::cerr << "오류: 잘못된 유형입니다. (income 또는 expense)" << std::endl;
        return 1;
    }

    // 금액 파싱
    try
    {
        data.Amount = std::stoll(amountIt->second);
    }
    catch (const std::exception&)
    {
        std::cerr << "오류: 잘못된 금액입니다." << std::endl;
        return 1;
    }

    // 카테고리
    data.Category = categoryIt->second;

    // 선택적 옵션
    auto itemIt = options.find("item");
    if (itemIt != options.end())
    {
        data.Item = itemIt->second;
    }

    auto descIt = options.find("desc");
    if (descIt != options.end())
    {
        data.Description = descIt->second;
    }

    auto receiptIt = options.find("receipt");
    if (receiptIt != options.end())
    {
        data.ReceiptNumber = receiptIt->second;
    }

    // 날짜 옵션 (미지정시 빈 문자열 -> Model에서 현재 날짜로 설정)
    auto dateIt = options.find("date");
    if (dateIt != options.end())
    {
        data.DateTime = dateIt->second;
    }
    else
    {
        data.DateTime = "";
    }

    // 환율 옵션 (선택적, 모두 지정 시 환율 적용)
    auto currencyIt = options.find("currency");
    auto originalIt = options.find("original");
    auto rateIt = options.find("rate");

    if (currencyIt != options.end() && originalIt != options.end() && rateIt != options.end())
    {
        data.UseExchangeRate = true;
        data.Currency = currencyIt->second;

        try
        {
            data.OriginalAmount = std::stod(originalIt->second);
            data.ExchangeRate = std::stod(rateIt->second);

            // 금액 자동 계산 (환율 적용)
            data.Amount = static_cast<int64_t>(data.OriginalAmount * data.ExchangeRate);
        }
        catch (const std::exception&)
        {
            std::cerr << "오류: 잘못된 환율 정보입니다." << std::endl;
            return 1;
        }
    }
    else
    {
        data.UseExchangeRate = false;
    }

    // 트랜잭션 추가
    AddTransaction(data);

    // 저장
    if (controller->SaveFile())
    {
        std::cout << "트랜잭션이 추가되었습니다." << std::endl;
        return 0;
    }
    else
    {
        std::cerr << "오류: 파일 저장에 실패했습니다." << std::endl;
        return 1;
    }
}

int FMLCLIView::cmdOpen(const std::string& filePath)
{
    auto controller = FMLMVCHolder::GetInstance().GetController();

    if (!controller->OpenFile(filePath))
    {
        std::cerr << "오류: 파일을 열 수 없습니다: " << filePath << std::endl;
        return 1;
    }

    std::cout << "파일 열림: " << filePath << std::endl;

    // 대화형 모드로 전환
    SilentMode = false;
    Run();
    return 0;
}

int FMLCLIView::cmdNew(const std::string& filePath)
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    controller->NewFile();

    if (!filePath.empty())
    {
        if (controller->SaveFileAs(filePath))
        {
            std::cout << "새 파일 생성됨: " << filePath << std::endl;
            return 0;
        }
        else
        {
            std::cerr << "오류: 파일 저장에 실패했습니다." << std::endl;
            return 1;
        }
    }

    std::cout << "새 파일이 생성되었습니다." << std::endl;
    return 0;
}

// 대화형 모드 entry point
void FMLCLIView::Run()
{
    printHelp();

    std::string command;
    while (true)
    {
        std::cout << "> ";
        std::cin >> command;

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

        // 금액 자동 계산
        data.Amount = static_cast<int64_t>(data.OriginalAmount * data.ExchangeRate);
        std::cout << "→ 원화 환산 금액: " << data.Amount << "원" << std::endl;

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
    if (!SilentMode)
    {
        std::cout << "[이벤트] 트랜잭션 추가됨 - ID: " << transactionData.TransactionId << std::endl;
    }
}

void FMLCLIView::OnTransactionRemoved(int transactionId)
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

void FMLCLIView::OnBudgetAdded(const FMLCategoryBudgetData& budgetData)
{
    if (!SilentMode)
    {
        std::cout << "[이벤트] 예산 추가됨 - 카테고리: " << budgetData.Category << std::endl;
    }
}

void FMLCLIView::OnBudgetRemoved(const std::string& category)
{
    if (!SilentMode)
    {
        std::cout << "[이벤트] 예산 삭제됨 - 카테고리: " << category << std::endl;
    }
}

void FMLCLIView::OnBudgetUpdated(const FMLCategoryBudgetData& budgetData)
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
    // 안전한 substr 헬퍼 람다
    auto safeSub = [](const std::string& str, size_t len) -> std::string {
        return str.length() > len ? str.substr(0, len) : str;
    };

    if (showHeader)
    {
        std::cout << "ID: " << data.TransactionId << std::endl;
        std::cout << "유형: " << getTypeString(data.Type) << std::endl;
        std::cout << "카테고리: " << data.Category << std::endl;
        std::cout << "항목: " << data.Item << std::endl;
        std::cout << "설명: " << data.Description << std::endl;
        std::cout << "금액: " << formatAmount(data.Amount) << std::endl;

        // 환율 정보 표시
        if (data.UseExchangeRate)
        {
            std::cout << "  ↳ 환율 적용: " << data.OriginalAmount << " " << data.Currency
                      << " × " << data.ExchangeRate << " = " << data.Amount << "원" << std::endl;
        }

        std::cout << "영수증 번호: " << data.ReceiptNumber << std::endl;
        std::cout << "날짜/시간: " << data.DateTime << std::endl;
    }
    else
    {
        std::cout << std::left
            << std::setw(6) << data.TransactionId
            << std::setw(8) << getTypeString(data.Type)
            << std::setw(15) << safeSub(data.Category, 14)
            << std::setw(20) << safeSub(data.Item, 19)
            << std::setw(15) << formatAmount(data.Amount)
            << std::setw(20) << safeSub(data.DateTime, 19)
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
    try
    {
        ss.imbue(std::locale(""));
    }
    catch (const std::runtime_error&)
    {
        // 시스템 로케일을 사용할 수 없으면 기본 로케일 사용
    }
    ss << std::fixed << amount << "원";
    return ss.str();
}
