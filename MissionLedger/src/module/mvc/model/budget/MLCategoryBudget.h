#pragma once

#include "MLDefine.h"

// 카테고리 예산 Entity (Data-Oriented Design)
class FMLCategoryBudget {
private:
    FMLItemBudgetData Data;

public:
    FMLCategoryBudget() {}

    // DTO 접근
    const FMLItemBudgetData& GetData() const { return Data; }
    void SetData(const FMLItemBudgetData& data) { Data = data; }

    // 비즈니스 로직
    bool IsValid() const {
        return !Data.Category.empty() &&
               Data.BudgetAmount >= 0;
    }

    // 예산 사용률 계산
    double GetBudgetUtilizationRate(int64_t actualExpense) const {
        if (Data.BudgetAmount == 0) return 0.0;
        return static_cast<double>(actualExpense) / Data.BudgetAmount * 100.0;
    }

    // 예산 초과 확인
    bool IsOverBudget(int64_t actualExpense) const {
        return actualExpense > Data.BudgetAmount;
    }
};
