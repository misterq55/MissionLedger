#include <wx/wx.h>

// wxWidgets 애플리케이션 클래스 정의
class MyApp : public wxApp {
public:
  virtual bool OnInit();
};

// 메인 윈도우 클래스 정의
class MyFrame : public wxFrame {
public:
  MyFrame() : wxFrame(nullptr, wxID_ANY, "Hello wxWidgets!") {
    wxPanel* panel = new wxPanel(this, wxID_ANY);
    wxButton* button = new wxButton(panel, wxID_ANY, "Click Me!", wxPoint(20, 20));

    // 버튼 클릭 이벤트 핸들러
    button->Bind(wxEVT_BUTTON, &MyFrame::OnButtonClick, this);
  }

private:
  void OnButtonClick(wxCommandEvent& event) {
    wxMessageBox("Hello, wxWidgets!", "Info", wxOK | wxICON_INFORMATION);
  }
};

// wxWidgets 애플리케이션 객체 생성
wxIMPLEMENT_APP(MyApp);

// 애플리케이션 초기화 함수
bool MyApp::OnInit() {
  MyFrame* frame = new MyFrame();
  frame->Show(true);
  return true;
}
