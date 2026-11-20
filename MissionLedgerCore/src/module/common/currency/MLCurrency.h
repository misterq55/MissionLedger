#pragma once

#include <cstdint>

/**
 * @brief Currency calculations shared by the GUI and the CLI (single source of truth)
 */
class FMLCurrency
{
public:
    // Foreign amount x exchange rate (KRW per unit) -> KRW, rounded to the nearest won (half away from zero).
    // Floating-point noise is removed first, so 1.15 x 100 gives 115 (not 114) and 12.345 x 100 gives 1,235.
    // Returns false when an input is not finite or the result is beyond +/-9e12 won.
    static bool ConvertToKRW(double originalAmount, double exchangeRate, int64_t& outAmount);
};
