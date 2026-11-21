// 대화형 모드: 인수 모드와 같은 명령을 받고, 열린 파일을 <파일> 자리에 끼우며, 빠진 값을 차례로 묻는다
// 설계: docs/CLI_DESIGN.md §5. 명령 실행은 dispatch() (MLCLIViewCommands.cpp)를 그대로 쓴다

#include "MLCLIView.h"
#include "MLCLIUtil.h"
#include "module/common/holder/MLMVCHolder.h"
#include "interface/IMLController.h"
#include "module/common/currency/MLCurrency.h"

#include <algorithm>
#include <set>

using namespace MLCLIUtil;

namespace
{
    void trim(std::string& text)
    {
        const char* spaces = " \t\r\n";
        const size_t first = text.find_first_not_of(spaces);
        if (first == std::string::npos) { text.clear(); return; }
        text = text.substr(first, text.find_last_not_of(spaces) - first + 1);
    }

    // 공백으로 나누되 "..." 또는 '...' 안의 공백은 유지
    bool tokenize(const std::string& line, std::vector<std::string>& out, std::string& error)
    {
        out.clear();
        std::string current;
        bool inToken = false;
        char quote = 0;
        for (const char c : line) {
            if (quote) {
                if (c == quote) quote = 0;
                else current += c;
                continue;
            }
            if (c == '"' || c == '\'') { quote = c; inToken = true; continue; }
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                if (inToken) { out.push_back(current); current.clear(); inToken = false; }
                continue;
            }
            current += c;
            inToken = true;
        }
        if (quote) { error = "따옴표가 닫히지 않았습니다"; return false; }
        if (inToken) out.push_back(current);
        return true;
    }

    std::string fileName(const std::string& path)
    {
        return std::filesystem::u8path(path).filename().u8string();
    }

    std::string amountText(double value)
    {
        return FMLFormat::FormatDecimalPlain(value, 6);   // 옵션 값으로도 넘기므로 쉼표 없이
    }

    // 값 있는 필드 옵션이 하나라도 있는지 (id 제외): 없으면 모든 항목을 차례로 묻는다
    bool hasFieldOptions(const FMLCLIArgs& args)
    {
        for (const auto& pair : args.Values)
            if (pair.first != "id") return true;
        return false;
    }
}

// ===================== 루프 =====================

void FMLCLIView::Run()
{
    SilentMode = true;      // 인수 모드와 같은 출력 (이벤트 로그 없음)
    Interactive = true;
    InputEnded = false;
    JsonMode = false;

    std::cout << "MissionLedger CLI 대화형 모드. 명령 목록: help, 끝내기: exit" << std::endl;

    if (!ShellFile.empty()) {
        const std::string path = ShellFile;
        ShellFile.clear();
        dispatch({ "info", path });   // 열기 확인 겸 요약 (실패하면 오류만 출력)
        auto controller = FMLMVCHolder::GetInstance().GetController();
        if (controller->GetCurrentFilePath() == path) ShellFile = path;
    }

    std::string line;
    while (!InputEnded) {
        std::cout << (ShellFile.empty() ? "" : "[" + fileName(ShellFile) + "] ") << "> " << std::flush;
        if (!std::getline(std::cin, line)) {
            std::cout << std::endl;
            break;
        }

        std::vector<std::string> tokens;
        std::string error;
        if (!tokenize(line, tokens, error)) {
            std::cerr << "오류: " << error << std::endl;
            continue;
        }
        if (tokens.empty()) continue;

        const std::string& command = tokens[0];
        if (command == "exit" || command == "quit") break;

        if (command == "help" && tokens.size() == 1) {
            printShellHelp();
            continue;
        }

        if (command == "open") {
            if (tokens.size() != 2) { std::cerr << "사용법: open <파일>  (띄어쓰기가 있으면 따옴표로)" << std::endl; continue; }
            auto controller = FMLMVCHolder::GetInstance().GetController();
            if (!controller->OpenFile(tokens[1])) {
                std::cerr << "오류: 파일을 열 수 없습니다: " << tokens[1] << std::endl;
                continue;
            }
            ShellFile = tokens[1];
            std::cout << "열림: " << fileName(ShellFile) << " (거래 " << controller->GetAllTransactionData().size()
                      << "건, 예산 " << controller->GetAllBudgets().size() << "건)" << std::endl;
            continue;
        }

        if (command == "close") {
            if (ShellFile.empty()) std::cout << "열린 파일이 없습니다." << std::endl;
            else std::cout << "닫힘: " << fileName(ShellFile) << std::endl;
            ShellFile.clear();
            continue;
        }

        if (command == "new") {
            // 만들고 바로 연다
            if (tokens.size() == 2 && dispatch(tokens) == kExitOk) {
                ShellFile = tokens[1];
                std::cout << "열림: " << fileName(ShellFile) << std::endl;
            } else if (tokens.size() != 2) {
                std::cerr << "사용법: new <파일>" << std::endl;
            }
            continue;
        }

        dispatch(tokens);
    }

    Interactive = false;
    std::cout << "종료합니다." << std::endl;
}

