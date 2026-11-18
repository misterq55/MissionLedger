#include "MLTransaction.h"

FMLTransaction::FMLTransaction()
{
    // data_는 기본값으로 초기화됨
}

// Data 접근
const FMLTransactionData& FMLTransaction::GetData() const
{
    return data_;
}

void FMLTransaction::SetData(const FMLTransactionData& data)
{
    data_ = data;
}

// 비즈니스 로직
bool FMLTransaction::MatchesFilter(const FMLFilterCriteria& criteria) const
{
    // 거래 유형 필터
    if (criteria.UseTypeFilter && data_.Type != criteria.TypeFilter)
    {
        return false;
    }

    // 날짜 범위 필터
    if (criteria.UseDateRangeFilter)
    {
        if (data_.DateTime < criteria.StartDate || data_.DateTime > criteria.EndDate)
        {
            return false;
        }
    }

    // 카테고리 필터
    if (criteria.UseCategoryFilter && data_.Category != criteria.CategoryFilter)
    {
        return false;
    }

    // 금액 범위 필터
    if (criteria.UseAmountRangeFilter)
    {
        if (data_.Amount < criteria.MinAmount || data_.Amount > criteria.MaxAmount)
        {
            return false;
        }
    }

    // 검색어 필터
    if (criteria.UseTextSearch && !matchesSearchText(criteria))
    {
        return false;
    }

    return true;
}

bool FMLTransaction::IsValid() const
{
    // 기본 유효성 검사
    if (data_.Amount <= 0)
    {
        return false;
    }

    if (data_.Category.empty())
    {
        return false;
    }

    return true;
}

// Private helpers
bool FMLTransaction::matchesSearchText(const FMLFilterCriteria& criteria) const
{
    // 검색 필드가 하나라도 선택되었는지 확인
    if (!criteria.SearchInItem && !criteria.SearchInDescription && !criteria.SearchInReceipt)
    {
        return true;  // 검색 필드 미선택 시 모두 포함
    }

    if (criteria.SearchInDescription && data_.Description.find(criteria.SearchText) != std::string::npos)
    {
        return true;
    }

    if (criteria.SearchInItem && data_.Item.find(criteria.SearchText) != std::string::npos)
    {
        return true;
    }

    if (criteria.SearchInReceipt && data_.ReceiptNumber.find(criteria.SearchText) != std::string::npos)
    {
        return true;
    }

    return false;
}
