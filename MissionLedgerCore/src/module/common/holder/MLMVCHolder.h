#pragma once

#include <memory>

// Forward declarations
class IMLModel;
class IMLView;
class IMLController;

/**
 * @brief MVC 컴포넌트들의 전역 접근을 위한 홀더 클래스
 *
 * 싱글톤 패턴을 사용하여 애플리케이션 전체에서 Model, View, Controller에 접근 가능
 * wxWidgets의 여러 윈도우/다이얼로그에서 동일한 MVC 컴포넌트를 사용할 때 필요
 */
class FMLMVCHolder
{
private:
    static FMLMVCHolder* Instance;

    std::shared_ptr<IMLModel> Model;
    std::shared_ptr<IMLView> View;
    std::shared_ptr<IMLController> Controller;

    // 생성자를 private으로 하여 외부에서 직접 생성 불가
    FMLMVCHolder() = default;

public:
    /**
     * @brief 싱글톤 인스턴스 반환
     * @return FMLMVCHolder& 유일한 인스턴스의 참조
     */
    static FMLMVCHolder& GetInstance();

    /**
     * @brief 싱글톤 인스턴스 해제 (애플리케이션 종료 시 호출)
     */
    static void DestroyInstance();

    // 복사 생성자와 대입 연산자 삭제 (싱글톤 패턴)
    FMLMVCHolder(const FMLMVCHolder&) = delete;
    FMLMVCHolder& operator=(const FMLMVCHolder&) = delete;

    /**
     * @brief Model 컴포넌트 설정
     * @param model IMLModel 인터페이스를 구현한 객체
     */
    void SetModel(std::shared_ptr<IMLModel> model);

    /**
     * @brief View 컴포넌트 설정
     * @param view IMLView 인터페이스를 구현한 객체
     */
    void SetView(std::shared_ptr<IMLView> view);

    /**
     * @brief Controller 컴포넌트 설정
     * @param controller IMLController 인터페이스를 구현한 객체
     */
    void SetController(std::shared_ptr<IMLController> controller);

    /**
     * @brief Model 컴포넌트 반환
     * @return std::shared_ptr<IMLModel> Model 객체의 shared_ptr
     */
    std::shared_ptr<IMLModel> GetModel() const;

    /**
     * @brief View 컴포넌트 반환
     * @return std::shared_ptr<IMLView> View 객체의 shared_ptr
     */
    std::shared_ptr<IMLView> GetView() const;

    /**
     * @brief Controller 컴포넌트 반환
     * @return std::shared_ptr<IMLController> Controller 객체의 shared_ptr
     */
    std::shared_ptr<IMLController> GetController() const;

    /**
     * @brief 모든 MVC 컴포넌트가 설정되었는지 확인
     * @return bool 모든 컴포넌트가 설정되었으면 true
     */
    bool IsInitialized() const;

    /**
     * @brief 모든 MVC 컴포넌트 초기화
     */
    void Reset();

private:
    /**
     * @brief 소멸자 (private으로 외부에서 직접 delete 불가)
     */
    ~FMLMVCHolder() = default;
};