#pragma once

/**
 * @file MLMVCHolderExample.h
 * @brief CMLMVCHolder 사용 예시 및 가이드
 *
 * 이 파일은 CMLMVCHolder를 실제 프로젝트에서 어떻게 사용하는지 보여주는 예시입니다.
 */

#include "MLMVCHolder.h"

/**
 * @brief main.cpp에서 MVC 컴포넌트 초기화 예시
 *
 * @code
 * #include "module/common/holder/MLMVCHolder.h"
 * #include "module/mvc/model/MLModel.h"
 * #include "module/mvc/view/MLView.h"
 * #include "module/mvc/controller/MLController.h"
 *
 * class MyApp : public wxApp
 * {
 * public:
 *     virtual bool OnInit() override
 *     {
 *         // MVC 컴포넌트 생성
 *         auto model = std::make_shared<CMLModel>();
 *         auto view = std::make_shared<CMLView>();
 *         auto controller = std::make_shared<CMLController>();
 *
 *         // 홀더에 등록
 *         auto& holder = CMLMVCHolder::GetInstance();
 *         holder.SetModel(model);
 *         holder.SetView(view);
 *         holder.SetController(controller);
 *
 *         // Observer 연결
 *         model->AddObserver(view);
 *
 *         // 초기화 확인
 *         if (!holder.IsInitialized()) {
 *             wxMessageBox("MVC 컴포넌트 초기화 실패!", "오류", wxOK | wxICON_ERROR);
 *             return false;
 *         }
 *
 *         // 메인 프레임 표시
 *         MyFrame* frame = new MyFrame();
 *         frame->Show();
 *         return true;
 *     }
 *
 *     virtual int OnExit() override
 *     {
 *         // 애플리케이션 종료 시 홀더 해제
 *         CMLMVCHolder::DestroyInstance();
 *         return wxApp::OnExit();
 *     }
 * };
 * @endcode
 */

/**
 * @brief wxWidgets 윈도우에서 홀더 사용 예시
 *
 * @code
 * class MainFrame : public wxFrame
 * {
 * public:
 *     MainFrame() : wxFrame(nullptr, wxID_ANY, "MissionLedger")
 *     {
 *         // 메뉴 이벤트 핸들러 바인딩
 *         Bind(wxEVT_MENU, &MainFrame::OnMenuFileOpen, this, wxID_OPEN);
 *         Bind(wxEVT_MENU, &MainFrame::OnMenuFileSave, this, wxID_SAVE);
 *     }
 *
 * private:
 *     void OnMenuFileOpen(wxCommandEvent& event)
 *     {
 *         // 홀더를 통해 Controller 접근
 *         auto controller = CMLMVCHolder::GetInstance().GetController();
 *         if (controller) {
 *             // 파일 열기 로직...
 *         }
 *     }
 *
 *     void OnMenuFileSave(wxCommandEvent& event)
 *     {
 *         // 홀더를 통해 Model 접근
 *         auto model = CMLMVCHolder::GetInstance().GetModel();
 *         if (model) {
 *             model->Save();
 *         }
 *     }
 * };
 * @endcode
 */

/**
 * @brief 다이얼로그에서 홀더 사용 예시
 *
 * @code
 * class AddTransactionDialog : public wxDialog
 * {
 * public:
 *     AddTransactionDialog(wxWindow* parent)
 *         : wxDialog(parent, wxID_ANY, "거래 추가")
 *     {
 *         // UI 구성...
 *         Bind(wxEVT_BUTTON, &AddTransactionDialog::OnOK, this, wxID_OK);
 *     }
 *
 * private:
 *     void OnOK(wxCommandEvent& event)
 *     {
 *         // 사용자 입력 데이터 수집
 *         SMLTransactionData transactionData;
 *         // ... 데이터 설정
 *
 *         // 홀더를 통해 Controller 접근하여 거래 추가
 *         auto controller = CMLMVCHolder::GetInstance().GetController();
 *         if (controller) {
 *             controller->AddTransaction(transactionData);
 *         }
 *
 *         EndModal(wxID_OK);
 *     }
 * };
 * @endcode
 */

/**
 * @brief 테스트에서 홀더 사용 예시
 *
 * @code
 * #include "gtest/gtest.h"
 * #include "MLMVCHolder.h"
 * #include "MockMLModel.h"
 *
 * class MLMVCHolderTest : public ::testing::Test
 * {
 * protected:
 *     void SetUp() override
 *     {
 *         // 테스트용 Mock 객체 생성
 *         mockModel = std::make_shared<MockMLModel>();
 *
 *         // 홀더에 Mock 객체 주입
 *         auto& holder = CMLMVCHolder::GetInstance();
 *         holder.SetModel(mockModel);
 *     }
 *
 *     void TearDown() override
 *     {
 *         // 테스트 후 홀더 초기화
 *         CMLMVCHolder::GetInstance().Reset();
 *     }
 *
 *     std::shared_ptr<MockMLModel> mockModel;
 * };
 *
 * TEST_F(MLMVCHolderTest, TestModelAccess)
 * {
 *     auto model = CMLMVCHolder::GetInstance().GetModel();
 *     ASSERT_NE(model, nullptr);
 *     ASSERT_EQ(model, mockModel);
 * }
 * @endcode
 */