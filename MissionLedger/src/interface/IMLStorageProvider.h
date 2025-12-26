#pragma once
#include <vector>
#include <memory>
#include <string>

class FMLTransaction;

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
    /// <param name="transaction">저장할 거래 객체</param>
    /// <returns>성공 여부</returns>
    virtual bool SaveTransaction(const FMLTransaction& transaction) = 0;

    /// <summary>
    /// 모든 거래 일괄 저장 (기존 데이터 덮어쓰기)
    /// </summary>
    /// <param name="transactions">저장할 거래 목록</param>
    /// <returns>성공 여부</returns>
    virtual bool SaveAllTransactions(const std::vector<std::shared_ptr<FMLTransaction>>& transactions) = 0;

    /// <summary>
    /// 저장소에서 모든 거래 로드
    /// </summary>
    /// <param name="outTransactions">로드된 거래가 저장될 벡터</param>
    /// <returns>성공 여부</returns>
    virtual bool LoadAllTransactions(std::vector<std::shared_ptr<FMLTransaction>>& outTransactions) = 0;

    /// <summary>
    /// 특정 거래 삭제
    /// </summary>
    /// <param name="transactionId">삭제할 거래 ID</param>
    /// <returns>성공 여부</returns>
    virtual bool DeleteTransaction(int transactionId) = 0;

    /// <summary>
    /// 기존 거래 정보 업데이트
    /// </summary>
    /// <param name="transaction">업데이트할 거래 객체</param>
    /// <returns>성공 여부</returns>
    virtual bool UpdateTransaction(const FMLTransaction& transaction) = 0;

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
};
