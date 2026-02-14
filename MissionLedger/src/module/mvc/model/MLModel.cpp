#include "MLModel.h"
#include "module/mvc/model/transaction/MLTransaction.h"
#include "module/mvc/model/budget/MLBudget.h"
#include "MLDefine.h"
#include <vector>
#include <algorithm>
#include <iostream>

#include "interface/IMLModelObserver.h"
#include "interface/IMLStorageProvider.h"
#include "module/export/MLPDFExporter.h"

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
bool FMLModel::AddTransaction(const FMLTransactionData& transactionData)
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

    return true;
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

bool FMLModel::DeleteTransaction(const int transactionId)
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
        ModelObserver->OnTransactionDeleted(transactionId);
    }

    return true;
}

// Data retrieval - DTO 기반
FMLTransactionData FMLModel::GetTransactionData(const int transactionId) const
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

std::vector<FMLTransactionData> FMLModel::GetAllTransactionData() const
{
    std::vector<FMLTransactionData> result;
    result.reserve(Transactions.size());

    for (const auto& pair : Transactions)
    {
        result.push_back(pair.second->GetData());
    }

    return result;
}

std::vector<FMLTransactionData> FMLModel::GetFilteredTransactionData(const FMLFilterCriteria& criteria) const
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

FMLTransactionSummary FMLModel::GetTransactionSummary() const
{
    return calculateTransactionSummary(GetAllTransactionData());
}

FMLTransactionSummary FMLModel::GetFilteredTransactionSummary(const FMLFilterCriteria& criteria) const
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

bool FMLModel::ExportSettlementToPDF(const std::string& filePath)
{
    // 결산안 데이터
    FMLSettlmentData settlmentData;

    // 현재 파일 경로에서 제목 추출
    if (!CurrentFilePath.empty())
    {
        size_t lastSlash = CurrentFilePath.find_last_of("/\\");
        std::string filename = (lastSlash != std::string::npos)
            ? CurrentFilePath.substr(lastSlash + 1)
            : CurrentFilePath;

        size_t lastDot = filename.find_last_of(".");
        if (lastDot != std::string::npos)
        {
            settlmentData.Title = filename.substr(0, lastDot);
        }
        else
        {
            settlmentData.Title = filename;
        }
    }

    // 예산 데이터 집계
    for (const auto& budget : Budgets)
    {
        const auto& budgetData = budget.second->GetData();
        if (budgetData.Type == E_MLTransactionType::Income)
        {
            settlmentData.TotalIncome += budgetData.BudgetAmount;
            settlmentData.BudgetIncomeCategories[budgetData.Category] += budgetData.BudgetAmount;
            // 중분류 집계 (카테고리 → 항목)
            settlmentData.BudgetIncomeItems[budgetData.Category][budgetData.Item] += budgetData.BudgetAmount;
        }
        else if (budgetData.Type == E_MLTransactionType::Expense)
        {
            settlmentData.TotalExpense += budgetData.BudgetAmount;
            settlmentData.BudgetExpenseCategories[budgetData.Category] += budgetData.BudgetAmount;
            // 중분류 집계 (카테고리 → 항목)
            settlmentData.BudgetExpenseItems[budgetData.Category][budgetData.Item] += budgetData.BudgetAmount;
        }
    }

    // 실적 데이터 집계
    for (const auto& transaction : Transactions)
    {
        const auto& transactionData = transaction.second->GetData();
        if (transactionData.Type == E_MLTransactionType::Income)
        {
            settlmentData.TotalActualIncome += transactionData.Amount;
            settlmentData.ActualIncomeCategories[transactionData.Category] += transactionData.Amount;
            // 중분류 집계 (카테고리 → 항목)
            settlmentData.ActualIncomeItems[transactionData.Category][transactionData.Item] += transactionData.Amount;
        }
        else if (transactionData.Type == E_MLTransactionType::Expense)
        {
            settlmentData.TotalActualExpense += transactionData.Amount;
            settlmentData.ActualExpenseCategories[transactionData.Category] += transactionData.Amount;
            // 중분류 집계 (카테고리 → 항목)
            settlmentData.ActualExpenseItems[transactionData.Category][transactionData.Item] += transactionData.Amount;
        }
    }

    settlmentData.TotalBalance = settlmentData.TotalActualIncome - settlmentData.TotalActualExpense;
    
    // 환율 정보
    std::map<std::string, double> exchangeRates;
    std::map<std::string, int> exchangeRateCounts;
    
    for (const auto& transaction : Transactions)
    {
        const auto& transactionData = transaction.second->GetData();
        if (transactionData.UseExchangeRate)
        {
            if (exchangeRates.find(transactionData.Currency) != exchangeRates.end())
            {
                exchangeRates[transactionData.Currency] += transactionData.ExchangeRate;
                exchangeRateCounts[transactionData.Currency]++;
            }
            else
            {
                exchangeRates[transactionData.Currency] = transactionData.ExchangeRate;
                exchangeRateCounts[transactionData.Currency] = 1;
            }
        }
    }
    
    for (const auto& exchangeRate : exchangeRates)
    {
        settlmentData.ExchangeRates[exchangeRate.first] = (exchangeRate.second / exchangeRateCounts[exchangeRate.first]);
    }

    // PDF 생성
    return FMLPDFExporter::ExportSettlement(settlmentData, filePath);
}

