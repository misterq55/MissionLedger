#include "MLModel.h"
#include "module/mvc/model/transaction/MLTransaction.h"
#include "MLDefine.h"
#include <vector>
#include <algorithm>

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
    int newId = TransactionIdIndex;

    auto newTransaction = std::make_shared<FMLTransaction>(
        newId, transactionData.Type,
        transactionData.Category, transactionData.Item,
        transactionData.Description, transactionData.Amount,
        transactionData.DateTime,
        transactionData.ReceiptNumber);

    Transactions.emplace(newId, newTransaction);
    TransactionIdIndex++;
    UnsavedChanges = true;

    if (ModelObserver)
    {
        // Entity에서 변환하여 완전한 Output DTO 전달
        ModelObserver->OnTransactionAdded(convertToTransactionData(newTransaction));
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
    transaction->SetType(transactionData.Type);
    transaction->SetCategory(transactionData.Category);
    transaction->SetItem(transactionData.Item);
    transaction->SetDescription(transactionData.Description);
    transaction->SetAmount(transactionData.Amount);
    transaction->SetReceiptNumber(transactionData.ReceiptNumber);
    UnsavedChanges = true;

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
        return convertToTransactionData(it->second);
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
        result.push_back(convertToTransactionData(pair.second));
    }

    return result;
}

std::vector<FMLTransactionData> FMLModel::GetFilteredTransactionData(const FMLFilterCriteria& criteria)
{
    std::vector<FMLTransactionData> result;

    for (const auto& pair : Transactions)
    {
        const auto& transaction = pair.second;
        FMLTransactionData data = convertToTransactionData(transaction);

        // 거래 유형 필터
        if (criteria.UseTypeFilter && data.Type != criteria.TypeFilter)
        {
            continue;
        }

        // 날짜 범위 필터
        if (criteria.UseDateRangeFilter)
        {
            std::string transactionDate = data.DateTime.substr(0, 10); // YYYY-MM-DD 부분만 추출
            if (transactionDate < criteria.StartDate || transactionDate > criteria.EndDate)
            {
                continue;
            }
        }

        // 카테고리 필터
        if (criteria.UseCategoryFilter && data.Category != criteria.CategoryFilter)
        {
            continue;
        }

        // 금액 범위 필터
        if (criteria.UseAmountRangeFilter)
        {
            if (data.Amount < criteria.MinAmount || data.Amount > criteria.MaxAmount)
            {
                continue;
            }
        }

        // 검색어 필터
        if (criteria.UseTextSearch)
        {
            // 검색 필드가 하나라도 선택되었는지 확인
            if (criteria.SearchInItem || criteria.SearchInDescription || criteria.SearchInReceipt)
            {
                bool bfound = false;

                if (criteria.SearchInDescription)
                {
                    if (data.Description.find(criteria.SearchText) != std::string::npos)
                    {
                        bfound = true;
                    }
                }

                if (criteria.SearchInItem)
                {
                    if (data.Item.find(criteria.SearchText) != std::string::npos)
                    {
                        bfound = true;
                    }
                }

                if (criteria.SearchInReceipt)
                {
                    if (data.ReceiptNumber.find(criteria.SearchText) != std::string::npos)
                    {
                        bfound = true;
                    }
                }

                if (!bfound)
                {
                    continue;
                }
            }
            // 검색 필드가 모두 false면 검색 스킵 (모든 거래 포함)
        }

        result.push_back(data);
    }

    return result;
}

// Data retrieval - Entity 기반
std::shared_ptr<FMLTransaction> FMLModel::GetTransaction(const int transactionId)
{
    auto it = Transactions.find(transactionId);
    if (it != Transactions.end())
    {
        return it->second;
    }
    return nullptr;
}

std::map<int, std::shared_ptr<FMLTransaction>> FMLModel::GetAllTransactions()
{
    return Transactions;
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

    // 파일에서 거래 로드
    std::vector<std::shared_ptr<FMLTransaction>> loadedTransactions;
    if (!StorageProvider->LoadAllTransactions(loadedTransactions))
    {
        StorageProvider->Close();
        return false;
    }

    // 메모리에 로드
    for (const auto& transaction : loadedTransactions)
    {
        Transactions.emplace(transaction->GetId(), transaction);
    }

    // TransactionIdIndex 갱신
    const int lastId = StorageProvider->GetLastTransactionId();
    TransactionIdIndex = (lastId >= 0) ? lastId + 1 : 0;

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

    // 모든 거래를 벡터로 변환
    std::vector<std::shared_ptr<FMLTransaction>> transactionList;
    transactionList.reserve(Transactions.size());
    for (const auto& pair : Transactions)
    {
        transactionList.push_back(pair.second);
    }

    if (!StorageProvider->SaveAllTransactions(transactionList))
    {
        return false;
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

        default:
            break;
        }
    }

    return transactionSummary;
}

// Private helper
FMLTransactionData FMLModel::convertToTransactionData(const std::shared_ptr<FMLTransaction>& transaction)
{
    FMLTransactionData data;
    data.TransactionId = transaction->GetId();
    data.Type = transaction->GetType();
    data.Category = transaction->GetCategory();
    data.Item = transaction->GetItem();
    data.Description = transaction->GetDescription();
    data.Amount = transaction->GetAmount();
    data.ReceiptNumber = transaction->GetReceiptNumber();
    data.DateTime = transaction->GetDateTimeString(); // 포맷된 문자열로 변환
    return data;
}
