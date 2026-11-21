// 명령어 모드: MissionLedgerCLI <명령> <파일.ml> [옵션]
// 명령과 옵션은 commandSpecs() 표 한곳에서 정의하고, 파싱/검증/도움말이 모두 이 표를 쓴다
// 설계: docs/CLI_DESIGN.md §3

#include "MLCLIView.h"
#include "module/common/holder/MLMVCHolder.h"
#include "interface/IMLController.h"
#include "module/common/format/MLFormat.h"
#include "module/common/currency/MLCurrency.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <ctime>
#include <filesystem>
#include <locale>
#include <set>
#include <sstream>
#include <system_error>

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#else
    #include <unistd.h>
#endif

namespace
{
    // 도움말과 오류 메시지에 쓰는 프로그램 이름 (CLI_DESIGN.md §2: 실행 파일 이름을 mledger로 바꿀 때 함께 변경)
    const char* const kProgram = "MissionLedgerCLI";

    constexpr int kExitOk    = 0;
    constexpr int kExitFail  = 1;
    constexpr int kExitUsage = 2;

    // ===== 값 파싱 (문자열 전체가 맞아야 함) =====

    // 정수 금액: 쉼표 허용 (50,000), 음수 불가
    bool parseAmount(const std::string& text, int64_t& out)
    {
        std::string digits;
        for (char c : text)
            if (c != ',') digits += c;
        if (digits.empty()) return false;
        const auto result = std::from_chars(digits.data(), digits.data() + digits.size(), out);
        return result.ec == std::errc() && result.ptr == digits.data() + digits.size() && out >= 0;
    }

    bool parseId(const std::string& text, int& out)
    {
        if (text.empty()) return false;
        const auto result = std::from_chars(text.data(), text.data() + text.size(), out);
        return result.ec == std::errc() && result.ptr == text.data() + text.size() && out >= 0;
    }

    // 소수 (외화 금액, 환율): 로케일과 무관하게 '.' 소수점, 양수만
    bool parsePositiveDecimal(const std::string& text, double& out)
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

    bool parseType(const std::string& text, E_MLTransactionType& out)
    {
        if (text == "income" || text == "i" || text == "수입") { out = E_MLTransactionType::Income; return true; }
        if (text == "expense" || text == "e" || text == "지출") { out = E_MLTransactionType::Expense; return true; }
        return false;
    }

    // YYYY-MM-DD
    bool isValidDate(const std::string& text)
    {
        if (text.size() != 10 || text[4] != '-' || text[7] != '-') return false;
        for (size_t i : {0, 1, 2, 3, 5, 6, 8, 9})
            if (!std::isdigit(static_cast<unsigned char>(text[i]))) return false;
        const int month = std::stoi(text.substr(5, 2));
        const int day = std::stoi(text.substr(8, 2));
        return month >= 1 && month <= 12 && day >= 1 && day <= 31;
    }

    std::string today()
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

    bool stdinIsTerminal()
    {
    #ifdef _WIN32
        // _isatty는 NUL 장치도 참으로 보므로 실제 콘솔인지로 판단
        DWORD mode = 0;
        return GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode) != 0;
    #else
        return isatty(STDIN_FILENO) != 0;
    #endif
    }

    std::string typeName(E_MLTransactionType type)
    {
        return type == E_MLTransactionType::Income ? "수입" : "지출";
    }

    std::string won(int64_t amount)
    {
        return FMLFormat::FormatAmount(amount) + "원";
    }

    // 한 줄 요약: "ID 11 | 지출 | 숙식 / 식비 | 50,000원 (PHP 2,000 @ 25) | 2026-11-21"
    std::string describeTransaction(const FMLTransactionData& data)
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

    std::string describeBudget(const FMLBudgetData& data)
    {
        std::string text = "ID " + std::to_string(data.BudgetId) + " | " + typeName(data.Type) + " | " + data.Category;
        if (!data.Item.empty()) text += " / " + data.Item;
        text += " | " + won(data.BudgetAmount);
        return text;
    }

    // .ml 옆에 기본 PDF 이름: <폴더>/<파일이름><suffix>
    std::string defaultOutputPath(const std::string& mlPath, const std::string& suffix)
    {
        const std::filesystem::path path = std::filesystem::u8path(mlPath);
        const std::filesystem::path out = path.parent_path() / std::filesystem::u8path(path.stem().u8string() + suffix);
        return out.u8string();
    }

    // ===== 공통 옵션 정의 =====

    FMLCLIOptionSpec opt(const char* name, const char* value, const char* help, bool required = false)
    {
        return FMLCLIOptionSpec{name, value, help, required};
    }

    const char* const kTypeValue = "<income|expense>";

    std::vector<FMLCLIOptionSpec> transactionFieldOptions(bool isAdd)
    {
        return {
            opt("type", kTypeValue, "거래 유형 (수입: income, i, 수입 / 지출: expense, e, 지출)", isAdd),
            opt("category", "<이름>", "카테고리", isAdd),
            opt("item", "<이름>", "항목"),
            opt("amount", "<원>", isAdd ? "금액 (원, 정수). 외화 거래는 생략 가능: 외화 금액 × 환율로 계산 (주면 1원 이내로 맞아야 함)"
                                         : "금액 (원, 정수. 외화 거래는 --original/--rate로 수정)"),
            opt("date", "<YYYY-MM-DD>", isAdd ? "날짜 (생략하면 오늘)" : "날짜"),
            opt("desc", "<설명>", "설명"),
            opt("receipt", "<번호>", "영수증 번호"),
            opt("currency", "<코드>", isAdd ? "외화 통화 코드 (USD, PHP 등). --original, --rate와 함께"
                                            : "외화 통화 코드. KRW를 주면 외화 거래를 해제 (--amount 필요)"),
            opt("original", "<외화금액>", "외화 금액 (예: 12.5)"),
            opt("rate", "<환율>", "환율 (원/외화, 예: 1350.5)"),
        };
    }
}

