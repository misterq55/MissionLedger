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

    // Length in bytes of the UTF-8 sequence starting at text[pos] and its code point.
    // An invalid or truncated sequence is taken as one byte (code point 0xFFFD)
    size_t decodeUtf8(const std::string& text, size_t pos, uint32_t& codePoint)
    {
        const unsigned char lead = static_cast<unsigned char>(text[pos]);
        size_t length = 1;
        if (lead < 0x80)      { codePoint = lead; return 1; }
        else if (lead >= 0xF0 && lead < 0xF8) { length = 4; codePoint = lead & 0x07; }
        else if (lead >= 0xE0) { length = 3; codePoint = lead & 0x0F; }
        else if (lead >= 0xC0) { length = 2; codePoint = lead & 0x1F; }
        else                  { codePoint = 0xFFFD; return 1; }

        if (pos + length > text.size()) { codePoint = 0xFFFD; return 1; }
        for (size_t i = 1; i < length; ++i)
        {
            const unsigned char next = static_cast<unsigned char>(text[pos + i]);
            if ((next & 0xC0) != 0x80) { codePoint = 0xFFFD; return 1; }
            codePoint = (codePoint << 6) | (next & 0x3F);
        }
        return length;
    }

    // East Asian Wide / Fullwidth ranges (simplified): 2 columns in a console
    size_t codePointWidth(uint32_t cp)
    {
        const bool wide =
            (cp >= 0x1100 && cp <= 0x115F) ||   // Hangul Jamo (leading)
            (cp >= 0x2E80 && cp <= 0x303E) ||   // CJK radicals, punctuation
            (cp >= 0x3041 && cp <= 0x33FF) ||   // Kana, Hangul compatibility Jamo, CJK symbols
            (cp >= 0x3400 && cp <= 0x4DBF) ||   // CJK extension A
            (cp >= 0x4E00 && cp <= 0x9FFF) ||   // CJK unified ideographs
            (cp >= 0xA000 && cp <= 0xA4CF) ||   // Yi
            (cp >= 0xAC00 && cp <= 0xD7A3) ||   // Hangul syllables
            (cp >= 0xF900 && cp <= 0xFAFF) ||   // CJK compatibility ideographs
            (cp >= 0xFE30 && cp <= 0xFE4F) ||   // CJK compatibility forms
            (cp >= 0xFF00 && cp <= 0xFF60) ||   // Fullwidth forms
            (cp >= 0xFFE0 && cp <= 0xFFE6) ||
            (cp >= 0x1F300 && cp <= 0x1F64F) || // Emoji
            (cp >= 0x1F900 && cp <= 0x1F9FF) ||
            (cp >= 0x20000 && cp <= 0x3FFFD);   // CJK extensions B and later
        return wide ? 2 : 1;
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

namespace
{
    std::string formatDecimal(double value, int maxDecimals, int minDecimals, bool grouping)
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

        const std::string integerDigits = std::to_string(integerPart);
        std::string result = grouping ? groupDigits(integerDigits) : integerDigits;
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
}

std::string FMLFormat::FormatDecimal(double value, int maxDecimals, int minDecimals)
{
    return formatDecimal(value, maxDecimals, minDecimals, true);
}

std::string FMLFormat::FormatDecimalPlain(double value, int maxDecimals)
{
    return formatDecimal(value, maxDecimals, 0, false);
}

size_t FMLFormat::DisplayWidth(const std::string& utf8)
{
    size_t width = 0;
    uint32_t codePoint = 0;
    for (size_t pos = 0; pos < utf8.size();)
    {
        pos += decodeUtf8(utf8, pos, codePoint);
        width += codePointWidth(codePoint);
    }
    return width;
}

std::string FMLFormat::FitColumn(const std::string& utf8, size_t width)
{
    const size_t maxTextWidth = width > 0 ? width - 1 : 0;

    std::string result;
    size_t used = 0;
    uint32_t codePoint = 0;
    for (size_t pos = 0; pos < utf8.size();)
    {
        const size_t length = decodeUtf8(utf8, pos, codePoint);
        const size_t charWidth = codePointWidth(codePoint);
        if (used + charWidth > maxTextWidth)
            break;
        result.append(utf8, pos, length);
        used += charWidth;
        pos += length;
    }
    result.append(width - used, ' ');
    return result;
}
