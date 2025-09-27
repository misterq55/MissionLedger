#include "MLMVCHolder.h"
#include "interface/IMLModel.h"
#include "interface/IMLView.h"
#include "interface/IMLController.h"

// 정적 멤버 초기화
CMLMVCHolder* CMLMVCHolder::Instance = nullptr;

CMLMVCHolder& CMLMVCHolder::GetInstance()
{
    if (Instance == nullptr)
    {
        Instance = new CMLMVCHolder();
    }
    return *Instance;
}

void CMLMVCHolder::DestroyInstance()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void CMLMVCHolder::SetModel(std::shared_ptr<IMLModel> model)
{
    Model = model;
}

void CMLMVCHolder::SetView(std::shared_ptr<IMLView> view)
{
    View = view;
}

void CMLMVCHolder::SetController(std::shared_ptr<IMLController> controller)
{
    Controller = controller;
}

std::shared_ptr<IMLModel> CMLMVCHolder::GetModel() const
{
    return Model;
}

std::shared_ptr<IMLView> CMLMVCHolder::GetView() const
{
    return View;
}

std::shared_ptr<IMLController> CMLMVCHolder::GetController() const
{
    return Controller;
}

bool CMLMVCHolder::IsInitialized() const
{
    return Model != nullptr && View != nullptr && Controller != nullptr;
}

void CMLMVCHolder::Reset()
{
    Model.reset();
    View.reset();
    Controller.reset();
}