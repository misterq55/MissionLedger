#include "MLModel.h"
#include "module/mvc/model/transaction/MLTransaction.h"
#include "module/mvc/model/budget/MLCategoryBudget.h"
#include "MLDefine.h"
#include <vector>
#include <algorithm>
#include <iostream>

#include "interface/IMLModelObserver.h"
#include "interface/IMLStorageProvider.h"

FMLModel::FMLModel()
{
}

FMLModel::~FMLModel()
{
}

void FMLModel::AddObserver(std::shared_ptr<IMLModelObserver> modelObserver)
{
    ModelObserver = modelObserver;
}

// Transaction CRUD operations
void FMLModel::AddTransaction(const FMLTransactionData& transactionData)
{
    const int newId = TransactionIdIndex;

    // Entity 생성 및 데이터 설정
    auto newTransaction = std::make_shared<FMLTransaction>();
    FMLTransactionData dataWithId = transactionData;
    dataWithId.TransactionId = newId;
    newTransaction->SetData(dataWithId);

    Transactions.emplace(newId, newTransaction);
    TransactionIdIndex++;
    UnsavedChanges = true;
    invalidateCategoryCache();

    if (ModelObserver)
    {
        ModelObserver->OnTransactionAdded(newTransaction->GetData());
    }
}

bool FMLModel::UpdateTransaction(const FMLTransactionData& transactionData)
{
    auto it = Transactions.find(transactionData.TransactionId);
    if (it == Transactions.end())
    {
        return false;
    }

    auto& transaction = it->second;
    transaction->SetData(transactionData);
    UnsavedChanges = true;
    invalidateCategoryCache();

    if (ModelObserver)
    {
        ModelObserver->OnTransactionUpdated(transactionData);
    }

    return true;
}

bool FMLModel::RemoveTransaction(const int transactionId)
{
    auto it = Transactions.find(transactionId);
    if (it == Transactions.end())
    {
        return false;
    }

    Transactions.erase(it);
    UnsavedChanges = true;
    invalidateCategoryCache();

    if (ModelObserver)
    {
        ModelObserver->OnTransactionRemoved(transactionId);
    }

    return true;
}

// Data retrieval - DTO 기반
FMLTransactionData FMLModel::GetTransactionData(const int transactionId)
{
    auto it = Transactions.find(transactionId);
    if (it != Transactions.end())
    {
        return it->second->GetData();
    }

    // 빈 데이터 반환 (ID -1로 표시)
    FMLTransactionData emptyData;
    emptyData.TransactionId = -1;
    return emptyData;
}

std::vector<FMLTransactionData> FMLModel::GetAllTransactionData()
{
    std::vector<FMLTransactionData> result;
    result.reserve(Transactions.size());

    for (const auto& pair : Transactions)
    {
        result.push_back(pair.second->GetData());
    }

    return result;
}

std::vector<FMLTransactionData> FMLModel::GetFilteredTransactionData(const FMLFilterCriteria& criteria)
{
    std::vector<FMLTransactionData> result;

    for (const auto& pair : Transactions)
    {
        const auto& transaction = pair.second;

        // Entity의 비즈니스 로직 사용
        if (transaction->MatchesFilter(criteria))
        {
            result.push_back(transaction->GetData());
        }
    }

    return result;
}

// Business logic

FMLTransactionSummary FMLModel::CalculateTransactionSummary()
{
    return calculateTransactionSummary(GetAllTransactionData());
}

FMLTransactionSummary FMLModel::CalculateFilteredTransactionSummary(const FMLFilterCriteria& criteria)
{
    return calculateTransactionSummary(GetFilteredTransactionData(criteria));
}

// Persistence
bool FMLModel::Save()
{
    return SaveFile();
}

bool FMLModel::Load()
{
    if (CurrentFilePath.empty()) return false;
    return OpenFile(CurrentFilePath);
}

