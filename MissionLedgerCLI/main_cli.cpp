#include "module/common/holder/MLMVCHolder.h"
#include "module/mvc/model/MLModel.h"
#include "module/mvc/controller/MLController.h"
#include "src/module/mvc/view/MLCLIView.h"
#include "module/storage/MLSQLiteStorage.h"
#include <iostream>
#include <memory>
#include <vector>
#include <string>

#ifdef _WIN32
#define NOMINMAX  // Windows.h의 min/max 매크로 비활성화
#include <windows.h>
#include <shellapi.h>  // CommandLineToArgvW
#endif

// UTF-16 문자열을 UTF-8로 변환
#ifdef _WIN32
std::string WideToUtf8(const std::wstring& wide)
{
    if (wide.empty()) return std::string();
    int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), nullptr, 0, nullptr, nullptr);
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), &result[0], size, nullptr, nullptr);
    return result;
}
#endif

int main(int argc, char* argv[])
{
    // UTF-8 콘솔 설정 (Windows)
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // stdout 버퍼링 비활성화 (UTF-8 출력 안정화)
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    #endif

    // Windows에서 UTF-8 명령 인자 가져오기
    #ifdef _WIN32
    int wargc;
    LPWSTR* wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
    std::vector<std::string> utf8Args;
    std::vector<char*> utf8Argv;

    if (wargv)
    {
        for (int i = 0; i < wargc; ++i)
        {
            utf8Args.push_back(WideToUtf8(wargv[i]));
        }
        LocalFree(wargv);

        for (auto& arg : utf8Args)
        {
            utf8Argv.push_back(&arg[0]);
        }

        argc = wargc;
        argv = utf8Argv.data();
    }
    #endif

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

    // 실행 모드 결정
    if (argc > 1)
    {
        // 명령어 모드: 인수가 있으면 명령어로 처리
        return view->RunWithArgs(argc, argv);
    }
    else
    {
        // 대화형 모드: 인수가 없으면 명령어 루프 실행
        std::cout << "MissionLedger CLI 시작..." << std::endl;
        view->Run();
        return 0;
    }
}
