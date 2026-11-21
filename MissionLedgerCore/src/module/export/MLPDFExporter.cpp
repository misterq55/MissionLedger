#include "MLPDFExporter.h"
#include "MLPDFRow.h"
#include "MLPDFDocument.h"
#include "MLPDFLayout.h"
#include "module/common/format/MLFormat.h"
#include <vector>
#include <algorithm>
#include <map>
#include <set>

using namespace MLPDFLayout;

namespace
{
    // 카테고리 → 항목 → 거래
    using FMLItemGroups = std::map<std::string, std::vector<FMLTransactionData>>;
    using FMLCategoryGroups = std::map<std::string, FMLItemGroups>;

    std::string formatWon(int64_t amount)
    {
        return FMLFormat::FormatAmount(amount) + "원";
    }

    // 2열 표 한 행: 항목 | 금액 (둘 다 굵게)
    void drawLabelAmountRow(FMLPDFDocument& doc, const std::string& label, const std::string& amount, double labelRatio)
    {
        doc.EnsureSpace(RowHeight);
        doc.DrawRow(FMLPDFRow(RowHeight)
            .AddCell(label, TableWidth * labelRatio)
            .AddCell(amount, TableWidth * (1.0 - labelRatio), E_MLPDFAlign::Right),
            MarginX, FontTable, true);
    }

    // 섹션 제목 + 그 아래 최소 내용(firstBlockHeight)이 한 페이지에 오도록 한 뒤 제목 출력
    void drawSection(FMLPDFDocument& doc, const std::string& title, double fontSize, double firstBlockHeight)
    {
        doc.EnsureSpace(fontSize + SectionTitleGap + firstBlockHeight);
        doc.DrawSectionTitle(title, fontSize);
    }

    // ===================== 결산 보고서 =====================

    struct FMLCompareRow {
        std::string Category;
        int64_t Budget;
        int64_t Actual;
    };

    // 예산 vs 실적 비교 표 (헤더, 카테고리 행, 합계 행)
    void drawCompareTable(FMLPDFDocument& doc, const std::string& title, const std::vector<FMLCompareRow>& rows, int64_t totalBudget, int64_t totalActual)
    {
        const double w0 = TableWidth * SettlementCompareRatio[0];
        const double w1 = TableWidth * SettlementCompareRatio[1];
        const double w2 = TableWidth * SettlementCompareRatio[2];

        drawSection(doc, title, FontSection, HeaderHeight + RowHeight);
        doc.DrawRow(FMLPDFRow(HeaderHeight, true)
            .AddCell("카테고리", w0)
            .AddCell("예산", w1, E_MLPDFAlign::Right)
            .AddCell("실적", w2, E_MLPDFAlign::Right),
            MarginX, FontTable, true);

        for (const auto& row : rows) {
            doc.EnsureSpace(RowHeight);
            doc.DrawRow(FMLPDFRow(RowHeight)
                .AddCell(row.Category, w0)
                .AddCell(formatWon(row.Budget), w1, E_MLPDFAlign::Right)
                .AddCell(formatWon(row.Actual), w2, E_MLPDFAlign::Right),
                MarginX, FontTable);
        }

        doc.EnsureSpace(HeaderHeight);
        doc.DrawRow(FMLPDFRow(HeaderHeight, true)
            .AddCell("합계", w0)
            .AddCell(formatWon(totalBudget), w1, E_MLPDFAlign::Right)
            .AddCell(formatWon(totalActual), w2, E_MLPDFAlign::Right),
            MarginX, FontTable, true);
    }

    // 두 맵의 카테고리를 합쳐 비교 행 목록으로
    std::vector<FMLCompareRow> buildCompareRows(const std::map<std::string, int64_t>& budget, const std::map<std::string, int64_t>& actual)
    {
        std::set<std::string> categories;
        for (const auto& entry : budget) categories.insert(entry.first);
        for (const auto& entry : actual) categories.insert(entry.first);

        std::vector<FMLCompareRow> rows;
        for (const auto& category : categories) {
            const auto budgetIt = budget.find(category);
            const auto actualIt = actual.find(category);
            rows.push_back({
                category,
                budgetIt != budget.end() ? budgetIt->second : 0,
                actualIt != actual.end() ? actualIt->second : 0 });
        }
        return rows;
    }