void FMLModel::ExportToExcel()
{
    // TODO: Excel export 구현
}

// File Operations
void FMLModel::SetStorageProvider(std::shared_ptr<IMLStorageProvider> storageProvider)
{
    StorageProvider = storageProvider;
}

bool FMLModel::OpenFile(const std::string& filePath)
{
    if (!StorageProvider) return false;

    if (!StorageProvider->Initialize(filePath))
    {
        return false;
    }

    // 기존 데이터 정리
    clearAllTransactions();

    // 파일에서 거래 로드 (DTO)
    std::vector<FMLTransactionData> loadedData;
    if (!StorageProvider->LoadAllTransactions(loadedData))
    {
        StorageProvider->Close();
        return false;
    }

    // DTO → Entity 변환 후 메모리에 로드
    for (const auto& data : loadedData)
    {
        auto transaction = std::make_shared<FMLTransaction>();
        transaction->SetData(data);
        Transactions.emplace(data.TransactionId, transaction);
    }

    // TransactionIdIndex 갱신
    const int lastId = StorageProvider->GetLastTransactionId();
    TransactionIdIndex = (lastId >= 0) ? lastId + 1 : 0;

    // 예산 로드
    Budgets.clear();
    std::vector<FMLCategoryBudgetData> loadedBudgets;
    if (StorageProvider->LoadAllBudgets(loadedBudgets))
    {
        for (const auto& budgetData : loadedBudgets)
        {
            auto budgetEntity = std::make_shared<FMLCategoryBudget>();
            budgetEntity->SetData(budgetData);
            Budgets[budgetData.Category] = budgetEntity;
        }
    }

    CurrentFilePath = filePath;
    UnsavedChanges = false;

    // Observer에게 데이터 로드 알림
    if (ModelObserver)
    {
        ModelObserver->OnDataLoaded();
    }

    return true;
}

bool FMLModel::SaveFile()
{
    if (CurrentFilePath.empty() || !StorageProvider)
    {
        return false;
    }

    if (!StorageProvider->IsReady())
    {
        if (!StorageProvider->Initialize(CurrentFilePath))
        {
            return false;
        }
    }

    // Entity → DTO 변환
    std::vector<FMLTransactionData> transactionDataList;
    transactionDataList.reserve(Transactions.size());
    for (const auto& pair : Transactions)
    {
        transactionDataList.push_back(pair.second->GetData());
    }

    if (!StorageProvider->SaveAllTransactions(transactionDataList))
    {
        return false;
    }

    // 예산 저장
    for (const auto& pair : Budgets)
    {
        if (!StorageProvider->SaveBudget(pair.second->GetData()))
        {
            return false;
        }
    }

    UnsavedChanges = false;

    // Observer에게 저장 완료 알림
    if (ModelObserver)
    {
        ModelObserver->OnDataSaved();
    }

    return true;
}

bool FMLModel::SaveFileAs(const std::string& filePath)
{
    if (!StorageProvider) return false;

    // 기존 연결 닫기
    StorageProvider->Close();

    // 새 파일 경로로 초기화
    if (!StorageProvider->Initialize(filePath))
    {
        return false;
    }

    CurrentFilePath = filePath;
    return SaveFile();
}

void FMLModel::NewFile()
{
    // 기존 데이터 정리
    clearAllTransactions();
    Budgets.clear();

    // 스토리지 연결 닫기
    if (StorageProvider)
    {
        StorageProvider->Close();
    }

    CurrentFilePath.clear();
    TransactionIdIndex = 0;
    UnsavedChanges = false;

    // Observer에게 데이터 클리어 알림
    if (ModelObserver)
    {
        ModelObserver->OnTransactionsCleared();
    }
}

const std::string& FMLModel::GetCurrentFilePath() const
{
    return CurrentFilePath;
}

bool FMLModel::HasUnsavedChanges() const
{
    return UnsavedChanges;
}

