#include "MLMVCHolder.h"
#include "interface/IMLModel.h"
#include "interface/IMLView.h"
#include "interface/IMLController.h"

// 정적 멤버 초기화
FMLMVCHolder* FMLMVCHolder::Instance = nullptr;

FMLMVCHolder& FMLMVCHolder::GetInstance()
{
    if (Instance == nullptr)
    {
        Instance = new FMLMVCHolder();
    }
    return *Instance;
}

void FMLMVCHolder::DestroyInstance()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void FMLMVCHolder::SetModel(std::shared_ptr<IMLModel> model)
{
    Model = model;
}

void FMLMVCHolder::SetView(std::shared_ptr<IMLView> view)
{
    View = view;
}

void FMLMVCHolder::SetController(std::shared_ptr<IMLController> controller)
{
    Controller = controller;
}

std::shared_ptr<IMLModel> FMLMVCHolder::GetModel() const
{
    return Model;
}

std::shared_ptr<IMLView> FMLMVCHolder::GetView() const
{
    return View;
}

std::shared_ptr<IMLController> FMLMVCHolder::GetController() const
{
    return Controller;
}

bool FMLMVCHolder::IsInitialized() const
{
    return Model != nullptr && View != nullptr && Controller != nullptr;
}

void FMLMVCHolder::Reset()
{
    Model.reset();
    View.reset();
    Controller.reset();
}