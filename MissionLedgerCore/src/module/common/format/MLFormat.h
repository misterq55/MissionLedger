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
};
