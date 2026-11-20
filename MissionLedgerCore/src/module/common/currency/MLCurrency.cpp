#include "MLCurrency.h"

#include <cmath>

bool FMLCurrency::ConvertToKRW(double originalAmount, double exchangeRate, int64_t& outAmount)
{
    const double product = originalAmount * exchangeRate;
    // 9e12 won keeps product * 1e6 below 2^53, where doubles still hold every integer exactly
    if (!std::isfinite(product) || std::fabs(product) >= 9.0e12)
        return false;

    // Snap to 6 decimals before rounding to won: 1234.4999999999998 (= 12.345 x 100) becomes 1234.5
    const double cleaned = std::round(product * 1.0e6) / 1.0e6;
    outAmount = static_cast<int64_t>(std::llround(cleaned));
    return true;
}
