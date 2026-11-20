#include <wx/wx.h>
#include "module/common/holder/MLMVCHolder.h"
#include "module/mvc/model/MLModel.h"
#include "module/mvc/controller/MLController.h"
#include "module/mvc/view/MLMainFrame.h"
#include "module/storage/MLSQLiteStorage.h"
#include <fstream>
#include <wx/stdpaths.h>

class MissionLedger : public wxApp {
public:
  bool OnInit() override {
#ifdef _DEBUG
    // Debug 빌드에서만 로그 파일 설정
    wxString logPath = wxStandardPaths::Get().GetUserDataDir() + wxFILE_SEP_PATH + "MissionLedger_debug.log";
    if (!wxDirExists(wxStandardPaths::Get().GetUserDataDir())) {
      wxMkdir(wxStandardPaths::Get().GetUserDataDir());
    }

    freopen(logPath.mb_str(), "w", stdout);
    freopen(logPath.mb_str(), "a", stderr);

    std::cout << "=== MissionLedger Debug Log ===" << std::endl;
    std::cout << "Log file: " << logPath.ToStdString() << std::endl;
    std::cout << "Startup time: " << wxDateTime::Now().FormatISOCombined() << std::endl;
    std::cout << std::endl;
#endif
    // MVC 컴포넌트 생성
    auto& mvcHolder = FMLMVCHolder::GetInstance();
    std::shared_ptr<FMLModel> model = std::make_shared<FMLModel>();
    std::shared_ptr<FMLController> controller = std::make_shared<FMLController>();

    // Storage Provider 생성 및 설정
    auto storageProvider = std::make_shared<FMLSQLiteStorage>();
    model->SetStorageProvider(storageProvider);

    // Frame 생성 (wxWidgets는 new로 생성, wxWidgets가 메모리 관리)
    wxMLMainFrame* frame = new wxMLMainFrame();
    MainFrame = frame;

    // MVCHolder에 등록
    mvcHolder.SetModel(model);
    mvcHolder.SetController(controller);
    mvcHolder.SetView(std::shared_ptr<IMLView>(frame, [](IMLView*){})); // wxWidgets 객체는 삭제 안 함

    // Observer 등록 (frame이 IMLModelObserver를 구현)
    model->AddObserver(std::shared_ptr<IMLModelObserver>(frame, [](IMLModelObserver*){}));

    frame->Show();

#ifdef _DEBUG
    std::cout << "Frame shown successfully" << std::endl;
    std::cout << "Command line arguments: argc=" << argc << std::endl;
    for (int i = 0; i < argc; i++) {
      std::cout << "  argv[" << i << "] = " << argv[i].ToStdString() << std::endl;
    }
#endif

    // 명령줄 인수로 파일이 전달된 경우 열기 (Windows: .ml 더블클릭, 연결 프로그램)
    if (argc > 1) {
      wxLogDebug("Received command-line argument: %s", argv[1]);
      frame->OpenFileFromShell(argv[1]);
    }

#ifdef _DEBUG
    std::cout << "OnInit completed successfully" << std::endl;
    std::cout << std::flush;
#endif
    return true;
  }

#ifdef __WXMAC__
  // macOS: Finder에서 .ml 더블클릭 / "다음으로 열기" / Dock 아이콘에 끌어놓기.
  // 파일은 명령줄 인수가 아니라 이 함수로 온다 (실행 중일 때도, 실행하면서 열 때도). 한 번에 하나만 연다
  void MacOpenFiles(const wxArrayString& fileNames) override {
    if (MainFrame && !fileNames.IsEmpty()) {
      MainFrame->OpenFileFromShell(fileNames[0]);
    }
  }
#endif

  int OnExit() override {
#ifdef _DEBUG
    std::cout << "=== Application Exit ===" << std::endl;
    std::cout << std::flush;
    fflush(stdout);
#endif
    return wxApp::OnExit();
  }

private:
  wxMLMainFrame* MainFrame = nullptr;   // wxWidgets가 소유 (삭제하지 않음)
};

wxIMPLEMENT_APP(MissionLedger);