    // 카테고리 → 항목 → 금액을 카테고리 합계로
    std::map<std::string, int64_t> sumByCategory(const std::map<std::string, std::map<std::string, int64_t>>& items)
    {
        std::map<std::string, int64_t> totals;
        for (const auto& category : items) {
            int64_t total = 0;
            for (const auto& item : category.second) total += item.second;
            totals[category.first] = total;
        }
        return totals;
    }

    // ===================== 거래 내역서 =====================

    // 카테고리별 합계 표: [카테고리 이름들 | 총계] / [금액들 | 총액]. 열이 많으면 여러 줄로 나눔
    void drawCategoryTotals(FMLPDFDocument& doc, const std::string& title, const std::map<std::string, int64_t>& totals, int64_t grandTotal, const std::string& totalLabel)
    {
        if (totals.empty()) return;

        const std::vector<std::pair<std::string, int64_t>> list(totals.begin(), totals.end());
        const size_t count = list.size();
        const size_t maxCols = std::max<size_t>(2, static_cast<size_t>(TableWidth / CategoryMinColWidth));
        const size_t perRow = maxCols - 1;   // 마지막 열은 총계/소계
        const double colWidth = (count < perRow) ? TableWidth / (count + 1) : TableWidth / maxCols;

        drawSection(doc, title, FontSubSection, RowHeight * 2);

        for (size_t start = 0; start < count; ) {
            const size_t end = std::min(start + perRow, count);
            const bool isLast = (end == count);

            FMLPDFRow names(RowHeight, true);
            FMLPDFRow amounts(RowHeight);
            int64_t chunkTotal = 0;
            for (size_t i = start; i < end; i++) {
                names.AddCell(list[i].first, colWidth);
                amounts.AddCell(FMLFormat::FormatAmount(list[i].second), colWidth, E_MLPDFAlign::Right);
                chunkTotal += list[i].second;
            }
            names.AddCell(isLast ? totalLabel : "소계", colWidth, E_MLPDFAlign::Left, true);
            amounts.AddCell(FMLFormat::FormatAmount(isLast ? grandTotal : chunkTotal), colWidth, E_MLPDFAlign::Right, true);

            doc.EnsureSpace(RowHeight * 2);
            doc.DrawRow(names, MarginX, FontTable);
            doc.DrawRow(amounts, MarginX, FontTable);

            if (!isLast) doc.MoveDown(ChunkGap);
            start = end;
        }
    }

    void drawTransactionHeader(FMLPDFDocument& doc, const std::string headers[6])
    {
        FMLPDFRow row(HeaderHeight, true);
        for (int i = 0; i < 6; i++)
            row.AddCell(headers[i], TransactionCols[i], i == 3 ? E_MLPDFAlign::Right : E_MLPDFAlign::Left);
        doc.DrawRow(row, MarginX, FontTable, true);
    }

    // 거래 한 행에 그릴 내용 (높이 계산을 위해 미리 준비)
    struct FMLTransactionRowLayout {
        std::vector<std::string> Description;
        std::vector<FMLPDFTextLine> Amount;   // 금액 + (외화면) 작은 환율 정보
        std::string Date;
        std::string Receipt;
        double Height;
    };

