#include <wx/wx.h>
#include "module/common/holder/MLMVCHolder.h"
#include "module/mvc/model/MLModel.h"
#include "module/mvc/controller/MLController.h"
#include "module/mvc/view/MLMainFrame.h"
#include "module/storage/MLSQLiteStorage.h"

class MissionLedger : public wxApp {
public:
  virtual bool OnInit() {
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

    // 명령줄 인수로 파일이 전달된 경우 열기
    if (argc > 1) {
      wxString filePath = argv[1];
      if (wxFileExists(filePath) && filePath.EndsWith(".ml")) {
        model->OpenFile(filePath.ToStdString());
      }
    }

    return true;
  }
};

wxIMPLEMENT_APP(MissionLedger);