void FMLCLIView::printShellHelp() const
{
    std::cout << "대화형 모드 명령:\n";
    std::cout << "  " << FMLFormat::FitColumn("open <파일>", 30) << "파일 열기 (이후 명령에서 <파일> 생략)\n";
    std::cout << "  " << FMLFormat::FitColumn("new <파일>", 30) << "새 파일을 만들고 열기\n";
    std::cout << "  " << FMLFormat::FitColumn("close", 30) << "파일 닫기\n";
    std::cout << "  " << FMLFormat::FitColumn("exit", 30) << "끝내기\n";
    std::cout << "\n명령 (인수 모드와 같음, 열린 파일이 있으면 <파일> 생략):\n";
    for (const auto& spec : commandSpecs()) {
        if (spec.Name == "help" || spec.Name == "new" || spec.Name == "shell") continue;
        std::cout << "  " << FMLFormat::FitColumn(spec.Name, 30) << spec.Summary << "\n";
    }
    std::cout << "\n옵션 없이 입력하면(예: add) 항목을 차례로 묻습니다. 질문 중 '취소'를 입력하면 그 명령을 취소합니다.\n";
    std::cout << "명령별 옵션: help <명령>   띄어쓰기가 있는 값은 따옴표로 (예: open \"2026 선교.ml\")\n";
    std::cout.flush();
}

// ===================== 입력 =====================

bool FMLCLIView::ask(const std::string& prompt, std::string& out)
{
    std::cout << prompt << " : " << std::flush;
    if (!std::getline(std::cin, out)) {
        InputEnded = true;
        std::cout << std::endl;
        return false;
    }
    trim(out);
    return out != "취소";
}

bool FMLCLIView::askName(const std::string& label, const std::vector<std::string>& choices, bool optional,
                         const std::string* current, std::string& out)
{
    std::string prompt = label;
    for (size_t i = 0; i < choices.size(); ++i)
        prompt += "  " + std::to_string(i + 1) + ") " + choices[i];
    prompt += choices.empty() ? "  (" : "  (번호 또는 ";
    prompt += "새 이름";
    if (current) prompt += ", Enter=그대로 [" + *current + "]" + (optional ? ", -=지움" : "");
    else if (optional) prompt += ", Enter=없음";
    prompt += ")";

    while (true) {
        std::string answer;
        if (!ask(prompt, answer)) return false;
        if (answer.empty()) {
            if (current) { out = *current; return true; }
            if (optional) { out.clear(); return true; }
            std::cout << "  값이 필요합니다." << std::endl;
            continue;
        }
        if (current && optional && answer == "-") { out.clear(); return true; }
        int index = 0;
        if (!choices.empty() && parseId(answer, index) && index >= 1 && index <= static_cast<int>(choices.size())) {
            out = choices[index - 1];
            return true;
        }
        out = answer;
        return true;
    }
}

bool FMLCLIView::askType(const E_MLTransactionType* current, E_MLTransactionType& out)
{
    const std::string prompt = std::string("유형  1) 수입  2) 지출") + (current ? "  (Enter=그대로 [" + typeName(*current) + "])" : "");
    while (true) {
        std::string answer;
        if (!ask(prompt, answer)) return false;
        if (answer.empty() && current) { out = *current; return true; }
        if (answer == "1") { out = E_MLTransactionType::Income; return true; }
        if (answer == "2") { out = E_MLTransactionType::Expense; return true; }
        if (parseType(answer, out)) return true;
        std::cout << "  1(수입) 또는 2(지출)을 입력하세요." << std::endl;
    }
}