void FMLModel::clearAllTransactions()
{
    Transactions.clear();
    TransactionIdIndex = 0;
    invalidateCategoryCache();
}

FMLTransactionSummary FMLModel::calculateTransactionSummary(const std::vector<FMLTransactionData>& transactionData)
{
    FMLTransactionSummary transactionSummary;
    for (const auto& transaction : transactionData)
    {
        transactionSummary.TransactionCount++;
        switch (transaction.Type)
        {
        case E_MLTransactionType::Income:
            transactionSummary.TotalIncome += transaction.Amount;
            transactionSummary.Balance += transaction.Amount;
            break;

        case E_MLTransactionType::Expense:
            transactionSummary.TotalExpense += transaction.Amount;
            transactionSummary.Balance -= transaction.Amount;
            break;

        case E_MLTransactionType::_Max:
        default:
            break;
        }
    }

    return transactionSummary;
}

// Private helper
// 카테고리 캐싱
// NOTE: 단일 스레드(GUI 메인 스레드) 전용 메서드
//       멀티스레드 환경에서는 mutable 변수 접근 시 동기화 필요
std::vector<std::string> FMLModel::GetAllCategories() const
{
    if (CategoryCacheDirty)
    {
        rebuildCategoryCache();
        CategoryCacheDirty = false;
    }

    return std::vector<std::string>(CachedCategories.begin(), CachedCategories.end());
}

void FMLModel::rebuildCategoryCache() const
{
    CachedCategories.clear();

    for (const auto& pair : Transactions)
    {
        const auto& category = pair.second->GetData().Category;
        if (!category.empty())
        {
            CachedCategories.insert(category);
        }
    }
}

void FMLModel::invalidateCategoryCache()
{
    CategoryCacheDirty = true;
}

// ========== 예산 관련 메서드 구현 ==========

bool FMLModel::AddBudget(const FMLCategoryBudgetData& budgetData)
{
    // 이미 존재하는지 확인
    if (Budgets.find(budgetData.Category) != Budgets.end()) {
        return false;  // 이미 존재 - 추가 실패
    }

    // 새 Entity 생성
    auto budgetEntity = std::make_shared<FMLCategoryBudget>();
    budgetEntity->SetData(budgetData);
    if (!budgetEntity->IsValid()) {
        return false;
    }

    Budgets[budgetData.Category] = budgetEntity;
    UnsavedChanges = true;

    return true;
}

bool FMLModel::UpdateBudget(const FMLCategoryBudgetData& budgetData)
{
    auto it = Budgets.find(budgetData.Category);

    if (it == Budgets.end()) {
        return false;  // 존재하지 않음 - 업데이트 실패
    }

    // 기존 Entity의 데이터만 업데이트 (재생성 안 함)
    it->second->SetData(budgetData);
    UnsavedChanges = true;

    return true;
}

bool FMLModel::DeleteBudget(const std::string& category)
{
    Budgets.erase(category);
    UnsavedChanges = true;
    return true;
}

std::vector<FMLCategoryBudgetData> FMLModel::GetAllBudgets() const
{
    std::vector<FMLCategoryBudgetData> result;
    result.reserve(Budgets.size());

    for (const auto& pair : Budgets) {
        result.push_back(pair.second->GetData());
    }

    return result;
}

FMLCategoryBudgetData FMLModel::GetBudget(const std::string& category) const
{
    auto it = Budgets.find(category);
    if (it != Budgets.end()) {
        return it->second->GetData();
    }
    return FMLCategoryBudgetData{category, 0, 0};
}