bool FMLModel::ExportTransactionListToPDF(const std::string& filePath)
{
    // 모든 거래 데이터 가져오기
    std::vector<FMLTransactionData> transactions = GetAllTransactionData();

    // PDF 생성
    return FMLPDFExporter::ExportTransactionList(transactions, filePath);
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
    std::vector<FMLBudgetData> loadedBudgets;
    if (StorageProvider->LoadAllBudgets(loadedBudgets))
    {
        for (const auto& budgetData : loadedBudgets)
        {
            auto budgetEntity = std::make_shared<FMLBudget>();
            budgetEntity->SetData(budgetData);
            Budgets.emplace(budgetData.BudgetId, budgetEntity);
        }

        // BudgetIdIndex 갱신 (가장 큰 BudgetId + 1)
        if (!loadedBudgets.empty())
        {
            int maxBudgetId = -1;
            for (const auto& budgetData : loadedBudgets)
            {
                if (budgetData.BudgetId > maxBudgetId)
                {
                    maxBudgetId = budgetData.BudgetId;
                }
            }
            BudgetIdIndex = maxBudgetId + 1;
        }
        else
        {
            BudgetIdIndex = 0;
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

    // 예산 저장 (기존 예산을 모두 삭제하고 현재 메모리의 예산만 저장)
    if (!StorageProvider->DeleteAllBudgets())
    {
        return false;
    }

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
    BudgetIdIndex = 0;
    UnsavedChanges = false;

    // Observer에게 데이터 클리어 알림
    if (ModelObserver)
    {
        ModelObserver->OnTransactionsCleared();
        ModelObserver->OnBudgetCleared();
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

FMLTransactionSummary FMLModel::calculateTransactionSummary(const std::vector<FMLTransactionData>& transactionData) const
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

bool FMLModel::AddBudget(const FMLBudgetData& budgetData)
{
    const int newId = BudgetIdIndex;

    // Entity 생성 및 데이터 설정
    auto newBudget = std::make_shared<FMLBudget>();
    FMLBudgetData dataWithId = budgetData;
    dataWithId.BudgetId = newId;
    newBudget->SetData(dataWithId);

    Budgets.emplace(newId, newBudget);
    BudgetIdIndex++;
    UnsavedChanges = true;
    invalidateCategoryCache();

    if (ModelObserver)
    {
        ModelObserver->OnBudgetAdded(newBudget->GetData());
    }

    return true;
}

bool FMLModel::UpdateBudget(const FMLBudgetData& budgetData)
{
    auto it = Budgets.find(budgetData.BudgetId);
    if (it == Budgets.end())
    {
        return false;
    }

    auto& budget = it->second;
    budget->SetData(budgetData);
    UnsavedChanges = true;
    invalidateCategoryCache();

    if (ModelObserver)
    {
        ModelObserver->OnBudgetUpdated(budgetData);
    }

    return true;
}

bool FMLModel::DeleteBudget(const int budgetId)
{
    auto it = Budgets.find(budgetId);
    if (it == Budgets.end())
    {
        return false;
    }

    Budgets.erase(it);
    UnsavedChanges = true;
    invalidateCategoryCache();

    if (ModelObserver)
    {
        ModelObserver->OnBudgetDeleted(budgetId);
    }

    return true;
}

std::vector<FMLBudgetData> FMLModel::GetAllBudgets() const
{
    std::vector<FMLBudgetData> result;
    result.reserve(Budgets.size());

    for (const auto& pair : Budgets)
    {
        result.push_back(pair.second->GetData());
    }

    return result;
}

std::vector<FMLBudgetData> FMLModel::GetAllBudgets(const FMLFilterCriteria& criteria) const
{
    std::vector<FMLBudgetData> result;
    result.reserve(Budgets.size());
    
    for (const auto& pair : Budgets)
    {
        const auto& budget = pair.second;
        if (budget->MatchesFilter(criteria))
        {
            result.push_back(pair.second->GetData());
        }
    }
    
    return result;
}

FMLBudgetData FMLModel::GetBudget(const int budgetId) const
{
    auto it = Budgets.find(budgetId);
    if (it != Budgets.end())
    {
        return it->second->GetData();
    }

    // 빈 데이터 반환 (ID -1로 표시)
    FMLBudgetData emptyData;
    emptyData.BudgetId = -1;
    return emptyData;
}

FMLBudgetSummary FMLModel::GetBudgetSummary() const
{
    FMLBudgetSummary summary;
    std::vector<FMLBudgetData> budgets = GetAllBudgets();

    for (const auto& budget : budgets)
    {
        summary.TotalBudget += budget.BudgetAmount;
    }

    return summary;
}

FMLBudgetSummary FMLModel::GetFilteredBudgetSummary(const FMLFilterCriteria& criteria) const
{
    FMLBudgetSummary summary;

    std::vector<FMLBudgetData> budgets = GetAllBudgets(criteria);

    for (const auto& budget : budgets)
    {
        summary.TotalBudget += budget.BudgetAmount;
    }

    return summary;
}