    FMLTransactionRowLayout layoutTransaction(const FMLPDFDocument& doc, const FMLTransactionData& transaction)
    {
        FMLTransactionRowLayout row;
        row.Description = doc.WrapText(transaction.Description, TransactionCols[2] - CellPadX * 2, false, FontTable);

        row.Amount.push_back({formatWon(transaction.Amount), FontTable, false, InkText});
        if (transaction.UseExchangeRate && transaction.Currency != "KRW") {
            // 외화 금액: 소수 둘째 자리까지 (센트), 환율: 넷째 자리까지
            row.Amount.push_back({
                "(" + transaction.Currency + " " + FMLFormat::FormatDecimal(transaction.OriginalAmount, 2)
                    + " @ " + FMLFormat::FormatDecimal(transaction.ExchangeRate, 4) + ")",
                FontNote, false, InkNote });
        }

        row.Date = transaction.DateTime.substr(0, 10);
        row.Receipt = transaction.ReceiptNumber;

        double amountHeight = 0.0;
        for (const auto& line : row.Amount) amountHeight += line.Size;
        amountHeight += (row.Amount.size() - 1) * LineGap;

        const double contentHeight = std::max(FMLPDFDocument::LinesHeight(row.Description.size(), FontTable), amountHeight);
        row.Height = std::max(RowHeight, contentHeight + CellPadY * 2);
        return row;
    }

    // 카테고리 그룹 (카테고리 셀과 항목 셀은 병합, 페이지가 넘어가면 나눠 그리고 헤더 반복)
    void drawCategoryGroup(FMLPDFDocument& doc, const std::string& category, const FMLItemGroups& itemGroups, const std::string headers[6])
    {
        if (itemGroups.empty()) return;

        const double* cols = TransactionCols;
        const double xCategory = MarginX;
        const double xItem     = xCategory + cols[0];
        const double xDesc     = xItem + cols[1];
        const double xAmount   = xDesc + cols[2];
        const double xDate     = xAmount + cols[3];
        const double xReceipt  = xDate + cols[4];

        const std::vector<std::string> categoryLines = doc.WrapText(category, cols[0] - CellPadX * 2, false, FontTable);

        // 병합 셀 한 조각 (위쪽 top ~ 현재 Y) 그리기
        auto drawMergedCell = [&](double x, double width, double top, const std::vector<std::string>& lines) {
            const double bottom = doc.GetY();
            const double height = top - bottom;
            if (height <= 0) return;
            doc.StrokeRect(x, bottom, width, height);
            doc.DrawCellText(lines, x, bottom, width, height, E_MLPDFAlign::Left, false, FontTable);
        };

        // 1) 행 높이 미리 계산: 항목 이름이 그 거래 행들보다 길면 항목의 마지막 행을,
        //    카테고리 이름이 카테고리 전체보다 길면 카테고리의 마지막 행을 늘린다
        struct FMLItemLayout {
            std::vector<std::string> Lines;
            std::vector<FMLTransactionRowLayout> Rows;
        };
        std::vector<FMLItemLayout> items;
        double categoryRowsHeight = 0.0;

        for (const auto& itemGroup : itemGroups) {
            FMLItemLayout item;
            item.Lines = doc.WrapText(itemGroup.first, cols[1] - CellPadX * 2, false, FontTable);

            double rowsHeight = 0.0;
            for (const auto& transaction : itemGroup.second) {
                item.Rows.push_back(layoutTransaction(doc, transaction));
                rowsHeight += item.Rows.back().Height;
            }

            const double itemHeight = FMLPDFDocument::LinesHeight(item.Lines.size(), FontTable) + CellPadY * 2;
            if (rowsHeight < itemHeight && !item.Rows.empty()) {
                item.Rows.back().Height += itemHeight - rowsHeight;
                rowsHeight = itemHeight;
            }

            categoryRowsHeight += rowsHeight;
            items.push_back(std::move(item));
        }

        const double categoryHeight = FMLPDFDocument::LinesHeight(categoryLines.size(), FontTable) + CellPadY * 2;
        if (categoryRowsHeight < categoryHeight && !items.back().Rows.empty())
            items.back().Rows.back().Height += categoryHeight - categoryRowsHeight;

        // 2) 그리기
        double categoryTop = doc.GetY();

        for (const auto& item : items) {
            const std::vector<std::string>& itemLines = item.Lines;
            const std::vector<FMLTransactionRowLayout>& rows = item.Rows;

            double itemTop = doc.GetY();

            for (const auto& row : rows) {
                // 페이지 넘김: 지금까지의 병합 셀 조각을 닫고, 새 페이지에 헤더 반복
                if (doc.GetY() - row.Height < MarginBottom) {
                    drawMergedCell(xItem, cols[1], itemTop, itemLines);
                    drawMergedCell(xCategory, cols[0], categoryTop, categoryLines);
                    doc.NewPage();
                    drawTransactionHeader(doc, headers);
                    categoryTop = itemTop = doc.GetY();
                }

                const double bottom = doc.GetY() - row.Height;
                doc.StrokeRect(xDesc, bottom, cols[2], row.Height);
                doc.StrokeRect(xAmount, bottom, cols[3], row.Height);
                doc.StrokeRect(xDate, bottom, cols[4], row.Height);
                doc.StrokeRect(xReceipt, bottom, cols[5], row.Height);

                doc.DrawCellText(row.Description, xDesc, bottom, cols[2], row.Height, E_MLPDFAlign::Left, false, FontTable);
                doc.DrawCellText(row.Amount, xAmount, bottom, cols[3], row.Height, E_MLPDFAlign::Right);
                doc.DrawCellText({row.Date}, xDate, bottom, cols[4], row.Height, E_MLPDFAlign::Left, false, FontTable);
                doc.DrawCellText({row.Receipt}, xReceipt, bottom, cols[5], row.Height, E_MLPDFAlign::Left, false, FontTable);

                doc.MoveDown(row.Height);
            }

            drawMergedCell(xItem, cols[1], itemTop, itemLines);
        }

        drawMergedCell(xCategory, cols[0], categoryTop, categoryLines);
    }