FMLBudgetSummary FMLModel::GetBudgetSummary() const
{
    FMLBudgetSummary summary;

    // 1. 예산 데이터 로드 (모든 카테고리)
    for (const auto& pair : Budgets) {
        const std::string& category = pair.first;
        const auto& budgetData = pair.second->GetData();
        auto& catSummary = summary.Categories[category];
        catSummary.Category = category;
        catSummary.BudgetIncome = budgetData.IncomeAmount;
        catSummary.BudgetExpense = budgetData.ExpenseAmount;
    }

    // 2. 실제 거래 집계 (모든 거래)
    for (const auto& pair : Transactions) {
        const auto& data = pair.second->GetData();
        auto& catSummary = summary.Categories[data.Category];

        if (catSummary.Category.empty()) {
            catSummary.Category = data.Category;
        }

        catSummary.TransactionCount++;
        if (data.Type == E_MLTransactionType::Income) {
            catSummary.ActualIncome += data.Amount;
        } else {
            catSummary.ActualExpense += data.Amount;
        }
    }

    // 3. 카테고리별 차이 계산 및 전체 합계 계산
    for (auto& pair : summary.Categories) {
        auto& catSummary = pair.second;
        catSummary.IncomeVariance = catSummary.ActualIncome - catSummary.BudgetIncome;
        catSummary.ExpenseVariance = catSummary.ActualExpense - catSummary.BudgetExpense;

        // 전체 합계에 추가
        summary.TotalBudgetIncome += catSummary.BudgetIncome;
        summary.TotalBudgetExpense += catSummary.BudgetExpense;
        summary.TotalActualIncome += catSummary.ActualIncome;
        summary.TotalActualExpense += catSummary.ActualExpense;
    }

    // 4. 전체 차이 계산
    summary.TotalIncomeVariance = summary.TotalActualIncome - summary.TotalBudgetIncome;
    summary.TotalExpenseVariance = summary.TotalActualExpense - summary.TotalBudgetExpense;
    summary.CategoryCount = static_cast<int>(summary.Categories.size());

    return summary;
}

FMLBudgetSummary FMLModel::GetFilteredBudgetSummary(const FMLFilterCriteria& criteria) const
{
    FMLBudgetSummary summary;

    // 1. 예산 데이터 로드 (필터 무관 - 모든 카테고리)
    for (const auto& pair : Budgets) {
        const std::string& category = pair.first;
        const auto& budgetData = pair.second->GetData();
        auto& catSummary = summary.Categories[category];
        catSummary.Category = category;
        catSummary.BudgetIncome = budgetData.IncomeAmount;
        catSummary.BudgetExpense = budgetData.ExpenseAmount;
    }

    // 2. 필터링된 거래만 집계
    for (const auto& pair : Transactions) {
        if (!pair.second->MatchesFilter(criteria)) continue;

        const auto& data = pair.second->GetData();
        auto& catSummary = summary.Categories[data.Category];

        if (catSummary.Category.empty()) {
            catSummary.Category = data.Category;
        }

        catSummary.TransactionCount++;
        if (data.Type == E_MLTransactionType::Income) {
            catSummary.ActualIncome += data.Amount;
        } else {
            catSummary.ActualExpense += data.Amount;
        }
    }

    // 3. 카테고리별 차이 계산 및 전체 합계 계산
    for (auto& pair : summary.Categories) {
        auto& catSummary = pair.second;
        catSummary.IncomeVariance = catSummary.ActualIncome - catSummary.BudgetIncome;
        catSummary.ExpenseVariance = catSummary.ActualExpense - catSummary.BudgetExpense;

        // 전체 합계에 추가
        summary.TotalBudgetIncome += catSummary.BudgetIncome;
        summary.TotalBudgetExpense += catSummary.BudgetExpense;
        summary.TotalActualIncome += catSummary.ActualIncome;
        summary.TotalActualExpense += catSummary.ActualExpense;
    }

    // 4. 전체 차이 계산
    summary.TotalIncomeVariance = summary.TotalActualIncome - summary.TotalBudgetIncome;
    summary.TotalExpenseVariance = summary.TotalActualExpense - summary.TotalBudgetExpense;
    summary.CategoryCount = static_cast<int>(summary.Categories.size());

    return summary;
}