// ===================== 명령 표 =====================

const std::vector<FMLCLIView::FMLCLICommandSpec>& FMLCLIView::commandSpecs()
{
    static const std::vector<FMLCLICommandSpec> specs = [] {
        std::vector<FMLCLICommandSpec> list;

        list.push_back({"help", "[명령]", 0, 2, "도움말 (명령을 주면 그 명령의 옵션과 예)",
            {}, &FMLCLIView::cmdHelp, {"help add", "help budget update"}});

        list.push_back({"new", "<파일>", 1, 1, "새 .ml 파일 만들기 (이미 있으면 거부)",
            {}, &FMLCLIView::cmdNew, {"new \"2026 선교.ml\""}});

        list.push_back({"info", "<파일>", 1, 1, "요약: 거래/예산 건수, 합계, 기간, 카테고리",
            {}, &FMLCLIView::cmdInfo, {"info data.ml"}});

        list.push_back({"list", "<파일>", 1, 1, "거래 목록 (필터 가능)",
            {
                opt("type", kTypeValue, "유형으로 거르기"),
                opt("category", "<이름>", "카테고리로 거르기 (정확히 일치)"),
                opt("from", "<YYYY-MM-DD>", "이 날짜부터"),
                opt("to", "<YYYY-MM-DD>", "이 날짜까지"),
                opt("search", "<글자>", "항목, 설명, 영수증 번호에 이 글자가 들어간 거래"),
            },
            &FMLCLIView::cmdList, {"list data.ml", "list data.ml --type expense --from 2026-01-01 --to 2026-01-31"}});

        list.push_back({"show", "<파일>", 1, 1, "거래 하나의 모든 내용",
            { opt("id", "<ID>", "거래 ID (list에 나옴)", true) },
            &FMLCLIView::cmdShow, {"show data.ml --id 3"}});

        list.push_back({"add", "<파일>", 1, 1, "거래 추가 (추가된 ID를 출력)",
            transactionFieldOptions(true), &FMLCLIView::cmdAdd,
            {"add data.ml --type expense --category 숙식 --item 식비 --amount 50000 --date 2026-01-13",
             "add data.ml --type expense --category 교통 --currency PHP --original 5000 --rate 24.5"}});

        {
            auto options = transactionFieldOptions(false);
            options.insert(options.begin(), opt("id", "<ID>", "수정할 거래 ID", true));
            list.push_back({"update", "<파일>", 1, 1, "거래 수정 (준 항목만 바꿈. 빈 값 \"\"이면 지움)",
                options, &FMLCLIView::cmdUpdate, {"update data.ml --id 3 --amount 55000 --desc \"\""}});
        }

        list.push_back({"delete", "<파일>", 1, 1, "거래 삭제 (터미널에서는 확인, 그 밖에는 --yes 필요)",
            { opt("id", "<ID>", "삭제할 거래 ID", true), opt("yes", "", "확인 없이 삭제") },
            &FMLCLIView::cmdDelete, {"delete data.ml --id 3 --yes"}});

        list.push_back({"budget list", "<파일>", 1, 1, "예산 목록",
            {}, &FMLCLIView::cmdBudgetList, {"budget list data.ml"}});

        list.push_back({"budget add", "<파일>", 1, 1, "예산 추가 (추가된 ID를 출력)",
            {
                opt("type", kTypeValue, "예산 유형", true),
                opt("category", "<이름>", "카테고리", true),
                opt("item", "<이름>", "항목"),
                opt("amount", "<원>", "예산 금액 (원, 정수)", true),
                opt("notes", "<비고>", "비고"),
            },
            &FMLCLIView::cmdBudgetAdd, {"budget add data.ml --type expense --category 숙식 --item 식비 --amount 700000"}});

        list.push_back({"budget update", "<파일>", 1, 1, "예산 수정 (준 항목만 바꿈)",
            {
                opt("id", "<ID>", "수정할 예산 ID (budget list에 나옴)", true),
                opt("type", kTypeValue, "예산 유형"),
                opt("category", "<이름>", "카테고리"),
                opt("item", "<이름>", "항목"),
                opt("amount", "<원>", "예산 금액"),
                opt("notes", "<비고>", "비고"),
            },
            &FMLCLIView::cmdBudgetUpdate, {"budget update data.ml --id 2 --amount 800000"}});

        list.push_back({"budget delete", "<파일>", 1, 1, "예산 삭제 (터미널에서는 확인, 그 밖에는 --yes 필요)",
            { opt("id", "<ID>", "삭제할 예산 ID", true), opt("yes", "", "확인 없이 삭제") },
            &FMLCLIView::cmdBudgetDelete, {"budget delete data.ml --id 2 --yes"}});

        list.push_back({"export settlement", "<파일>", 1, 1, "결산 보고서 PDF",
            { opt("out", "<PDF 경로>", "저장할 PDF (생략하면 .ml 옆에 <파일이름>-결산보고서.pdf)") },
            &FMLCLIView::cmdExportSettlement, {"export settlement data.ml", "export settlement data.ml --out 결산.pdf"}});

        list.push_back({"export transactions", "<파일>", 1, 1, "거래 내역서 PDF",
            { opt("out", "<PDF 경로>", "저장할 PDF (생략하면 .ml 옆에 <파일이름>-거래내역서.pdf)") },
            &FMLCLIView::cmdExportTransactions, {"export transactions data.ml"}});

        list.push_back({"shell", "[파일]", 0, 1, "대화형 모드 (인수 없이 실행해도 같음)",
            {}, &FMLCLIView::cmdShell, {"shell data.ml"}});

        return list;
    }();
    return specs;
}