    // 수입 또는 지출 거래 표 전체
    void drawTransactionSection(FMLPDFDocument& doc, const std::string& title, const std::string& amountHeader, const std::vector<FMLTransactionData>& transactions)
    {
        if (transactions.empty()) return;

        const std::string headers[6] = {"구분", "항목", "내용", amountHeader, "년월일", "영수증번호"};

        FMLCategoryGroups groups;
        for (const auto& transaction : transactions)
            groups[transaction.Category][transaction.Item].push_back(transaction);

        drawSection(doc, title, FontSection, HeaderHeight + RowHeight);
        drawTransactionHeader(doc, headers);

        for (const auto& group : groups)
            drawCategoryGroup(doc, group.first, group.second, headers);
    }
}

// 결산 보고서 PDF 생성
bool FMLPDFExporter::ExportSettlement(
    const FMLSettlmentData& data,
    const std::string& filePath)
{
    try {
        FMLPDFDocument doc;
        if (!doc.Open(filePath)) return false;

        doc.DrawTitle(data.Title, data.Subtitle);

        // === 1. 요약: 예산 ===
        drawLabelAmountRow(doc, "총 수입 (예산)", formatWon(data.TotalIncome), SettlementLabelRatio);
        drawLabelAmountRow(doc, "총 지출 (예산)", formatWon(data.TotalExpense), SettlementLabelRatio);
        doc.MoveDown(TableGap);

        // === 1. 요약: 실적 ===
        drawLabelAmountRow(doc, "총 실수입", formatWon(data.TotalActualIncome), SettlementLabelRatio);
        drawLabelAmountRow(doc, "총 실지출", formatWon(data.TotalActualExpense), SettlementLabelRatio);
        drawLabelAmountRow(doc, "잔액", formatWon(data.TotalBalance), SettlementLabelRatio);
        doc.MoveDown(SectionGap);

        // === 2. 환율 정보 ===
        if (!data.ExchangeRates.empty()) {
            const double w0 = TableWidth * SettlementLabelRatio;
            const double w1 = TableWidth - w0;

            drawSection(doc, "환율 정보", FontSection, HeaderHeight + RowHeight);
            doc.DrawRow(FMLPDFRow(HeaderHeight, true)
                .AddCell("통화", w0)
                .AddCell("평균 환율", w1, E_MLPDFAlign::Right),
                MarginX, FontTable, true);

            for (const auto& rate : data.ExchangeRates) {
                doc.EnsureSpace(RowHeight);
                doc.DrawRow(FMLPDFRow(RowHeight)
                    .AddCell(rate.first, w0)
                    .AddCell(FMLFormat::FormatDecimal(rate.second, 2, 2) + "원", w1, E_MLPDFAlign::Right),
                    MarginX, FontTable);
            }
            doc.MoveDown(SectionGap);
        }

        // === 3. 수입 비교 (예산 vs 실적) ===
        drawCompareTable(doc, "수입 비교",
            buildCompareRows(data.BudgetIncomeCategories, data.ActualIncomeCategories),
            data.TotalIncome, data.TotalActualIncome);
        doc.MoveDown(SectionGap);

        // === 4. 지출 비교 (예산 vs 실적) ===
        drawCompareTable(doc, "지출 비교",
            buildCompareRows(sumByCategory(data.BudgetExpenseItems), sumByCategory(data.ActualExpenseItems)),
            data.TotalExpense, data.TotalActualExpense);

        doc.Close();
        return true;
    }
    catch (...) {
        return false;
    }
}

