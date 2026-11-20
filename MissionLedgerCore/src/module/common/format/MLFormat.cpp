#include "MLFormat.h"

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
