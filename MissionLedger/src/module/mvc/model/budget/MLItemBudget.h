#pragma once

#include "MLDefine.h"

// 항목 예산 Entity (Data-Oriented Design)
// 예: Category="항공", Item="항공료 선결제", BudgetAmount=3,000,000
class FMLItemBudget {
private:
    FMLItemBudgetData Data;

public:
    FMLItemBudget() {}

    // DTO 접근
    const FMLItemBudgetData& GetData() const { return Data; }
    void SetData(const FMLItemBudgetData& data) { Data = data; }

    // 비즈니스 로직
    bool IsValid() const {
        return !Data.Category.empty() &&
               !Data.Item.empty() &&
               Data.BudgetAmount >= 0;
    }

    // 예산 사용률 계산
    double GetUtilizationRate(int64_t actualAmount) const {
        if (Data.BudgetAmount == 0) return 0.0;
        return static_cast<double>(actualAmount) / Data.BudgetAmount * 100.0;
    }

    // 예산 초과 확인
    bool IsOverBudget(int64_t actualAmount) const {
        return actualAmount > Data.BudgetAmount;
    }

    // 남은 예산 계산
    int64_t GetRemainingBudget(int64_t actualAmount) const {
        return Data.BudgetAmount - actualAmount;
    }
};
