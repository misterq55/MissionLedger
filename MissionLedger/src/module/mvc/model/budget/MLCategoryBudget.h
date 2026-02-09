#pragma once

#include "MLDefine.h"

// 카테고리 예산 Entity (Data-Oriented Design)
class FMLCategoryBudget {
private:
    FMLCategoryBudgetData Data;

public:
    FMLCategoryBudget() {}

    // DTO 접근
    const FMLCategoryBudgetData& GetData() const { return Data; }
    void SetData(const FMLCategoryBudgetData& data) { Data = data; }

    // 비즈니스 로직
    bool IsValid() const {
        return !Data.Category.empty() &&
               Data.IncomeAmount >= 0 &&
               Data.ExpenseAmount >= 0;
    }

    // 예산 사용률 계산
    double GetIncomeUtilizationRate(int64_t actualIncome) const {
        if (Data.IncomeAmount == 0) return 0.0;
        return static_cast<double>(actualIncome) / Data.IncomeAmount * 100.0;
    }

    double GetExpenseUtilizationRate(int64_t actualExpense) const {
        if (Data.ExpenseAmount == 0) return 0.0;
        return static_cast<double>(actualExpense) / Data.ExpenseAmount * 100.0;
    }

    // 예산 초과 확인
    bool IsIncomeOverBudget(int64_t actualIncome) const {
        return actualIncome > Data.IncomeAmount;
    }

    bool IsExpenseOverBudget(int64_t actualExpense) const {
        return actualExpense > Data.ExpenseAmount;
    }
};