const FMLCLIView::FMLCLICommandSpec* FMLCLIView::findCommand(const std::string& name)
{
    for (const auto& spec : commandSpecs())
        if (spec.Name == name) return &spec;
    return nullptr;
}

// ===================== 진입점 =====================

int FMLCLIView::RunWithArgs(int argc, char* argv[])
{
    SilentMode = true;
    CurrentCommand = nullptr;

    const std::vector<std::string> tokens(argv + 1, argv + argc);
    if (tokens.empty()) {
        printGeneralHelp(std::cerr);
        return kExitUsage;
    }

    if (tokens[0] == "--help" || tokens[0] == "-h") {
        printGeneralHelp(std::cout);
        return kExitOk;
    }

    // 두 단어 명령(budget add, export settlement)을 먼저 찾는다
    const FMLCLICommandSpec* spec = nullptr;
    size_t consumed = 0;
    if (tokens.size() >= 2) {
        spec = findCommand(tokens[0] + " " + tokens[1]);
        if (spec) consumed = 2;
    }
    if (!spec) {
        spec = findCommand(tokens[0]);
        if (spec) consumed = 1;
    }
    if (!spec) {
        // 하위 명령이 필요한 묶음 (budget, export)
        std::vector<std::string> subcommands;
        for (const auto& candidate : commandSpecs())
            if (candidate.Name.rfind(tokens[0] + " ", 0) == 0)
                subcommands.push_back(candidate.Name.substr(tokens[0].size() + 1));
        if (!subcommands.empty()) {
            std::string message = (tokens.size() >= 2 ? "알 수 없는 하위 명령입니다: " + tokens[0] + " " + tokens[1]
                                                      : tokens[0] + " 다음에 하위 명령이 필요합니다");
            message += " (";
            for (size_t i = 0; i < subcommands.size(); ++i)
                message += (i ? ", " : "") + subcommands[i];
            message += ")";
            return usageError(nullptr, message);
        }
        return usageError(nullptr, "알 수 없는 명령입니다: " + tokens[0]);
    }

    CurrentCommand = spec;

    FMLCLIArgs args;
    const std::vector<std::string> rest(tokens.begin() + consumed, tokens.end());
    const std::string parseError = ParseCLIArgs(rest, spec->Options, args);
    if (!parseError.empty())
        return usageError(spec, parseError);

    if (args.Flags.count("help")) {
        printCommandHelp(std::cout, *spec);
        return kExitOk;
    }

    if (args.Positionals.size() < spec->MinPositionals)
        return usageError(spec, spec->MinPositionals == 1 ? ".ml 파일 경로가 필요합니다" : "인수가 부족합니다");
    if (args.Positionals.size() > spec->MaxPositionals)
        return usageError(spec, "인수가 너무 많습니다: " + args.Positionals.back()
            + " (값에 띄어쓰기가 있으면 따옴표로 묶으세요)");

    for (const auto& option : spec->Options)
        if (option.Required && !args.Has(option.Name))
            return usageError(spec, "--" + option.Name + " 옵션이 필요합니다");

    return (this->*(spec->Handler))(args);
}