// 파일에 있는 카테고리(category가 비었을 때) 또는 그 카테고리의 항목 이름. 거래와 예산 모두에서
std::vector<std::string> FMLCLIView::knownNames(E_MLTransactionType type, const std::string& category) const
{
    auto controller = FMLMVCHolder::GetInstance().GetController();
    std::set<std::string> names;
    auto collect = [&](E_MLTransactionType itemType, const std::string& itemCategory, const std::string& item) {
        if (itemType != type) return;
        if (category.empty()) { if (!itemCategory.empty()) names.insert(itemCategory); }
        else if (itemCategory == category && !item.empty()) names.insert(item);
    };
    for (const auto& t : controller->GetAllTransactionData()) collect(t.Type, t.Category, t.Item);
    for (const auto& b : controller->GetAllBudgets()) collect(b.Type, b.Category, b.Item);
    return std::vector<std::string>(names.begin(), names.end());
}

bool FMLCLIView::promptId(FMLCLIArgs& args, const std::string& label)
{
    if (args.Has("id")) return true;
    while (true) {
        std::string answer;
        if (!ask(label, answer)) return false;
        int id = 0;
        if (parseId(answer, id)) { args.Values["id"] = answer; return true; }
        std::cout << "  0 이상의 정수를 입력하세요." << std::endl;
    }
}

// ===================== 명령별 질문 =====================

// 반환: 0 계속 실행, 1 취소/실패 (메시지 출력함)
int FMLCLIView::promptForCommand(const FMLCLICommandSpec& spec, FMLCLIArgs& args, bool& confirmBeforeSave)
{
    confirmBeforeSave = false;
    const std::string& name = spec.Name;
    auto cancelled = [this]() {
        if (!InputEnded) std::cout << "취소했습니다." << std::endl;
        return kExitFail;
    };

    // 파일 내용(카테고리 목록, 수정 전 값)이 필요한 명령은 먼저 연다 (명령 실행 때 다시 연다)
    auto controller = FMLMVCHolder::GetInstance().GetController();
    auto openFile = [&]() {
        if (controller->OpenFile(args.Positionals[0])) return true;
        std::cerr << "오류: 파일을 열 수 없습니다: " << args.Positionals[0] << std::endl;
        return false;
    };

    if (name == "show" || name == "delete") {
        return promptId(args, "거래 ID (list로 확인)") ? kExitOk : cancelled();
    }
    if (name == "budget delete") {
        return promptId(args, "예산 ID (budget list로 확인)") ? kExitOk : cancelled();
    }

    if (name == "add") {
        if (!openFile()) return kExitFail;
        confirmBeforeSave = !hasFieldOptions(args);
        return promptTransaction(args, nullptr) ? kExitOk : cancelled();
    }

    if (name == "update") {
        if (!promptId(args, "수정할 거래 ID (list로 확인)")) return cancelled();
        if (hasFieldOptions(args)) return kExitOk;   // 옵션으로 바꿀 값을 줬으면 묻지 않음
        if (!openFile()) return kExitFail;
        int id = 0;
        parseId(*args.Get("id"), id);
        const FMLTransactionData current = controller->GetTransactionData(id);
        if (current.TransactionId < 0) return kExitOk;   // 명령이 "없는 ID" 오류를 낸다
        std::cout << "바꿀 항목만 입력하세요. Enter = 그대로" << std::endl;
        confirmBeforeSave = true;
        if (!promptTransaction(args, &current)) return cancelled();
        if (!hasFieldOptions(args)) { std::cout << "바뀐 것이 없습니다." << std::endl; return kExitFail; }
        return kExitOk;
    }

    if (name == "budget add") {
        if (!openFile()) return kExitFail;
        confirmBeforeSave = !hasFieldOptions(args);
        return promptBudget(args, nullptr) ? kExitOk : cancelled();
    }

    if (name == "budget update") {
        if (!promptId(args, "수정할 예산 ID (budget list로 확인)")) return cancelled();
        if (hasFieldOptions(args)) return kExitOk;
        if (!openFile()) return kExitFail;
        int id = 0;
        parseId(*args.Get("id"), id);
        bool found = false;
        FMLBudgetData current;
        for (const auto& budget : controller->GetAllBudgets())
            if (budget.BudgetId == id) { current = budget; found = true; break; }
        if (!found) return kExitOk;
        std::cout << "바꿀 항목만 입력하세요. Enter = 그대로" << std::endl;
        confirmBeforeSave = true;
        if (!promptBudget(args, &current)) return cancelled();
        if (!hasFieldOptions(args)) { std::cout << "바뀐 것이 없습니다." << std::endl; return kExitFail; }
        return kExitOk;
    }

    if ((name == "export settlement" || name == "export transactions") && !args.Has("out")) {
        const std::string fallback = defaultOutputPath(args.Positionals[0],
            name == "export settlement" ? "-결산보고서.pdf" : "-거래내역서.pdf");
        std::string answer;
        if (!ask("저장할 PDF [" + fallback + "]", answer)) return cancelled();
        if (!answer.empty()) args.Values["out"] = answer;
        return kExitOk;
    }

    return kExitOk;
}

