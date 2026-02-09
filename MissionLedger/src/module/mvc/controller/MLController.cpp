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

std::vector<FMLTransactionData> FMLController::GetFilteredTransactionData(const FMLFilterCriteria& criteria)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->GetFilteredTransactionData(criteria);
    }

    // Model이 없으면 빈 벡터 반환
    return std::vector<FMLTransactionData>();
}

// Transaction Summary
FMLTransactionSummary FMLController::GetTransactionSummary()
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->CalculateTransactionSummary();
    }

    // Model이 없으면 빈 Summary 반환
    FMLTransactionSummary emptySummary;
    return emptySummary;
}

FMLTransactionSummary FMLController::GetFilteredTransactionSummary(const FMLFilterCriteria& criteria)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->CalculateFilteredTransactionSummary(criteria);
    }

    // Model이 없으면 빈 Summary 반환
    FMLTransactionSummary emptySummary;
    return emptySummary;
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

// File operations
void FMLController::NewFile()
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        model->NewFile();
    }
}

bool FMLController::OpenFile(const std::string& filePath)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->OpenFile(filePath);
    }
    return false;
}

bool FMLController::SaveFile()
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->SaveFile();
    }
    return false;
}

bool FMLController::SaveFileAs(const std::string& filePath)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->SaveFileAs(filePath);
    }
    return false;
}

std::string FMLController::GetCurrentFilePath() const
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->GetCurrentFilePath();
    }
    return "";
}

bool FMLController::HasUnsavedChanges() const
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->HasUnsavedChanges();
    }
    return false;
}

std::vector<std::string> FMLController::GetAllCategories() const
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->GetAllCategories();
    }
    return {};
}

// ========== 예산 관련 메서드 구현 (Model에 위임) ==========

bool FMLController::AddBudget(const FMLCategoryBudgetData& budgetData)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->AddBudget(budgetData);
    }
    return false;
}

bool FMLController::UpdateBudget(const FMLCategoryBudgetData& budgetData)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->UpdateBudget(budgetData);
    }
    return false;
}

bool FMLController::DeleteBudget(const std::string& category)
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->DeleteBudget(category);
    }
    return false;
}

std::vector<FMLCategoryBudgetData> FMLController::GetAllBudgets() const
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->GetAllBudgets();
    }
    return {};
}

FMLCategoryBudgetData FMLController::GetBudget(const std::string& category) const
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->GetBudget(category);
    }
    return FMLCategoryBudgetData{category, 0, 0};
}

FMLBudgetSummary FMLController::GetBudgetSummary() const
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->GetBudgetSummary();
    }
    return FMLBudgetSummary{};
}

FMLBudgetSummary FMLController::GetFilteredBudgetSummary(const FMLFilterCriteria& criteria) const
{
    auto model = FMLMVCHolder::GetInstance().GetModel();
    if (model)
    {
        return model->GetFilteredBudgetSummary(criteria);
    }
    return FMLBudgetSummary{};
}
