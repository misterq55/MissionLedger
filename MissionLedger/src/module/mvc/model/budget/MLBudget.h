#pragma once

#include "MLDefine.h"

// 항목 예산 Entity (Data-Oriented Design)
// 예: Category="항공", Item="항공료 선결제", BudgetAmount=3,000,000
class FMLBudget {
private:
    FMLBudgetData Data;

public:
    FMLBudget() {}

    // DTO 접근
    const FMLBudgetData& GetData() const { return Data; }
    void SetData(const FMLBudgetData& data) { Data = data; }

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
    
    // 필터 확인
    bool MatchesFilter(const FMLFilterCriteria& criteria) const {
        // 거래 유형 필터
        if (criteria.UseTypeFilter && Data.Type != criteria.TypeFilter)
        {
            return false;
        }

        // 카테고리 필터
        if (criteria.UseCategoryFilter && Data.Category != criteria.CategoryFilter)
        {
            return false;
        }

        // 검색어 필터 (Category, Item, Notes에서 검색)
        if (criteria.UseTextSearch)
        {
            bool found = false;

            // Category에서 검색
            if (Data.Category.find(criteria.SearchText) != std::string::npos)
            {
                found = true;
            }

            // Item에서 검색
            if (!found && !Data.Item.empty() &&
                Data.Item.find(criteria.SearchText) != std::string::npos)
            {
                found = true;
            }

            // Notes에서 검색
            if (!found && !Data.Notes.empty() &&
                Data.Notes.find(criteria.SearchText) != std::string::npos)
            {
                found = true;
            }

            if (!found)
            {
                return false;
            }
        }

        return true;
    }
};
