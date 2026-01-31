#include "module/common/holder/MLMVCHolder.h"
#include "module/mvc/model/MLModel.h"
#include "module/mvc/controller/MLController.h"
#include "src/module/mvc/view/MLCLIView.h"
#include "module/storage/MLSQLiteStorage.h"
#include "MLDefine.h"
#include <iostream>
#include <string>
#include <memory>
#include <limits>

#ifdef _WIN32
#define NOMINMAX  // Windows.h의 min/max 매크로 비활성화
#include <windows.h>
#endif

void printHelp()
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

void addTransactionInteractive(std::shared_ptr<FMLController> controller)
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

    // 날짜는 자동으로 현재 시간으로 설정됨 (빈 문자열)
    data.DateTime = "";

    // Controller를 통해 추가
    controller->AddTransaction(data);

    std::cout << "트랜잭션이 추가되었습니다." << std::endl;
}

int main(int argc, char* argv[])
{
    // UTF-8 콘솔 설정 (Windows)
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    #endif

    std::cout << "MissionLedger CLI 시작..." << std::endl;

    // MVC 컴포넌트 생성
    auto& mvcHolder = FMLMVCHolder::GetInstance();
    std::shared_ptr<FMLModel> model = std::make_shared<FMLModel>();
    std::shared_ptr<FMLController> controller = std::make_shared<FMLController>();
    std::shared_ptr<FMLCLIView> view = std::make_shared<FMLCLIView>();

    // Storage Provider 생성 및 설정
    auto storageProvider = std::make_shared<FMLSQLiteStorage>();
    model->SetStorageProvider(storageProvider);

    // MVCHolder에 등록
    mvcHolder.SetModel(model);
    mvcHolder.SetController(controller);
    mvcHolder.SetView(view);

    // Observer 등록
    model->AddObserver(view);

    // 명령줄 인수로 파일이 전달된 경우 열기
    if (argc > 1)
    {
        std::string filePath = argv[1];
        if (filePath.find(".ml") != std::string::npos)
        {
            std::cout << "파일 열기: " << filePath << std::endl;
            if (!controller->OpenFile(filePath))
            {
                std::cout << "경고: 파일을 열 수 없습니다." << std::endl;
            }
        }
    }

    printHelp();

    // 명령어 루프
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
            std::cout << "새 파일 생성" << std::endl;
            controller->NewFile();
        }
        else if (command == "open")
        {
            std::string filename;
            std::cin >> filename;
            std::cout << "파일 열기: " << filename << std::endl;
            if (!controller->OpenFile(filename))
            {
                std::cout << "오류: 파일을 열 수 없습니다." << std::endl;
            }
        }
        else if (command == "save")
        {
            std::cout << "파일 저장 중..." << std::endl;
            if (controller->SaveFile())
            {
                std::cout << "저장 완료" << std::endl;
            }
            else
            {
                std::cout << "오류: 파일을 저장할 수 없습니다." << std::endl;
            }
        }
        else if (command == "add")
        {
            addTransactionInteractive(controller);
        }
        else if (command == "list")
        {
            auto transactions = controller->GetAllTransactionData();
            view->DisplayTransactions(transactions);
        }
        else
        {
            std::cout << "알 수 없는 명령어입니다. 'help'를 입력하세요." << std::endl;
        }
    }

    return 0;
}
