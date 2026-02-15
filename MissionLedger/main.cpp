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
  virtual bool OnInit() {
#ifdef _DEBUG
    // Debug 빌드에서만 로그 파일 설정
    wxString logPath = wxStandardPaths::Get().GetUserDataDir() + "\\MissionLedger_debug.log";
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

    // 명령줄 인수로 파일이 전달된 경우 열기
    if (argc > 1) {
      wxString filePath = argv[1];
      wxLogDebug("Received command-line argument: %s", filePath);

      if (!wxFileExists(filePath)) {
        wxMessageBox(
          wxString::Format(wxString::FromUTF8("파일을 찾을 수 없습니다:\n%s"), filePath),
          wxString::FromUTF8("파일 열기 실패"),
          wxOK | wxICON_ERROR
        );
        wxLogDebug("File does not exist: %s", filePath);
      } else if (!filePath.EndsWith(".ml")) {
        wxMessageBox(
          wxString::FromUTF8("MissionLedger 파일(.ml)이 아닙니다."),
          wxString::FromUTF8("파일 형식 오류"),
          wxOK | wxICON_ERROR
        );
        wxLogDebug("File does not have .ml extension");
      } else {
        // UTF-8로 명시적 변환 (한글 경로 지원)
        const std::string utf8Path = filePath.ToUTF8().data();
        const bool result = model->OpenFile(utf8Path);

        if (!result) {
#ifdef _DEBUG
          wxMessageBox(
            wxString::Format(
              wxString::FromUTF8("파일을 열 수 없습니다:\n%s\n\n로그 파일:\n%s"),
              filePath,
              wxStandardPaths::Get().GetUserDataDir() + "\\MissionLedger_debug.log"
            ),
            wxString::FromUTF8("파일 열기 실패"),
            wxOK | wxICON_ERROR
          );
#else
          wxMessageBox(
            wxString::Format(wxString::FromUTF8("파일을 열 수 없습니다:\n%s"), filePath),
            wxString::FromUTF8("파일 열기 실패"),
            wxOK | wxICON_ERROR
          );
#endif
        }
      }
    }

#ifdef _DEBUG
    std::cout << "OnInit completed successfully" << std::endl;
    std::cout << std::flush;
#endif
    return true;
  }

  virtual int OnExit() {
#ifdef _DEBUG
    std::cout << "=== Application Exit ===" << std::endl;
    std::cout << std::flush;
    fflush(stdout);
#endif
    return wxApp::OnExit();
  }
};

wxIMPLEMENT_APP(MissionLedger);
