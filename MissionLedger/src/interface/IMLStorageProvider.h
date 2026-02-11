#pragma once
#include <vector>
#include <memory>
#include <string>
#include "MLDefine.h"

/// <summary>
/// 저장소 타입 열거형
/// </summary>
enum class E_MLStorageType
{
    SQLite,
    JSON,
    XML,
    Binary
};

/// <summary>
/// 저장소 제공자 인터페이스
/// Strategy Pattern을 사용하여 다양한 저장 방식을 추상화
/// DTO 기반으로 Entity에 의존하지 않음
/// </summary>
class IMLStorageProvider
{
public:
    virtual ~IMLStorageProvider() = default;

    /// <summary>
    /// 저장소 초기화 및 연결
    /// </summary>
    /// <param name="filePath">저장소 파일 경로</param>
    /// <returns>성공 여부</returns>
    virtual bool Initialize(const std::string& filePath) = 0;

    /// <summary>
    /// 저장소 연결 종료 및 리소스 정리
    /// </summary>
    /// <returns>성공 여부</returns>
    virtual bool Close() = 0;

    /// <summary>
    /// 단일 거래 저장 (추가 또는 업데이트)
    /// </summary>
    /// <param name="data">저장할 거래 데이터 (DTO)</param>
    /// <returns>성공 여부</returns>
    virtual bool SaveTransaction(const FMLTransactionData& data) = 0;

    /// <summary>
    /// 모든 거래 일괄 저장 (기존 데이터 덮어쓰기)
    /// </summary>
    /// <param name="transactions">저장할 거래 데이터 목록 (DTO)</param>
    /// <returns>성공 여부</returns>
    virtual bool SaveAllTransactions(const std::vector<FMLTransactionData>& transactions) = 0;

    /// <summary>
    /// 저장소에서 모든 거래 로드
    /// </summary>
    /// <param name="outTransactions">로드된 거래 데이터가 저장될 벡터 (DTO)</param>
    /// <returns>성공 여부</returns>
    virtual bool LoadAllTransactions(std::vector<FMLTransactionData>& outTransactions) = 0;

    /// <summary>
    /// 특정 거래 삭제
    /// </summary>
    /// <param name="transactionId">삭제할 거래 ID</param>
    /// <returns>성공 여부</returns>
    virtual bool DeleteTransaction(int transactionId) = 0;

    /// <summary>
    /// 기존 거래 정보 업데이트
    /// </summary>
    /// <param name="data">업데이트할 거래 데이터 (DTO)</param>
    /// <returns>성공 여부</returns>
    virtual bool UpdateTransaction(const FMLTransactionData& data) = 0;

    /// <summary>
    /// 저장소에서 마지막으로 사용된 거래 ID 조회
    /// </summary>
    /// <returns>마지막 거래 ID (데이터 없으면 -1)</returns>
    virtual int GetLastTransactionId() = 0;

    /// <summary>
    /// 현재 저장소 타입 반환
    /// </summary>
    /// <returns>저장소 타입</returns>
    virtual E_MLStorageType GetStorageType() const = 0;

    /// <summary>
    /// 저장소가 정상적으로 초기화되어 사용 가능한지 확인
    /// </summary>
    /// <returns>사용 가능 여부</returns>
    virtual bool IsReady() const = 0;

    // ========== 예산 관련 메서드 ==========

    /// <summary>
    /// 단일 예산 저장 (추가 또는 업데이트)
    /// </summary>
    /// <param name="budget">저장할 예산 데이터 (DTO)</param>
    /// <returns>성공 여부</returns>
    virtual bool SaveBudget(const FMLBudgetData& budget) = 0;

    /// <summary>
    /// 저장소에서 모든 예산 로드
    /// </summary>
    /// <param name="outBudgets">로드된 예산 데이터가 저장될 벡터 (DTO)</param>
    /// <returns>성공 여부</returns>
    virtual bool LoadAllBudgets(std::vector<FMLBudgetData>& outBudgets) = 0;

    /// <summary>
    /// 특정 항목의 예산 삭제
    /// </summary>
    /// <param name="category">카테고리명</param>
    /// <param name="item">항목명</param>
    /// <returns>성공 여부</returns>
    virtual bool DeleteBudget(const int budgetId) = 0;
};
