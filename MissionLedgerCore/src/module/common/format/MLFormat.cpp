#include "MLFormat.h"

#include <algorithm>
#include <cmath>

namespace
{
    // Inserts a comma every three digits of a string of decimal digits
    std::string groupDigits(const std::string& digits)
    {
        std::string result;
        result.reserve(digits.size() + digits.size() / 3);
        for (size_t i = 0; i < digits.size(); ++i)
        {
            if (i > 0 && (digits.size() - i) % 3 == 0)
                result += ',';
            result += digits[i];
        }
        return result;
    }
}

std::string FMLFormat::FormatAmount(int64_t amount)
{
    const bool negative = amount < 0;
    // INT64_MIN cannot be negated as int64_t; the unsigned negation is well defined
    const uint64_t magnitude = negative ? 0 - static_cast<uint64_t>(amount)
                                        : static_cast<uint64_t>(amount);
    const std::string grouped = groupDigits(std::to_string(magnitude));
    return negative ? "-" + grouped : grouped;
}

std::string FMLFormat::FormatDecimal(double value, int maxDecimals, int minDecimals)
{
    maxDecimals = std::clamp(maxDecimals, 0, 6);
    minDecimals = std::clamp(minDecimals, 0, maxDecimals);

    uint64_t scale = 1;
    for (int i = 0; i < maxDecimals; ++i)
        scale *= 10;

    const double scaledValue = std::fabs(value) * static_cast<double>(scale);
    if (!std::isfinite(scaledValue) || scaledValue >= 9.0e18)
        return "-";

    // Integer arithmetic from here on: no locale (decimal point) and no printf rounding differences
    const uint64_t scaled = static_cast<uint64_t>(std::llround(scaledValue));
    const uint64_t integerPart = scaled / scale;
    const uint64_t fractionPart = scaled % scale;

    std::string result = groupDigits(std::to_string(integerPart));
    if (maxDecimals > 0)
    {
        std::string fraction = std::to_string(fractionPart);
        fraction.insert(0, static_cast<size_t>(maxDecimals) - fraction.size(), '0');
        while (static_cast<int>(fraction.size()) > minDecimals && fraction.back() == '0')
            fraction.pop_back();
        if (!fraction.empty())
            result += "." + fraction;
    }

    // No "-0" when a small negative value rounds to zero
    return (value < 0 && scaled != 0) ? "-" + result : result;
}
