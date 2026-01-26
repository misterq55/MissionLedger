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
float FMLModel::GetCategoryTotal(const std::string& category)
{
    float total = 0.0f;
    for (const auto& pair : Transactions)
    {
        if (pair.second->GetCategory() == category)
        {
            total += static_cast<float>(pair.second->GetAmount());
        }
    }
    return total;
}

float FMLModel::GetAllTotal()
{
    float total = 0.0f;
    for (const auto& pair : Transactions)
    {
        if (pair.second->IsIncome())
        {
            total += static_cast<float>(pair.second->GetAmount());
        }
        else if (pair.second->IsExpense())
        {
            total -= static_cast<float>(pair.second->GetAmount());
        }
    }
    return total;
}

int FMLModel::GetNextTransactionId()
{
    return TransactionIdIndex;
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
    int lastId = StorageProvider->GetLastTransactionId();
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
    data.DateTime = transaction->GetDateTimeString();  // 포맷된 문자열로 변환
    return data;
}