// 거래 내역서 PDF 생성 (개별 거래 내역 표시)
bool FMLPDFExporter::ExportTransactionList(
    const std::vector<FMLTransactionData>& transactions,
    const std::string& subtitle,
    const std::string& filePath)
{
    try {
        FMLPDFDocument doc;
        if (!doc.Open(filePath)) return false;

        std::vector<FMLTransactionData> incomeList;
        std::vector<FMLTransactionData> expenseList;
        int64_t totalIncome = 0;
        int64_t totalExpense = 0;
        std::map<std::string, int64_t> incomeCategoryTotals;
        std::map<std::string, int64_t> expenseCategoryTotals;

        for (const auto& transaction : transactions) {
            if (transaction.Type == E_MLTransactionType::Income) {
                incomeList.push_back(transaction);
                totalIncome += transaction.Amount;
                incomeCategoryTotals[transaction.Category] += transaction.Amount;
            } else if (transaction.Type == E_MLTransactionType::Expense) {
                expenseList.push_back(transaction);
                totalExpense += transaction.Amount;
                expenseCategoryTotals[transaction.Category] += transaction.Amount;
            }
        }

        doc.DrawTitle("거래 내역서", subtitle);

        // === 합계 ===
        drawSection(doc, "합계", FontSection, RowHeight * 3);
        drawLabelAmountRow(doc, "총 수입", formatWon(totalIncome), SummaryLabelRatio);
        drawLabelAmountRow(doc, "총 지출", formatWon(totalExpense), SummaryLabelRatio);
        drawLabelAmountRow(doc, "남은 금액", formatWon(totalIncome - totalExpense), SummaryLabelRatio);
        doc.MoveDown(SectionGap);

        // === 카테고리별 합계 ===
        drawCategoryTotals(doc, "수입 카테고리별", incomeCategoryTotals, totalIncome, "총 수입");
        if (!incomeCategoryTotals.empty() && !expenseCategoryTotals.empty())
            doc.MoveDown(TableGap);
        drawCategoryTotals(doc, "지출 카테고리별", expenseCategoryTotals, totalExpense, "총 지출");
        doc.MoveDown(SectionGap);

        // === 수입 / 지출 거래 ===
        drawTransactionSection(doc, "수입", "수입금액", incomeList);
        if (!incomeList.empty() && !expenseList.empty())
            doc.MoveDown(SectionGap);
        drawTransactionSection(doc, "지출", "지출금액", expenseList);

        doc.Close();
        return true;
    }
    catch (...) {
        return false;
    }
}

void FMLPDFExporter::drawHeader(void* pdfWriter, const std::string& title)
{
    // TODO: 헤더 그리기 구현
}

void FMLPDFExporter::drawFooter(void* pdfWriter)
{
    // TODO: 푸터 그리기 구현
}
