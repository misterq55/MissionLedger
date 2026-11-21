#pragma once
// CLI 값 파싱과 표시 헬퍼 (인수 모드와 대화형이 같은 규칙을 쓰도록 한곳에)
#include "MLDefine.h"
#include "module/common/format/MLFormat.h"
#include <cctype>
#include <charconv>
#include <ctime>
#include <filesystem>
#include <locale>
#include <sstream>
#include <string>
#include <system_error>

namespace MLCLIUtil
{
// 종료 코드: 0 성공, 1 실행 실패, 2 사용법 오류
constexpr int kExitOk    = 0;
constexpr int kExitFail  = 1;
constexpr int kExitUsage = 2;

// ===== 값 파싱 (문자열 전체가 맞아야 함) =====

// 정수 금액: 쉼표 허용 (50,000), 음수 불가
inline bool parseAmount(const std::string& text, int64_t& out)
{
    std::string digits;
    for (char c : text)
        if (c != ',') digits += c;
    if (digits.empty()) return false;
    const auto result = std::from_chars(digits.data(), digits.data() + digits.size(), out);
    return result.ec == std::errc() && result.ptr == digits.data() + digits.size() && out >= 0;
}

inline bool parseId(const std::string& text, int& out)
{
    if (text.empty()) return false;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), out);
    return result.ec == std::errc() && result.ptr == text.data() + text.size() && out >= 0;
}

// 소수 (외화 금액, 환율): 로케일과 무관하게 '.' 소수점, 양수만
inline bool parsePositiveDecimal(const std::string& text, double& out)
{
    std::string digits;
    for (char c : text)
        if (c != ',') digits += c;
    if (digits.empty()) return false;
    std::istringstream stream(digits);
    stream.imbue(std::locale::classic());
    stream >> out;
    return stream && stream.peek() == std::char_traits<char>::eof() && out > 0.0;
}

inline bool parseType(const std::string& text, E_MLTransactionType& out)
{
    if (text == "income" || text == "i" || text == "수입") { out = E_MLTransactionType::Income; return true; }
    if (text == "expense" || text == "e" || text == "지출") { out = E_MLTransactionType::Expense; return true; }
    return false;
}

// YYYY-MM-DD
inline bool isValidDate(const std::string& text)
{
    if (text.size() != 10 || text[4] != '-' || text[7] != '-') return false;
    for (size_t i : {0, 1, 2, 3, 5, 6, 8, 9})
        if (!std::isdigit(static_cast<unsigned char>(text[i]))) return false;
    const int month = std::stoi(text.substr(5, 2));
    const int day = std::stoi(text.substr(8, 2));
    return month >= 1 && month <= 12 && day >= 1 && day <= 31;
}

inline std::string today()
{
    const std::time_t now = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    char buffer[11];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &local);
    return buffer;
}

inline std::string typeName(E_MLTransactionType type)
{
    return type == E_MLTransactionType::Income ? "수입" : "지출";
}

inline std::string won(int64_t amount)
{
    return FMLFormat::FormatAmount(amount) + "원";
}

// 한 줄 요약: "ID 11 | 지출 | 숙식 / 식비 | 50,000원 (PHP 2,000 @ 25) | 2026-11-21"
inline std::string describeTransaction(const FMLTransactionData& data)
{
    std::string text = "ID " + std::to_string(data.TransactionId) + " | " + typeName(data.Type) + " | " + data.Category;
    if (!data.Item.empty()) text += " / " + data.Item;
    text += " | " + won(data.Amount);
    if (data.UseExchangeRate)
        text += " (" + data.Currency + " " + FMLFormat::FormatDecimal(data.OriginalAmount, 2)
            + " @ " + FMLFormat::FormatDecimal(data.ExchangeRate, 4) + ")";
    text += " | " + data.DateTime;
    return text;
}

inline std::string describeBudget(const FMLBudgetData& data)
{
    std::string text = "ID " + std::to_string(data.BudgetId) + " | " + typeName(data.Type) + " | " + data.Category;
    if (!data.Item.empty()) text += " / " + data.Item;
    text += " | " + won(data.BudgetAmount);
    return text;
}

// .ml 옆에 기본 PDF 이름: <폴더>/<파일이름><suffix>
inline std::string defaultOutputPath(const std::string& mlPath, const std::string& suffix)
{
    const std::filesystem::path path = std::filesystem::u8path(mlPath);
    const std::filesystem::path out = path.parent_path() / std::filesystem::u8path(path.stem().u8string() + suffix);
    return out.u8string();
}
}