// ===================== 도움말 =====================

std::string FMLCLIView::usageLine(const FMLCLICommandSpec& spec)
{
    std::string line = std::string(kProgram) + " " + spec.Name;
    if (!spec.Positionals.empty()) line += " " + spec.Positionals;
    bool hasOptional = false;
    for (const auto& option : spec.Options) {
        if (option.Required)
            line += " --" + option.Name + (option.ValueName.empty() ? "" : " " + option.ValueName);
        else
            hasOptional = true;
    }
    if (hasOptional) line += " [옵션]";
    return line;
}

void FMLCLIView::printGeneralHelp(std::ostream& os) const
{
    os << "MissionLedger CLI\n\n";
    os << "사용법: " << kProgram << " <명령> <파일.ml> [옵션]\n\n";
    os << "명령:\n";
    for (const auto& spec : commandSpecs()) {
        const std::string left = spec.Name + (spec.Positionals.empty() ? "" : " " + spec.Positionals);
        os << "  " << FMLFormat::FitColumn(left, 30) << spec.Summary << "\n";
    }
    os << "\n";
    os << "명령별 옵션과 예: " << kProgram << " help <명령>  (또는 <명령> --help)\n";
    os << "대화형 모드: 인수 없이 실행하거나 " << kProgram << " shell <파일>\n";
    os << "종료 코드: 0 성공, 1 실행 실패 (파일/ID 없음, 저장 실패), 2 사용법 오류\n";
}

void FMLCLIView::printCommandHelp(std::ostream& os, const FMLCLICommandSpec& spec) const
{
    os << "사용법: " << usageLine(spec) << "\n";
    os << "  " << spec.Summary << "\n";
    if (!spec.Options.empty()) {
        os << "\n옵션:\n";
        for (const auto& option : spec.Options) {
            const std::string left = "--" + option.Name + (option.ValueName.empty() ? "" : " " + option.ValueName);
            os << "  " << FMLFormat::FitColumn(left, 26) << (option.Required ? "(필수) " : "") << option.Help << "\n";
        }
    }
    if (!spec.Examples.empty()) {
        os << "\n예:\n";
        for (const auto& example : spec.Examples)
            os << "  " << kProgram << " " << example << "\n";
    }
}

int FMLCLIView::usageError(const FMLCLICommandSpec* spec, const std::string& message) const
{
    std::cerr << "오류: " << message << "\n";
    if (spec) {
        std::cerr << "사용법: " << usageLine(*spec) << "\n";
        std::cerr << "자세히: " << kProgram << " help " << spec->Name << "\n";
    } else {
        std::cerr << "명령 목록: " << kProgram << " help\n";
    }
    std::cerr.flush();
    return kExitUsage;
}

int FMLCLIView::cmdHelp(const FMLCLIArgs& args)
{
    if (args.Positionals.empty()) {
        printGeneralHelp(std::cout);
        return kExitOk;
    }
    std::string name = args.Positionals[0];
    if (args.Positionals.size() == 2) name += " " + args.Positionals[1];
    const FMLCLICommandSpec* spec = findCommand(name);
    if (!spec)
        return usageError(nullptr, "알 수 없는 명령입니다: " + name);
    printCommandHelp(std::cout, *spec);
    return kExitOk;
}

// ===================== 공통 헬퍼 =====================

bool FMLCLIView::openFileForCommand(const std::string& filePath)
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller->OpenFile(filePath)) {
        std::cerr << "오류: 파일을 열 수 없습니다: " << filePath << std::endl;
        return false;
    }
    return true;
}

int FMLCLIView::saveForCommand()
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller->SaveFile()) {
        std::cerr << "오류: 파일 저장에 실패했습니다." << std::endl;
        return kExitFail;
    }
    return kExitOk;
}

