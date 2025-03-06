#include <wx/wx.h>
#include <wx/listctrl.h>

class MyFrame : public wxFrame {
public:
  MyFrame() : wxFrame(nullptr, wxID_ANY, "wxListCtrl 예제", wxDefaultPosition, wxSize(400, 300)) {
    wxPanel* panel = new wxPanel(this);
    wxListCtrl* listCtrl = new wxListCtrl(panel, wxID_ANY, wxPoint(10, 10), wxSize(380, 250),
                                          wxLC_REPORT);  // 리스트 형식으로 표시

    // 컬럼(헤더) 추가
    listCtrl->InsertColumn(0, "이름", wxLIST_FORMAT_LEFT, 100);
    listCtrl->InsertColumn(1, "나이", wxLIST_FORMAT_LEFT, 50);
    listCtrl->InsertColumn(2, "직업", wxLIST_FORMAT_LEFT, 100);

    // 데이터 추가
    long index = listCtrl->InsertItem(0, "홍길동");  // 첫 번째 열 (이름)
    listCtrl->SetItem(index, 1, "25");  // 두 번째 열 (나이)
    listCtrl->SetItem(index, 2, "개발자");  // 세 번째 열 (직업)

    index = listCtrl->InsertItem(1, "이순신");
    listCtrl->SetItem(index, 1, "45");
    listCtrl->SetItem(index, 2, "군인");
  }
};

class MyApp : public wxApp {
public:
  virtual bool OnInit() {
    MyFrame* frame = new MyFrame();
    frame->Show();
    return true;
  }
};

wxIMPLEMENT_APP(MyApp);
