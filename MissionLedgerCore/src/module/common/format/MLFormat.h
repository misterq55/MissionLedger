#pragma once

#include <cstdint>
#include <string>

/**
 * @brief Text formatting shared by the GUI, the CLI and PDF export (single source of truth)
 *
 * Results never depend on the system locale: the same input gives the same text on every PC.
 * Strings are UTF-8; the GUI wraps them with wxString::FromUTF8().
 * Currency units ("원") are not added here: callers append them where they are shown.
 */
class FMLFormat
{
public:
    // Thousands separators: 1234567 -> "1,234,567", -5000 -> "-5,000"
    static std::string FormatAmount(int64_t amount);

    // Thousands separators, rounded to maxDecimals, trailing zeros removed down to minDecimals:
    //   (5000, 2) -> "5,000", (12.5, 2) -> "12.5", (12.345, 2) -> "12.35", (24.5, 2, 2) -> "24.50"
    // Decimals are limited to 0..6. Non-finite or huge values (|value| * 10^maxDecimals >= 9e18) give "-"
    static std::string FormatDecimal(double value, int maxDecimals, int minDecimals = 0);

    // Columns a UTF-8 string takes in a monospaced console: Hangul, CJK and fullwidth characters
    // count 2, everything else 1. Invalid bytes count 1 each
    static size_t DisplayWidth(const std::string& utf8);

    // Fixed-width console column: cut at a character boundary to at most width - 1 columns
    // (so one space always separates columns), then pad with spaces to exactly width columns
    static std::string FitColumn(const std::string& utf8, size_t width);
};
