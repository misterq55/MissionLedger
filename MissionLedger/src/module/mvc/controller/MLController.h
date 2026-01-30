#pragma once
#include "interface/IMLController.h"

/**
 * @brief MVC 패턴의 Controller 구현 클래스
 *
 * View와 Model 사이를 중재하며, 사용자 입력을 Model 작업으로 변환하고
 * Model 데이터를 View용 DTO로 제공합니다.
 *
 * FMLMVCHolder를 통해 Model에 접근하므로 생성 시 의존성이 없습니다.
 */
class FMLController : public IMLController
{
public:
    FMLController() = default;
    virtual ~FMLController() = default;

public:
    // Transaction operations
    virtual void AddTransaction(const FMLTransactionData& transactionData) override;
    virtual bool UpdateTransaction(const FMLTransactionData& transactionData) override;
    virtual bool RemoveTransaction(const int transactionId) override;

    // Data retrieval for View
    virtual FMLTransactionData GetTransactionData(const int transactionId) override;
    virtual std::vector<FMLTransactionData> GetAllTransactionData() override;
    virtual std::vector<FMLTransactionData> GetFilteredTransactionData(const FMLFilterCriteria& criteria) override;

    // Transaction Summary
    virtual FMLTransactionSummary GetTransactionSummary() override;
    virtual FMLTransactionSummary GetFilteredTransactionSummary(const FMLFilterCriteria& criteria) override;

    // Persistence
    virtual bool SaveData() override;
    virtual bool LoadData() override;

    // File operations
    virtual void NewFile() override;
    virtual bool OpenFile(const std::string& filePath) override;
    virtual bool SaveFile() override;
    virtual bool SaveFileAs(const std::string& filePath) override;
    virtual std::string GetCurrentFilePath() const override;
    virtual bool HasUnsavedChanges() const override;
};