// 거래: current가 없으면 추가 (옵션이 없으면 전부, 있으면 빠진 필수만), 있으면 수정 (전부, 바뀐 것만 옵션으로)
bool FMLCLIView::promptTransaction(FMLCLIArgs& args, const FMLTransactionData* current)
{
    const bool askAll = current || !hasFieldOptions(args);
    auto set = [&](const char* key, const std::string& value) { args.Values[key] = value; };

    // 유형
    E_MLTransactionType type = current ? current->Type : E_MLTransactionType::Expense;
    if (const std::string* given = args.Get("type")) {
        if (!parseType(*given, type)) return true;   // 잘못된 값은 명령이 오류로 알림
    } else if (askAll || !current) {
        if (!askType(current ? &current->Type : nullptr, type)) return false;
        if (!current || type != current->Type) set("type", type == E_MLTransactionType::Income ? "income" : "expense");
    }

    // 카테고리
    std::string category = current ? current->Category : std::string();
    if (const std::string* given = args.Get("category")) {
        category = *given;
    } else {
        if (!askName("카테고리", knownNames(type, ""), false, current ? &current->Category : nullptr, category)) return false;
        if (!current || category != current->Category) set("category", category);
    }

    if (!askAll) {
        // 옵션을 일부 준 추가: 금액만 더 확인 (외화 옵션이 없을 때)
        if (!args.Has("amount") && !args.Has("currency")) {
            while (true) {
                std::string answer;
                if (!ask("금액 (원)", answer)) return false;
                int64_t amount = 0;
                if (parseAmount(answer, amount)) { set("amount", answer); break; }
                std::cout << "  0 이상의 정수를 입력하세요 (예: 50000 또는 50,000)." << std::endl;
            }
        }
        return true;
    }

    // 항목
    std::string item;
    if (!askName("항목", knownNames(type, category), true, current ? &current->Item : nullptr, item)) return false;
    if (current ? item != current->Item : !item.empty()) set("item", item);

    // 금액 (원화) 또는 외화
    const bool wasExchange = current && current->UseExchangeRate;
    bool exchange = wasExchange;
    if (!current) {
        std::string answer;
        if (!ask("외화 거래? [y/N]", answer)) return false;
        exchange = (answer == "y" || answer == "Y" || answer == "yes");
    }

    if (exchange) {
        if (!current) {
            std::string code;
            while (code.empty()) {
                if (!ask("  통화 (예: USD, PHP)", code)) return false;
            }
            set("currency", code);
        }
        const std::string currency = current ? current->Currency : *args.Get("currency");
        double original = current ? current->OriginalAmount : 0.0;
        double rate = current ? current->ExchangeRate : 0.0;
        bool changed = !current;

        auto askDecimal = [&](const std::string& label, double& value) {
            while (true) {
                std::string answer;
                const std::string prompt = label + (current ? " [" + amountText(value) + "]" : "");
                if (!ask(prompt, answer)) return false;
                if (answer.empty() && current) return true;
                double parsed = 0.0;
                if (parsePositiveDecimal(answer, parsed)) { value = parsed; changed = true; return true; }
                std::cout << "  0보다 큰 숫자를 입력하세요 (예: 12.5)." << std::endl;
            }
        };
        if (!askDecimal("  외화 금액 (" + currency + ")", original)) return false;
        if (!askDecimal("  환율 (원/" + currency + ")", rate)) return false;
        if (changed) {
            // 둘 다 넘긴다 (명령이 외화 금액 × 환율로 원화 금액을 다시 계산)
            set("original", amountText(original));
            set("rate", amountText(rate));
            int64_t converted = 0;
            if (FMLCurrency::ConvertToKRW(original, rate, converted))
                std::cout << "  → " << won(converted) << std::endl;
        }
    } else {
        while (true) {
            std::string answer;
            const std::string prompt = std::string("금액 (원)") + (current ? " [" + won(current->Amount) + "]" : "");
            if (!ask(prompt, answer)) return false;
            if (answer.empty() && current) break;
            int64_t amount = 0;
            if (parseAmount(answer, amount)) {
                if (!current || amount != current->Amount) set("amount", answer);
                break;
            }
            std::cout << "  0 이상의 정수를 입력하세요 (예: 50000 또는 50,000)." << std::endl;
        }
    }

    // 날짜
    const std::string dateDefault = current ? current->DateTime : today();
    while (true) {
        std::string answer;
        if (!ask("날짜 [" + dateDefault + "]", answer)) return false;
        if (answer.empty()) {
            if (!current) set("date", dateDefault);
            break;
        }
        if (isValidDate(answer)) {
            if (!current || answer != current->DateTime) set("date", answer);
            break;
        }
        std::cout << "  YYYY-MM-DD 형식으로 입력하세요." << std::endl;
    }

    // 설명, 영수증 번호 (선택)
    auto askText = [&](const std::string& label, const char* key, const std::string* currentValue) {
        std::string answer;
        const std::string prompt = label + (currentValue ? " [" + *currentValue + "] (Enter=그대로, -=지움)" : " (Enter=없음)");
        if (!ask(prompt, answer)) return false;
        if (currentValue) {
            if (answer == "-") { if (!currentValue->empty()) set(key, ""); }
            else if (!answer.empty() && answer != *currentValue) set(key, answer);
        } else if (!answer.empty()) {
            set(key, answer);
        }
        return true;
    };
    if (!askText("설명", "desc", current ? &current->Description : nullptr)) return false;
    if (!askText("영수증 번호", "receipt", current ? &current->ReceiptNumber : nullptr)) return false;
    return true;
}