// 거래 옵션을 data에 반영. 반환: 0 성공, 2 사용법 오류 (error에 메시지)
int FMLCLIView::applyTransactionOptions(const FMLCLIArgs& args, FMLTransactionData& data, bool isAdd, std::string& error)
{
    if (const std::string* value = args.Get("type")) {
        if (!parseType(*value, data.Type)) { error = "잘못된 유형입니다: " + *value + " (income 또는 expense)"; return kExitUsage; }
    }
    if (const std::string* value = args.Get("category")) {
        if (value->empty()) { error = "카테고리는 비울 수 없습니다"; return kExitUsage; }
        data.Category = *value;
    }
    if (const std::string* value = args.Get("item"))    data.Item = *value;
    if (const std::string* value = args.Get("desc"))    data.Description = *value;
    if (const std::string* value = args.Get("receipt")) data.ReceiptNumber = *value;
    if (const std::string* value = args.Get("date")) {
        if (!isValidDate(*value)) { error = "잘못된 날짜입니다: " + *value + " (YYYY-MM-DD)"; return kExitUsage; }
        data.DateTime = *value;
    } else if (isAdd) {
        data.DateTime = today();
    }

    // 외화: --currency/--original/--rate
    const std::string* currency = args.Get("currency");
    const std::string* original = args.Get("original");
    const std::string* rate = args.Get("rate");
    const std::string* amount = args.Get("amount");
    const bool anyExchange = currency || original || rate;

    if (currency && (*currency == "KRW" || *currency == "krw")) {
        // 외화 해제 (원화 거래로)
        if (original || rate) { error = "--currency KRW와 --original/--rate는 함께 쓸 수 없습니다"; return kExitUsage; }
        if (!amount) { error = "원화 거래로 바꾸려면 --amount가 필요합니다"; return kExitUsage; }
        data.UseExchangeRate = false;
        data.Currency = "KRW";
        data.OriginalAmount = 0.0;
        data.ExchangeRate = 1.0;
    } else if (anyExchange) {
        if (isAdd && !(currency && original && rate)) {
            error = "외화 거래는 --currency, --original, --rate를 모두 주어야 합니다";
            return kExitUsage;
        }
        if (!isAdd && !data.UseExchangeRate && !(currency && original && rate)) {
            error = "원화 거래를 외화 거래로 바꾸려면 --currency, --original, --rate를 모두 주어야 합니다";
            return kExitUsage;
        }

        if (currency) {
            std::string code = *currency;
            std::transform(code.begin(), code.end(), code.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
            data.Currency = code;
        }
        if (original && !parsePositiveDecimal(*original, data.OriginalAmount)) { error = "잘못된 외화 금액입니다: " + *original; return kExitUsage; }
        if (rate && !parsePositiveDecimal(*rate, data.ExchangeRate)) { error = "잘못된 환율입니다: " + *rate; return kExitUsage; }
        data.UseExchangeRate = true;
        if (!FMLCurrency::ConvertToKRW(data.OriginalAmount, data.ExchangeRate, data.Amount)) {
            error = "원화 환산 금액이 범위를 벗어났습니다";
            return kExitUsage;
        }
        // --amount를 함께 주면 확인용: 계산값과 1원 이내여야 함 (저장은 계산값)
        if (amount) {
            int64_t given = 0;
            if (!parseAmount(*amount, given)) { error = "잘못된 금액입니다: " + *amount + " (0 이상의 정수, 원)"; return kExitUsage; }
            if (given - data.Amount > 1 || data.Amount - given > 1) {
                error = "--amount " + won(given) + "이 외화 금액 × 환율 = " + won(data.Amount)
                    + "과 다릅니다 (외화 거래는 --amount를 생략해도 됩니다)";
                return kExitUsage;
            }
        }
        return kExitOk;
    }

    if (amount) {
        if (data.UseExchangeRate) {
            error = "외화 거래의 금액은 --original/--rate로 수정하세요 (원화 거래로 바꾸려면 --currency KRW --amount <원>)";
            return kExitUsage;
        }
        if (!parseAmount(*amount, data.Amount)) { error = "잘못된 금액입니다: " + *amount + " (0 이상의 정수, 원)"; return kExitUsage; }
    } else if (isAdd && !data.UseExchangeRate) {
        error = "--amount 옵션이 필요합니다 (외화 거래면 --currency, --original, --rate)";
        return kExitUsage;
    }
    return kExitOk;
}

int FMLCLIView::applyBudgetOptions(const FMLCLIArgs& args, FMLBudgetData& data, std::string& error)
{
    if (const std::string* value = args.Get("type")) {
        if (!parseType(*value, data.Type)) { error = "잘못된 유형입니다: " + *value + " (income 또는 expense)"; return kExitUsage; }
    }
    if (const std::string* value = args.Get("category")) {
        if (value->empty()) { error = "카테고리는 비울 수 없습니다"; return kExitUsage; }
        data.Category = *value;
    }
    if (const std::string* value = args.Get("item"))  data.Item = *value;
    if (const std::string* value = args.Get("notes")) data.Notes = *value;
    if (const std::string* value = args.Get("amount")) {
        if (!parseAmount(*value, data.BudgetAmount)) { error = "잘못된 금액입니다: " + *value + " (0 이상의 정수, 원)"; return kExitUsage; }
    }
    return kExitOk;
}

// 삭제 확인. 반환: 0 진행, 1 사용자가 취소, 2 터미널이 아닌데 --yes 없음
int FMLCLIView::confirmDelete(const FMLCLIArgs& args, const std::string& what)
{
    if (args.Has("yes")) return kExitOk;
    if (!stdinIsTerminal())
        return usageError(CurrentCommand, "삭제하려면 --yes가 필요합니다 (확인 질문을 할 수 없는 환경)");

    std::cout << what << "\n삭제할까요? [y/N] " << std::flush;
    std::string answer;
    std::getline(std::cin, answer);
    if (answer == "y" || answer == "Y" || answer == "yes") return kExitOk;
    std::cout << "취소했습니다." << std::endl;
    return kExitFail;
}

// ===================== 명령 =====================

int FMLCLIView::cmdShell(const FMLCLIArgs& args)
{
    if (!args.Positionals.empty()) {
        if (!openFileForCommand(args.Positionals[0])) return kExitFail;
        std::cout << "파일 열림: " << args.Positionals[0] << std::endl;
    }
    SilentMode = false;
    Run();
    return kExitOk;
}

int FMLCLIView::cmdNew(const FMLCLIArgs& args)
{
    const std::string& filePath = args.Positionals[0];

    // 이미 있는 파일에 만들면 저장(전체 삭제 후 다시 쓰기)이 기존 거래/예산을 모두 지우므로 거부
    std::error_code ec;
    if (std::filesystem::exists(std::filesystem::u8path(filePath), ec)) {
        std::cerr << "오류: 파일이 이미 있습니다: " << filePath << std::endl;
        std::cerr << "기존 파일은 지우지 않습니다. 다른 이름을 쓰거나 기존 파일을 먼저 옮기세요." << std::endl;
        return kExitFail;
    }

    auto controller = FMLMVCHolder::GetInstance().GetController();
    controller->NewFile();
    if (!controller->SaveFileAs(filePath)) {
        std::cerr << "오류: 파일을 만들 수 없습니다: " << filePath << std::endl;
        return kExitFail;
    }
    std::cout << "새 파일 생성됨: " << filePath << std::endl;
    return kExitOk;
}

int FMLCLIView::cmdInfo(const FMLCLIArgs& args)
{
    const std::string& filePath = args.Positionals[0];
    if (!openFileForCommand(filePath)) return kExitFail;
    auto controller = FMLMVCHolder::GetInstance().GetController();

    const auto transactions = controller->GetAllTransactionData();
    const auto budgets = controller->GetAllBudgets();
    const FMLTransactionSummary summary = controller->GetTransactionSummary();

    int incomeCount = 0;
    std::string firstDate, lastDate;
    std::set<std::string> incomeCategories, expenseCategories;
    for (const auto& t : transactions) {
        if (t.Type == E_MLTransactionType::Income) { ++incomeCount; incomeCategories.insert(t.Category); }
        else expenseCategories.insert(t.Category);
        if (!t.DateTime.empty()) {
            if (firstDate.empty() || t.DateTime < firstDate) firstDate = t.DateTime;
            if (lastDate.empty() || t.DateTime > lastDate) lastDate = t.DateTime;
        }
    }
    int64_t incomeBudget = 0, expenseBudget = 0;
    for (const auto& b : budgets)
        (b.Type == E_MLTransactionType::Income ? incomeBudget : expenseBudget) += b.BudgetAmount;

    auto join = [](const std::set<std::string>& names) {
        std::string text;
        for (const auto& name : names) text += (text.empty() ? "" : ", ") + name;
        return text.empty() ? std::string("-") : text;
    };

    std::cout << "파일: " << filePath << "\n";
    std::cout << "거래: " << transactions.size() << "건 (수입 " << incomeCount << "건, 지출 "
              << (transactions.size() - incomeCount) << "건)\n";
    std::cout << "합계: 수입 " << won(summary.TotalIncome) << ", 지출 " << won(summary.TotalExpense)
              << ", 잔액 " << won(summary.Balance) << "\n";
    std::cout << "기간: " << (firstDate.empty() ? "-" : firstDate + " ~ " + lastDate) << "\n";
    std::cout << "예산: " << budgets.size() << "건 (수입 " << won(incomeBudget) << ", 지출 " << won(expenseBudget) << ")\n";
    std::cout << "수입 카테고리: " << join(incomeCategories) << "\n";
    std::cout << "지출 카테고리: " << join(expenseCategories) << std::endl;
    return kExitOk;
}

int FMLCLIView::cmdList(const FMLCLIArgs& args)
{
    FMLFilterCriteria criteria;
    if (const std::string* value = args.Get("type")) {
        if (!parseType(*value, criteria.TypeFilter))
            return usageError(CurrentCommand, "잘못된 유형입니다: " + *value + " (income 또는 expense)");
        criteria.UseTypeFilter = true;
    }
    if (const std::string* value = args.Get("category")) {
        criteria.UseCategoryFilter = true;
        criteria.CategoryFilter = *value;
    }
    const std::string* from = args.Get("from");
    const std::string* to = args.Get("to");
    if (from && !isValidDate(*from)) return usageError(CurrentCommand, "잘못된 날짜입니다: " + *from + " (YYYY-MM-DD)");
    if (to && !isValidDate(*to))     return usageError(CurrentCommand, "잘못된 날짜입니다: " + *to + " (YYYY-MM-DD)");
    if (from || to) {
        criteria.UseDateRangeFilter = true;
        criteria.StartDate = from ? *from : "0000-00-00";
        // 날짜 뒤에 시간이 붙어 있어도 그날을 포함하도록 '~'(숫자/공백보다 큼)를 붙인다
        criteria.EndDate = (to ? *to : "9999-12-31") + "~";
    }
    if (const std::string* value = args.Get("search")) {
        criteria.UseTextSearch = true;
        criteria.SearchText = *value;
        criteria.SearchInItem = criteria.SearchInDescription = criteria.SearchInReceipt = true;
    }

    if (!openFileForCommand(args.Positionals[0])) return kExitFail;
    auto controller = FMLMVCHolder::GetInstance().GetController();
    DisplayTransactions(controller->GetFilteredTransactionData(criteria));
    return kExitOk;
}

int FMLCLIView::cmdShow(const FMLCLIArgs& args)
{
    int id = 0;
    if (!parseId(*args.Get("id"), id)) return usageError(CurrentCommand, "잘못된 ID입니다: " + *args.Get("id"));
    if (!openFileForCommand(args.Positionals[0])) return kExitFail;

    auto controller = FMLMVCHolder::GetInstance().GetController();
    const FMLTransactionData data = controller->GetTransactionData(id);
    if (data.TransactionId < 0) {
        std::cerr << "오류: ID " << id << "인 거래가 없습니다." << std::endl;
        return kExitFail;
    }
    DisplayTransaction(data);
    return kExitOk;
}

int FMLCLIView::cmdAdd(const FMLCLIArgs& args)
{
    FMLTransactionData data;
    std::string error;
    if (applyTransactionOptions(args, data, true, error) != kExitOk)
        return usageError(CurrentCommand, error);

    if (!openFileForCommand(args.Positionals[0])) return kExitFail;
    auto controller = FMLMVCHolder::GetInstance().GetController();

    LastAddedTransactionId = -1;
    if (!controller->AddTransaction(data) || LastAddedTransactionId < 0) {
        std::cerr << "오류: 거래를 추가하지 못했습니다." << std::endl;
        return kExitFail;
    }
    if (const int result = saveForCommand(); result != kExitOk) return result;

    std::cout << "추가됨: " << describeTransaction(controller->GetTransactionData(LastAddedTransactionId)) << std::endl;
    return kExitOk;
}

int FMLCLIView::cmdUpdate(const FMLCLIArgs& args)
{
    int id = 0;
    if (!parseId(*args.Get("id"), id)) return usageError(CurrentCommand, "잘못된 ID입니다: " + *args.Get("id"));
    if (args.Values.size() == 1)
        return usageError(CurrentCommand, "바꿀 항목이 없습니다 (예: --amount, --desc)");

    if (!openFileForCommand(args.Positionals[0])) return kExitFail;
    auto controller = FMLMVCHolder::GetInstance().GetController();

    FMLTransactionData data = controller->GetTransactionData(id);
    if (data.TransactionId < 0) {
        std::cerr << "오류: ID " << id << "인 거래가 없습니다." << std::endl;
        return kExitFail;
    }

    std::string error;
    if (applyTransactionOptions(args, data, false, error) != kExitOk)
        return usageError(CurrentCommand, error);

    if (!controller->UpdateTransaction(data)) {
        std::cerr << "오류: 거래를 수정하지 못했습니다." << std::endl;
        return kExitFail;
    }
    if (const int result = saveForCommand(); result != kExitOk) return result;

    std::cout << "수정됨: " << describeTransaction(controller->GetTransactionData(id)) << std::endl;
    return kExitOk;
}

int FMLCLIView::cmdDelete(const FMLCLIArgs& args)
{
    int id = 0;
    if (!parseId(*args.Get("id"), id)) return usageError(CurrentCommand, "잘못된 ID입니다: " + *args.Get("id"));
    if (!openFileForCommand(args.Positionals[0])) return kExitFail;
    auto controller = FMLMVCHolder::GetInstance().GetController();

    const FMLTransactionData data = controller->GetTransactionData(id);
    if (data.TransactionId < 0) {
        std::cerr << "오류: ID " << id << "인 거래가 없습니다." << std::endl;
        return kExitFail;
    }
    if (const int result = confirmDelete(args, describeTransaction(data)); result != kExitOk) return result;

    if (!controller->DeleteTransaction(id)) {
        std::cerr << "오류: 거래를 삭제하지 못했습니다." << std::endl;
        return kExitFail;
    }
    if (const int result = saveForCommand(); result != kExitOk) return result;

    std::cout << "삭제됨: " << describeTransaction(data) << std::endl;
    return kExitOk;
}

int FMLCLIView::cmdBudgetList(const FMLCLIArgs& args)
{
    if (!openFileForCommand(args.Positionals[0])) return kExitFail;
    auto controller = FMLMVCHolder::GetInstance().GetController();
    DisplayBudgets(controller->GetAllBudgets());
    return kExitOk;
}

int FMLCLIView::cmdBudgetAdd(const FMLCLIArgs& args)
{
    FMLBudgetData data;
    std::string error;
    if (applyBudgetOptions(args, data, error) != kExitOk)
        return usageError(CurrentCommand, error);
    data.ActualAmount = 0;

    if (!openFileForCommand(args.Positionals[0])) return kExitFail;
    auto controller = FMLMVCHolder::GetInstance().GetController();

    LastAddedBudgetId = -1;
    if (!controller->AddBudget(data) || LastAddedBudgetId < 0) {
        std::cerr << "오류: 예산을 추가하지 못했습니다." << std::endl;
        return kExitFail;
    }
    if (const int result = saveForCommand(); result != kExitOk) return result;

    std::cout << "추가됨: " << describeBudget(controller->GetBudget(LastAddedBudgetId)) << std::endl;
    return kExitOk;
}

int FMLCLIView::cmdBudgetUpdate(const FMLCLIArgs& args)
{
    int id = 0;
    if (!parseId(*args.Get("id"), id)) return usageError(CurrentCommand, "잘못된 ID입니다: " + *args.Get("id"));
    if (args.Values.size() == 1)
        return usageError(CurrentCommand, "바꿀 항목이 없습니다 (예: --amount, --notes)");

    if (!openFileForCommand(args.Positionals[0])) return kExitFail;
    auto controller = FMLMVCHolder::GetInstance().GetController();

    FMLBudgetData data;
    bool found = false;
    for (const auto& budget : controller->GetAllBudgets())
        if (budget.BudgetId == id) { data = budget; found = true; break; }
    if (!found) {
        std::cerr << "오류: ID " << id << "인 예산이 없습니다." << std::endl;
        return kExitFail;
    }

    std::string error;
    if (applyBudgetOptions(args, data, error) != kExitOk)
        return usageError(CurrentCommand, error);

    if (!controller->UpdateBudget(data)) {
        std::cerr << "오류: 예산을 수정하지 못했습니다." << std::endl;
        return kExitFail;
    }
    if (const int result = saveForCommand(); result != kExitOk) return result;

    std::cout << "수정됨: " << describeBudget(controller->GetBudget(id)) << std::endl;
    return kExitOk;
}

int FMLCLIView::cmdBudgetDelete(const FMLCLIArgs& args)
{
    int id = 0;
    if (!parseId(*args.Get("id"), id)) return usageError(CurrentCommand, "잘못된 ID입니다: " + *args.Get("id"));
    if (!openFileForCommand(args.Positionals[0])) return kExitFail;
    auto controller = FMLMVCHolder::GetInstance().GetController();

    FMLBudgetData data;
    bool found = false;
    for (const auto& budget : controller->GetAllBudgets())
        if (budget.BudgetId == id) { data = budget; found = true; break; }
    if (!found) {
        std::cerr << "오류: ID " << id << "인 예산이 없습니다." << std::endl;
        return kExitFail;
    }
    if (const int result = confirmDelete(args, describeBudget(data)); result != kExitOk) return result;

    if (!controller->DeleteBudget(id)) {
        std::cerr << "오류: 예산을 삭제하지 못했습니다." << std::endl;
        return kExitFail;
    }
    if (const int result = saveForCommand(); result != kExitOk) return result;

    std::cout << "삭제됨: " << describeBudget(data) << std::endl;
    return kExitOk;
}

int FMLCLIView::cmdExportSettlement(const FMLCLIArgs& args)
{
    const std::string& filePath = args.Positionals[0];
    const std::string outPath = args.Get("out") ? *args.Get("out") : defaultOutputPath(filePath, "-결산보고서.pdf");

    if (!openFileForCommand(filePath)) return kExitFail;
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller->ExportSettlementToPDF(outPath)) {
        std::cerr << "오류: 결산 보고서를 만들지 못했습니다: " << outPath << std::endl;
        return kExitFail;
    }
    std::cout << "결산 보고서 생성됨: " << outPath << std::endl;
    return kExitOk;
}

int FMLCLIView::cmdExportTransactions(const FMLCLIArgs& args)
{
    const std::string& filePath = args.Positionals[0];
    const std::string outPath = args.Get("out") ? *args.Get("out") : defaultOutputPath(filePath, "-거래내역서.pdf");

    if (!openFileForCommand(filePath)) return kExitFail;
    auto controller = FMLMVCHolder::GetInstance().GetController();
    if (!controller->ExportTransactionListToPDF(outPath)) {
        std::cerr << "오류: 거래 내역서를 만들지 못했습니다: " << outPath << std::endl;
        return kExitFail;
    }
    std::cout << "거래 내역서 생성됨: " << outPath << std::endl;
    return kExitOk;
}
