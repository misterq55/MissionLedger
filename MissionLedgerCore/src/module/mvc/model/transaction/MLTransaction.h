#pragma once
#include "MLDefine.h"
#include <string>

/// <summary>
/// 트랜잭션 Entity
/// FMLTransactionData를 포함하여 비즈니스 로직을 제공
/// Data-Oriented Design: Data와 Behavior 분리
/// </summary>
class FMLTransaction
{
public:
    FMLTransaction();

    // Data 접근
    const FMLTransactionData& GetData() const;
    void SetData(const FMLTransactionData& data);

    // 비즈니스 로직
    bool MatchesFilter(const FMLFilterCriteria& criteria) const;
    bool IsValid() const;

private:
    FMLTransactionData data_;

    // Private helpers
    bool matchesSearchText(const FMLFilterCriteria& criteria) const;
};