bool FMLCLIView::promptBudget(FMLCLIArgs& args, const FMLBudgetData* current)
{
    const bool askAll = current || !hasFieldOptions(args);
    auto set = [&](const char* key, const std::string& value) { args.Values[key] = value; };

    E_MLTransactionType type = current ? current->Type : E_MLTransactionType::Expense;
    if (const std::string* given = args.Get("type")) {
        if (!parseType(*given, type)) return true;
    } else {
        if (!askType(current ? &current->Type : nullptr, type)) return false;
        if (!current || type != current->Type) set("type", type == E_MLTransactionType::Income ? "income" : "expense");
    }

    std::string category = current ? current->Category : std::string();
    if (const std::string* given = args.Get("category")) {
        category = *given;
    } else {
        if (!askName("카테고리", knownNames(type, ""), false, current ? &current->Category : nullptr, category)) return false;
        if (!current || category != current->Category) set("category", category);
    }

    if (askAll) {
        std::string item;
        if (!askName("항목", knownNames(type, category), true, current ? &current->Item : nullptr, item)) return false;
        if (current ? item != current->Item : !item.empty()) set("item", item);
    }

    if (!args.Has("amount")) {
        while (true) {
            std::string answer;
            const std::string prompt = std::string("예산 금액 (원)") + (current ? " [" + won(current->BudgetAmount) + "]" : "");
            if (!ask(prompt, answer)) return false;
            if (answer.empty() && current) break;
            int64_t amount = 0;
            if (parseAmount(answer, amount)) {
                if (!current || amount != current->BudgetAmount) set("amount", answer);
                break;
            }
            std::cout << "  0 이상의 정수를 입력하세요." << std::endl;
        }
    }

    if (askAll) {
        std::string answer;
        const std::string prompt = std::string("비고") + (current ? " [" + current->Notes + "] (Enter=그대로, -=지움)" : " (Enter=없음)");
        if (!ask(prompt, answer)) return false;
        if (current) {
            if (answer == "-") { if (!current->Notes.empty()) set("notes", ""); }
            else if (!answer.empty() && answer != current->Notes) set("notes", answer);
        } else if (!answer.empty()) {
            set("notes", answer);
        }
    }
    return true;
}
