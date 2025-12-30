#include "MLController.h"
#include "interface/IMLModel.h"
#include "module/common/holder/MLMVCHolder.h"
#include "MLDefine.h"

// Transaction operations
void FMLController::AddTransaction(const FMLTransactionData& transactionData)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        model->AddTransaction(transactionData);
    }
}

bool FMLController::UpdateTransaction(const FMLTransactionData& transactionData)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->UpdateTransaction(transactionData);
    }
    return false;
}

bool FMLController::RemoveTransaction(const int transactionId)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->RemoveTransaction(transactionId);
    }
    return false;
}

// Data retrieval for View
FMLTransactionData FMLController::GetTransactionData(const int transactionId)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->GetTransactionData(transactionId);
    }

    // Model이 없으면 빈 데이터 반환
    FMLTransactionData emptyData;
    emptyData.TransactionId = -1;
    return emptyData;
}

std::vector<FMLTransactionData> FMLController::GetAllTransactionData()
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->GetAllTransactionData();
    }

    // Model이 없으면 빈 벡터 반환
    return std::vector<FMLTransactionData>();
}

// Business logic delegation
float FMLController::GetCategoryTotal(const std::string& category)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->GetCategoryTotal(category);
    }
    return 0.0f;
}

float FMLController::GetAllTotal()
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->GetAllTotal();
    }
    return 0.0f;
}

// Persistence
bool FMLController::SaveData()
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->Save();
    }
    return false;
}

bool FMLController::LoadData()
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->Load();
    }
    return false;
}
