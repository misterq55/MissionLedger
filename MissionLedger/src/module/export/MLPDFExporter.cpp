#include "MLPDFExporter.h"
#include "MLPDFRow.h"
#include "MLPDFDocument.h"
#include "PDFWriter/PageContentContext.h"
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <map>
#include <set>

// 파일 경로에서 확장자 없는 파일명 추출
static std::string extractFilename(const std::string& filePath)
{
    size_t lastSlash = filePath.find_last_of("/\\");
    std::string filename = (lastSlash != std::string::npos) ? filePath.substr(lastSlash + 1) : filePath;
    size_t lastDot = filename.find_last_of('.');
    if (lastDot != std::string::npos)
        filename = filename.substr(0, lastDot);
    return filename;
}

// 금액 포맷 헬퍼 (천 단위 콤마)
static std::string formatAmountHelper(int64_t amount)
{
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << amount;
    return ss.str();
}

// UTF-8 문자열을 지정된 너비에 맞게 여러 줄로 분할
static std::vector<std::string> wrapText(const std::string& text, double maxWidth, double fontSize)
{
    std::vector<std::string> lines;
    if (text.empty()) {
        lines.push_back("");
        return lines;
    }

    const double avgCharWidth = fontSize * 1.1;
    const int maxCharsPerLine = static_cast<int>(maxWidth / avgCharWidth);

    if (maxCharsPerLine <= 0) {
        lines.push_back(text);
        return lines;
    }

    std::string currentLine;
    int charCount = 0;
    size_t i = 0;

    while (i < text.length()) {
        unsigned char c = text[i];
        int charBytes = 1;
        if ((c & 0x80) == 0)         charBytes = 1;
        else if ((c & 0xE0) == 0xC0) charBytes = 2;
        else if ((c & 0xF0) == 0xE0) charBytes = 3;
        else if ((c & 0xF8) == 0xF0) charBytes = 4;

        std::string currentChar = text.substr(i, charBytes);

        if (charCount >= maxCharsPerLine && !currentLine.empty()) {
            lines.push_back(currentLine);
            currentLine.clear();
            charCount = 0;
        }

        currentLine += currentChar;
        charCount++;
        i += charBytes;
    }

    if (!currentLine.empty())
        lines.push_back(currentLine);

    return lines;
}

// 카테고리 그룹 그리기 (Category 셀 병합, 내부에 여러 Item 그룹 포함)
static void drawCategoryGroup(
    FMLPDFDocument& doc,
    const std::string& category,
    const std::map<std::string, std::vector<FMLTransactionData>>& itemGroups,
    const double colWidths[6],
    double fontSize,
    double rowHeight,
    double leftMargin,
    double headerHeight,
    const std::string headers[6])
{
    if (itemGroups.empty()) return;

    auto* ctx = doc.GetContext();
    double categoryStartY = doc.GetY();
    double x = leftMargin;
    double itemX = x + colWidths[0];

    for (const auto& itemGroup : itemGroups)
    {
        const auto& transactions = itemGroup.second;

        std::vector<std::string> itemLines = wrapText(itemGroup.first, colWidths[1] - 6, fontSize);

        std::vector<double> transactionHeights;
        for (const auto& transaction : transactions) {
            std::vector<std::string> descLines = wrapText(transaction.Description, colWidths[2] - 6, fontSize);
            size_t lineCount = descLines.size();
            if (lineCount == 0) lineCount = 1;
            transactionHeights.push_back(rowHeight * lineCount);
        }

        // 항목 텍스트 최소 높이 확보 (첫 번째 거래에 추가)
        double itemTextHeight = itemLines.size() * rowHeight;
        double totalTxHeight = 0;
        for (auto h : transactionHeights) totalTxHeight += h;
        if (totalTxHeight < itemTextHeight && !transactionHeights.empty())
            transactionHeights[0] += itemTextHeight - totalTxHeight;

        // 항목 셀 세그먼트 추적
        double itemSegmentStartY = doc.GetY();
        bool itemTextDrawn = false;

        // 나머지 열 - 거래 단위로 페이지 분기 처리
        double contentStartX = itemX + colWidths[1];
        double currentY = doc.GetY();

        auto finishSegments = [&]() {
            // 항목 병합 셀 세그먼트 완성
            double segHeight = itemSegmentStartY - currentY;
            if (segHeight > 0) {
                ctx->q(); ctx->w(0.5); ctx->k(0, 0, 0, 1);
                ctx->re(itemX, currentY, colWidths[1], segHeight);
                ctx->S(); ctx->Q();

                // 항목 텍스트는 첫 번째 세그먼트에만 출력
                if (!itemTextDrawn) {
                    double textY = itemSegmentStartY - 2 - fontSize;
                    for (const auto& line : itemLines) {
                        ctx->BT(); ctx->k(0, 0, 0, 1);
                        ctx->Tf(doc.GetFontRegular(), fontSize);
                        ctx->Tm(1, 0, 0, 1, itemX + 3, textY);
                        ctx->Tj(line); ctx->ET();
                        textY -= fontSize + 3;
                    }
                    itemTextDrawn = true;
                }
            }

            // 카테고리 병합 셀 세그먼트 완성
            double categoryHeight = categoryStartY - currentY;
            if (categoryHeight > 0) {
                ctx->q(); ctx->w(0.5); ctx->k(0, 0, 0, 1);
                ctx->re(x, currentY, colWidths[0], categoryHeight);
                ctx->S(); ctx->Q();
                ctx->BT(); ctx->k(0, 0, 0, 1);
                ctx->Tf(doc.GetFontRegular(), fontSize);
                double textY = currentY + categoryHeight / 2 - fontSize / 2 + 2;
                ctx->Tm(1, 0, 0, 1, x + 3, textY);
                ctx->Tj(category); ctx->ET();
            }
        };

        for (size_t i = 0; i < transactions.size(); i++)
        {
            double txHeight = transactionHeights[i];

            // 거래 단위 페이지 분기 체크
            if (currentY - txHeight < 30)
            {
                finishSegments();

                doc.NewPage();
                ctx = doc.GetContext();
                { FMLPDFRow r(headerHeight, true); for (int j = 0; j < 6; j++) r.AddCell(headers[j], colWidths[j], j == 3 ? E_MLPDFAlign::Right : E_MLPDFAlign::Left); doc.DrawRow(r, leftMargin, 9, true); }
                doc.SetY(doc.GetY() + headerHeight); // 헤더 하단(= 콘텐츠 시작점)으로 복귀

                categoryStartY    = doc.GetY();
                itemSegmentStartY = doc.GetY();
                currentY          = doc.GetY();
            }

            const auto& transaction = transactions[i];
            double rowY = currentY - txHeight;

            // 내용 셀
            ctx->q();
            ctx->w(0.5);
            ctx->k(0, 0, 0, 1);
            ctx->re(contentStartX, rowY, colWidths[2], txHeight);
            ctx->S();
            ctx->Q();

            std::vector<std::string> descLines = wrapText(transaction.Description, colWidths[2] - 6, fontSize);
            double totalDescVisualHeight = fontSize * descLines.size() + 3 * (descLines.size() - 1);
            double descTopPadding = (txHeight - totalDescVisualHeight) / 2;
            if (descTopPadding < 2) descTopPadding = 2;
            double textY = rowY + txHeight - descTopPadding - fontSize;
            for (const auto& line : descLines) {
                ctx->BT();
                ctx->k(0, 0, 0, 1);
                ctx->Tf(doc.GetFontRegular(), fontSize);
                ctx->Tm(1, 0, 0, 1, contentStartX + 3, textY);
                ctx->Tj(line);
                ctx->ET();
                textY -= fontSize + 3;
            }

            // 금액 셀 (오른쪽 정렬, 환율 정보 포함)
            double amountX = contentStartX + colWidths[2];
            ctx->q();
            ctx->w(0.5);
            ctx->k(0, 0, 0, 1);
            ctx->re(amountX, rowY, colWidths[3], txHeight);
            ctx->S();
            ctx->Q();

            std::string amountStr = formatAmountHelper(transaction.Amount) + "원";
            if (transaction.UseExchangeRate && transaction.Currency != "KRW") {
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(1);
                oss << " (" << transaction.Currency << " "
                    << formatAmountHelper(static_cast<int64_t>(transaction.OriginalAmount))
                    << " @ " << transaction.ExchangeRate << ")";
                amountStr += oss.str();
            }

            double estimatedWidth = amountStr.length() * fontSize * 0.465;
            ctx->BT();
            ctx->k(0, 0, 0, 1);
            ctx->Tf(doc.GetFontRegular(), fontSize);
            ctx->Tm(1, 0, 0, 1, amountX + colWidths[3] - estimatedWidth - 6, rowY + (txHeight - fontSize) / 2 + 2);
            ctx->Tj(amountStr);
            ctx->ET();

            // 날짜 셀
            double dateX = amountX + colWidths[3];
            ctx->q();
            ctx->w(0.5);
            ctx->k(0, 0, 0, 1);
            ctx->re(dateX, rowY, colWidths[4], txHeight);
            ctx->S();
            ctx->Q();

            std::string dateOnly = transaction.DateTime.substr(0, 10);
            ctx->BT();
            ctx->k(0, 0, 0, 1);
            ctx->Tf(doc.GetFontRegular(), fontSize);
            ctx->Tm(1, 0, 0, 1, dateX + 3, rowY + (txHeight - fontSize) / 2 + 2);
            ctx->Tj(dateOnly);
            ctx->ET();

            // 영수증번호 셀
            double receiptX = dateX + colWidths[4];
            ctx->q();
            ctx->w(0.5);
            ctx->k(0, 0, 0, 1);
            ctx->re(receiptX, rowY, colWidths[5], txHeight);
            ctx->S();
            ctx->Q();

            ctx->BT();
            ctx->k(0, 0, 0, 1);
            ctx->Tf(doc.GetFontRegular(), fontSize);
            ctx->Tm(1, 0, 0, 1, receiptX + 3, rowY + (txHeight - fontSize) / 2 + 2);
            ctx->Tj(transaction.ReceiptNumber);
            ctx->ET();

            currentY = rowY;
        }

        doc.SetY(currentY);

        // 항목 그룹 마지막 세그먼트 완성 (항목 셀만, 카테고리는 전체 반복 후 한 번)
        double segHeight = itemSegmentStartY - currentY;
        if (segHeight > 0) {
            ctx->q(); ctx->w(0.5); ctx->k(0, 0, 0, 1);
            ctx->re(itemX, currentY, colWidths[1], segHeight);
            ctx->S(); ctx->Q();

            if (!itemTextDrawn) {
                double textY = itemSegmentStartY - 2 - fontSize;
                for (const auto& line : itemLines) {
                    ctx->BT(); ctx->k(0, 0, 0, 1);
                    ctx->Tf(doc.GetFontRegular(), fontSize);
                    ctx->Tm(1, 0, 0, 1, itemX + 3, textY);
                    ctx->Tj(line); ctx->ET();
                    textY -= fontSize + 3;
                }
            }
        }
    }

    // 마지막 페이지의 카테고리 셀 완성
    double categoryHeight = categoryStartY - doc.GetY();
    if (categoryHeight > 0) {
        ctx->q(); ctx->w(0.5); ctx->k(0, 0, 0, 1);
        ctx->re(x, doc.GetY(), colWidths[0], categoryHeight);
        ctx->S(); ctx->Q();
        ctx->BT(); ctx->k(0, 0, 0, 1);
        ctx->Tf(doc.GetFontRegular(), fontSize);
        double textY = doc.GetY() + categoryHeight / 2 - fontSize / 2 + 2;
        ctx->Tm(1, 0, 0, 1, x + 3, textY);
        ctx->Tj(category); ctx->ET();
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

        auto* ctx       = doc.GetContext();
        auto* fontBold  = doc.GetFontBold();

        const double leftMargin     = 50.0;
        const double tableWidth     = 495.0;
        const double rowHeight      = 20.0;
        const double headerHeight   = 25.0;
        const double sectionSpacing = 15.0;

        // === 제목 (중앙 정렬 + 자동 크기 조절) ===
        double titleFontSize = 20.0;

        size_t charCount = 0;
        for (size_t i = 0; i < data.Title.length(); ) {
            unsigned char c = static_cast<unsigned char>(data.Title[i]);
            if      (c < 0x80) { i += 1; charCount++; }
            else if (c < 0xE0) { i += 2; charCount++; }
            else if (c < 0xF0) { i += 3; charCount++; }
            else               { i += 4; charCount++; }
        }

        double titleWidth = charCount * titleFontSize * 0.7;
        const double maxTitleWidth = doc.GetPageWidth() - 100.0;
        if (titleWidth > maxTitleWidth) {
            titleFontSize = titleFontSize * maxTitleWidth / titleWidth;
            titleWidth = charCount * titleFontSize * 0.7;
        }

        const double titleX = (doc.GetPageWidth() - titleWidth) / 2.0;
        ctx->BT();
        ctx->k(0, 0, 0, 1);
        ctx->Tf(fontBold, titleFontSize);
        ctx->Tm(1, 0, 0, 1, titleX, doc.GetY());
        ctx->Tj(data.Title);
        ctx->ET();
        doc.MoveDown(40.0);

        // === 1. 요약 - 총 수입 / 총 지출 (예산) ===
        doc.CheckPageBreak();
        doc.DrawRow(FMLPDFRow(rowHeight).AddCell("총 수입 (예산)", tableWidth * 0.6).AddCell(formatAmount(data.TotalIncome) + "원", tableWidth * 0.4, E_MLPDFAlign::Right), leftMargin, 10, true);

        doc.CheckPageBreak();
        doc.DrawRow(FMLPDFRow(rowHeight).AddCell("총 지출 (예산)", tableWidth * 0.6).AddCell(formatAmount(data.TotalExpense) + "원", tableWidth * 0.4, E_MLPDFAlign::Right), leftMargin, 10, true);
        doc.MoveDown(sectionSpacing);

        // === 1. 요약 - 총 실수입 / 총 실지출 / 잔액 (실적) ===
        doc.CheckPageBreak();
        doc.DrawRow(FMLPDFRow(rowHeight).AddCell("총 실수입", tableWidth * 0.6).AddCell(formatAmount(data.TotalActualIncome) + "원", tableWidth * 0.4, E_MLPDFAlign::Right), leftMargin, 10, true);

        doc.CheckPageBreak();
        doc.DrawRow(FMLPDFRow(rowHeight).AddCell("총 실지출", tableWidth * 0.6).AddCell(formatAmount(data.TotalActualExpense) + "원", tableWidth * 0.4, E_MLPDFAlign::Right), leftMargin, 10, true);

        doc.CheckPageBreak();
        doc.DrawRow(FMLPDFRow(rowHeight).AddCell("잔액", tableWidth * 0.6).AddCell(formatAmount(data.TotalBalance) + "원", tableWidth * 0.4, E_MLPDFAlign::Right), leftMargin, 10, true);
        doc.MoveDown(sectionSpacing);

        // === 2. 환율 정보 ===
        if (!data.ExchangeRates.empty()) {
            doc.CheckPageBreak();
            ctx->BT();
            ctx->Tf(fontBold, 14);
            ctx->Tm(1, 0, 0, 1, leftMargin, doc.GetY());
            ctx->Tj("환율 정보");
            ctx->ET();
            doc.MoveDown(30.0);

            doc.CheckPageBreak();
            doc.DrawRow(FMLPDFRow(headerHeight, true).AddCell("통화", tableWidth * 0.6).AddCell("평균 환율", tableWidth * 0.4, E_MLPDFAlign::Right), leftMargin, 11, true);

            for (const auto& rate : data.ExchangeRates) {
                doc.CheckPageBreak();
                std::stringstream ss;
                ss << std::fixed << std::setprecision(2) << rate.second << "원";
                doc.DrawRow(FMLPDFRow(rowHeight).AddCell(rate.first, tableWidth * 0.6).AddCell(ss.str(), tableWidth * 0.4, E_MLPDFAlign::Right), leftMargin, 10);
            }
            doc.MoveDown(sectionSpacing);
        }

        // === 3. 수입 비교 (예산 vs 실적) ===
        doc.CheckPageBreak();
        ctx->BT();
        ctx->Tf(fontBold, 14);
        ctx->Tm(1, 0, 0, 1, leftMargin, doc.GetY());
        ctx->Tj("수입 비교");
        ctx->ET();
        doc.MoveDown(30.0);

        doc.CheckPageBreak();
        doc.DrawRow(FMLPDFRow(headerHeight, true).AddCell("카테고리", tableWidth * 0.5).AddCell("예산", tableWidth * 0.25, E_MLPDFAlign::Right).AddCell("실적", tableWidth * 0.25, E_MLPDFAlign::Right), leftMargin, 11, true);

        std::set<std::string> incomeCategories;
        for (const auto& cat : data.BudgetIncomeCategories) incomeCategories.insert(cat.first);
        for (const auto& cat : data.ActualIncomeCategories)  incomeCategories.insert(cat.first);

        for (const auto& category : incomeCategories) {
            doc.CheckPageBreak();
            int64_t budgetAmount = 0, actualAmount = 0;
            auto budgetIt = data.BudgetIncomeCategories.find(category);
            if (budgetIt != data.BudgetIncomeCategories.end()) budgetAmount = budgetIt->second;
            auto actualIt = data.ActualIncomeCategories.find(category);
            if (actualIt != data.ActualIncomeCategories.end()) actualAmount = actualIt->second;
            doc.DrawRow(FMLPDFRow(rowHeight).AddCell(category, tableWidth * 0.5).AddCell(formatAmount(budgetAmount) + "원", tableWidth * 0.25, E_MLPDFAlign::Right).AddCell(formatAmount(actualAmount) + "원", tableWidth * 0.25, E_MLPDFAlign::Right), leftMargin, 10);
        }

        doc.CheckPageBreak();
        doc.DrawRow(FMLPDFRow(headerHeight, true).AddCell("합계", tableWidth * 0.5).AddCell(formatAmount(data.TotalIncome) + "원", tableWidth * 0.25, E_MLPDFAlign::Right).AddCell(formatAmount(data.TotalActualIncome) + "원", tableWidth * 0.25, E_MLPDFAlign::Right), leftMargin, 9, true);
        doc.MoveDown(sectionSpacing);

        // === 4. 지출 비교 (예산 vs 실적) ===
        doc.CheckPageBreak();
        ctx->BT();
        ctx->Tf(fontBold, 14);
        ctx->Tm(1, 0, 0, 1, leftMargin, doc.GetY());
        ctx->Tj("지출 비교");
        ctx->ET();
        doc.MoveDown(30.0);

        doc.CheckPageBreak();
        doc.DrawRow(FMLPDFRow(headerHeight, true).AddCell("카테고리", tableWidth * 0.5).AddCell("예산", tableWidth * 0.25, E_MLPDFAlign::Right).AddCell("실적", tableWidth * 0.25, E_MLPDFAlign::Right), leftMargin, 11, true);

        std::map<std::string, int64_t> budgetExpenseCategories;
        std::map<std::string, int64_t> actualExpenseCategories;
        for (const auto& category : data.BudgetExpenseItems) {
            int64_t total = 0;
            for (const auto& item : category.second) total += item.second;
            budgetExpenseCategories[category.first] = total;
        }
        for (const auto& category : data.ActualExpenseItems) {
            int64_t total = 0;
            for (const auto& item : category.second) total += item.second;
            actualExpenseCategories[category.first] = total;
        }

        std::set<std::string> allExpenseCategories;
        for (const auto& cat : budgetExpenseCategories) allExpenseCategories.insert(cat.first);
        for (const auto& cat : actualExpenseCategories)  allExpenseCategories.insert(cat.first);

        for (const auto& category : allExpenseCategories) {
            doc.CheckPageBreak();
            int64_t budgetAmount = 0, actualAmount = 0;
            auto budgetIt = budgetExpenseCategories.find(category);
            if (budgetIt != budgetExpenseCategories.end()) budgetAmount = budgetIt->second;
            auto actualIt = actualExpenseCategories.find(category);
            if (actualIt != actualExpenseCategories.end()) actualAmount = actualIt->second;
            doc.DrawRow(FMLPDFRow(rowHeight).AddCell(category, tableWidth * 0.5).AddCell(formatAmount(budgetAmount) + "원", tableWidth * 0.25, E_MLPDFAlign::Right).AddCell(formatAmount(actualAmount) + "원", tableWidth * 0.25, E_MLPDFAlign::Right), leftMargin, 10);
        }

        doc.CheckPageBreak();
        doc.DrawRow(FMLPDFRow(headerHeight, true).AddCell("합계", tableWidth * 0.5).AddCell(formatAmount(data.TotalExpense) + "원", tableWidth * 0.25, E_MLPDFAlign::Right).AddCell(formatAmount(data.TotalActualExpense) + "원", tableWidth * 0.25, E_MLPDFAlign::Right), leftMargin, 9, true);

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
    const std::string& filePath)
{
    try {
        FMLPDFDocument doc;
        if (!doc.Open(filePath)) return false;

        auto* ctx      = doc.GetContext();
        auto* fontBold = doc.GetFontBold();

        std::string filename = extractFilename(filePath);

        const double leftMargin     = 30.0;
        const double rowHeight      = 18.0;
        const double headerHeight   = 22.0;
        const double sectionSpacing = 20.0;

        const double colWidths[6] = {50, 60, 170, 135, 52, 58};
        double tableWidth = 0;
        for (int i = 0; i < 6; i++) tableWidth += colWidths[i];

        // === 제목 (파일명) ===
        ctx->BT();
        ctx->k(0, 0, 0, 1);
        ctx->Tf(fontBold, 18);
        ctx->Tm(1, 0, 0, 1, leftMargin, doc.GetY());
        ctx->Tj(filename);
        ctx->ET();
        doc.MoveDown(25.0);

        // === 합계 요약 섹션 ===
        int64_t totalIncome = 0;
        int64_t totalExpense = 0;

        std::vector<FMLTransactionData> incomeList;
        std::vector<FMLTransactionData> expenseList;

        for (const auto& transaction : transactions) {
            if (transaction.Type == E_MLTransactionType::Income) {
                incomeList.push_back(transaction);
                totalIncome += transaction.Amount;
            } else if (transaction.Type == E_MLTransactionType::Expense) {
                expenseList.push_back(transaction);
                totalExpense += transaction.Amount;
            }
        }

        int64_t balance = totalIncome - totalExpense;

        ctx->BT();
        ctx->Tf(fontBold, 14);
        ctx->Tm(1, 0, 0, 1, leftMargin, doc.GetY());
        ctx->Tj("합계");
        ctx->ET();
        doc.MoveDown(30.0);

        const double summaryWidth = 300.0;
        doc.DrawRow(FMLPDFRow(headerHeight, true).AddCell("총 수입", summaryWidth * 0.6).AddCell(formatAmount(totalIncome) + "원", summaryWidth * 0.4, E_MLPDFAlign::Right), leftMargin, 10, true);
        doc.DrawRow(FMLPDFRow(headerHeight, true).AddCell("총 지출", summaryWidth * 0.6).AddCell(formatAmount(totalExpense) + "원", summaryWidth * 0.4, E_MLPDFAlign::Right), leftMargin, 10, true);
        doc.DrawRow(FMLPDFRow(headerHeight, true).AddCell("남은 금액", summaryWidth * 0.6).AddCell(formatAmount(balance) + "원", summaryWidth * 0.4, E_MLPDFAlign::Right), leftMargin, 10, true);
        doc.MoveDown(sectionSpacing);

        // === 카테고리별 합계 ===
        std::map<std::string, int64_t> incomeCategoryTotals;
        std::map<std::string, int64_t> expenseCategoryTotals;
        for (const auto& t : incomeList)  incomeCategoryTotals[t.Category]  += t.Amount;
        for (const auto& t : expenseList) expenseCategoryTotals[t.Category] += t.Amount;

        // 수입 카테고리 합계 표
        if (!incomeCategoryTotals.empty()) {
            ctx->BT();
            ctx->Tf(fontBold, 11);
            ctx->Tm(1, 0, 0, 1, leftMargin, doc.GetY());
            ctx->Tj("수입 카테고리별");
            ctx->ET();
            doc.MoveDown(18.0);

            const double catAvailWidth = 595.0 - leftMargin - 30.0;
            const double minCatColWidth = 60.0;
            const double categoryRowHeight = 16.0;

            std::vector<std::pair<std::string, int64_t>> incomeCatList(incomeCategoryTotals.begin(), incomeCategoryTotals.end());
            const size_t incomeTotalCats = incomeCatList.size();
            const int incomeMaxTotalCols = std::max(2, (int)(catAvailWidth / minCatColWidth));
            const int incomeMaxCatsPerRow = incomeMaxTotalCols - 1;
            const double incomeColWidth = ((int)incomeTotalCats < incomeMaxCatsPerRow)
                ? catAvailWidth / (incomeTotalCats + 1)
                : catAvailWidth / incomeMaxTotalCols;

            for (size_t rowStart = 0; rowStart < incomeTotalCats; ) {
                const size_t rowEnd = std::min(rowStart + (size_t)incomeMaxCatsPerRow, incomeTotalCats);
                const bool isLastRow = (rowEnd == incomeTotalCats);

                double currentX = leftMargin;
                for (size_t i = rowStart; i < rowEnd; i++) {
                    ctx->q(); ctx->w(0.5); ctx->k(0, 0, 0, 1); ctx->k(0, 0, 0, 0.1);
                    ctx->re(currentX, doc.GetY(), incomeColWidth, categoryRowHeight);
                    ctx->B(); ctx->Q();
                    ctx->BT(); ctx->k(0, 0, 0, 1); ctx->Tf(doc.GetFontRegular(), 8);
                    ctx->Tm(1, 0, 0, 1, currentX + 3, doc.GetY() + 4);
                    ctx->Tj(incomeCatList[i].first); ctx->ET();
                    currentX += incomeColWidth;
                }
                ctx->q(); ctx->w(0.5); ctx->k(0, 0, 0, 1); ctx->k(0, 0, 0, 0.1);
                ctx->re(currentX, doc.GetY(), incomeColWidth, categoryRowHeight);
                ctx->B(); ctx->Q();
                ctx->BT(); ctx->k(0, 0, 0, 1); ctx->Tf(fontBold, 8);
                ctx->Tm(1, 0, 0, 1, currentX + 3, doc.GetY() + 4);
                ctx->Tj(isLastRow ? "총 수입" : "소계"); ctx->ET();
                doc.MoveDown(categoryRowHeight);

                currentX = leftMargin;
                int64_t incomeRowTotal = 0;
                for (size_t i = rowStart; i < rowEnd; i++) {
                    incomeRowTotal += incomeCatList[i].second;
                    ctx->q(); ctx->w(0.5); ctx->k(0, 0, 0, 1);
                    ctx->re(currentX, doc.GetY(), incomeColWidth, categoryRowHeight);
                    ctx->S(); ctx->Q();
                    std::string amountStr = formatAmount(incomeCatList[i].second);
                    double estimatedWidth = amountStr.length() * 8 * 0.45;
                    ctx->BT(); ctx->k(0, 0, 0, 1); ctx->Tf(doc.GetFontRegular(), 8);
                    ctx->Tm(1, 0, 0, 1, currentX + incomeColWidth - estimatedWidth - 8, doc.GetY() + 4);
                    ctx->Tj(amountStr); ctx->ET();
                    currentX += incomeColWidth;
                }
                int64_t incomeDisplayTotal = isLastRow ? totalIncome : incomeRowTotal;
                ctx->q(); ctx->w(0.5); ctx->k(0, 0, 0, 1);
                ctx->re(currentX, doc.GetY(), incomeColWidth, categoryRowHeight);
                ctx->S(); ctx->Q();
                std::string totalStr = formatAmount(incomeDisplayTotal);
                double estimatedWidth = totalStr.length() * 8 * 0.45;
                ctx->BT(); ctx->k(0, 0, 0, 1); ctx->Tf(fontBold, 8);
                ctx->Tm(1, 0, 0, 1, currentX + incomeColWidth - estimatedWidth - 8, doc.GetY() + 4);
                ctx->Tj(totalStr); ctx->ET();
                doc.MoveDown(categoryRowHeight + (isLastRow ? 10 : 3));

                rowStart = rowEnd;
            }
        }

        // 지출 카테고리 합계 표
        if (!expenseCategoryTotals.empty()) {
            ctx->BT();
            ctx->Tf(fontBold, 11);
            ctx->Tm(1, 0, 0, 1, leftMargin, doc.GetY());
            ctx->Tj("지출 카테고리별");
            ctx->ET();
            doc.MoveDown(18.0);

            const double catAvailWidth = 595.0 - leftMargin - 30.0;
            const double minCatColWidth = 60.0;
            const double categoryRowHeight = 16.0;

            std::vector<std::pair<std::string, int64_t>> expenseCatList(expenseCategoryTotals.begin(), expenseCategoryTotals.end());
            const size_t expenseTotalCats = expenseCatList.size();
            const int expenseMaxTotalCols = std::max(2, (int)(catAvailWidth / minCatColWidth));
            const int expenseMaxCatsPerRow = expenseMaxTotalCols - 1;
            const double expenseColWidth = ((int)expenseTotalCats < expenseMaxCatsPerRow)
                ? catAvailWidth / (expenseTotalCats + 1)
                : catAvailWidth / expenseMaxTotalCols;

            for (size_t rowStart = 0; rowStart < expenseTotalCats; ) {
                const size_t rowEnd = std::min(rowStart + (size_t)expenseMaxCatsPerRow, expenseTotalCats);
                const bool isLastRow = (rowEnd == expenseTotalCats);

                double currentX = leftMargin;
                for (size_t i = rowStart; i < rowEnd; i++) {
                    ctx->q(); ctx->w(0.5); ctx->k(0, 0, 0, 1); ctx->k(0, 0, 0, 0.1);
                    ctx->re(currentX, doc.GetY(), expenseColWidth, categoryRowHeight);
                    ctx->B(); ctx->Q();
                    ctx->BT(); ctx->k(0, 0, 0, 1); ctx->Tf(doc.GetFontRegular(), 8);
                    ctx->Tm(1, 0, 0, 1, currentX + 3, doc.GetY() + 4);
                    ctx->Tj(expenseCatList[i].first); ctx->ET();
                    currentX += expenseColWidth;
                }
                ctx->q(); ctx->w(0.5); ctx->k(0, 0, 0, 1); ctx->k(0, 0, 0, 0.1);
                ctx->re(currentX, doc.GetY(), expenseColWidth, categoryRowHeight);
                ctx->B(); ctx->Q();
                ctx->BT(); ctx->k(0, 0, 0, 1); ctx->Tf(fontBold, 8);
                ctx->Tm(1, 0, 0, 1, currentX + 3, doc.GetY() + 4);
                ctx->Tj(isLastRow ? "총 지출" : "소계"); ctx->ET();
                doc.MoveDown(categoryRowHeight);

                currentX = leftMargin;
                int64_t expenseRowTotal = 0;
                for (size_t i = rowStart; i < rowEnd; i++) {
                    expenseRowTotal += expenseCatList[i].second;
                    ctx->q(); ctx->w(0.5); ctx->k(0, 0, 0, 1);
                    ctx->re(currentX, doc.GetY(), expenseColWidth, categoryRowHeight);
                    ctx->S(); ctx->Q();
                    std::string amountStr = formatAmount(expenseCatList[i].second);
                    double estimatedWidth = amountStr.length() * 8 * 0.45;
                    ctx->BT(); ctx->k(0, 0, 0, 1); ctx->Tf(doc.GetFontRegular(), 8);
                    ctx->Tm(1, 0, 0, 1, currentX + expenseColWidth - estimatedWidth - 8, doc.GetY() + 4);
                    ctx->Tj(amountStr); ctx->ET();
                    currentX += expenseColWidth;
                }
                int64_t expenseDisplayTotal = isLastRow ? totalExpense : expenseRowTotal;
                ctx->q(); ctx->w(0.5); ctx->k(0, 0, 0, 1);
                ctx->re(currentX, doc.GetY(), expenseColWidth, categoryRowHeight);
                ctx->S(); ctx->Q();
                std::string totalStr = formatAmount(expenseDisplayTotal);
                double estimatedWidth = totalStr.length() * 8 * 0.45;
                ctx->BT(); ctx->k(0, 0, 0, 1); ctx->Tf(fontBold, 8);
                ctx->Tm(1, 0, 0, 1, currentX + expenseColWidth - estimatedWidth - 8, doc.GetY() + 4);
                ctx->Tj(totalStr); ctx->ET();
                doc.MoveDown(categoryRowHeight + (isLastRow ? sectionSpacing : 3));

                rowStart = rowEnd;
            }
        }

        // === 수입 섹션 ===
        if (!incomeList.empty()) {
            ctx->BT();
            ctx->Tf(fontBold, 14);
            ctx->Tm(1, 0, 0, 1, leftMargin, doc.GetY());
            ctx->Tj("수입");
            ctx->ET();
            doc.MoveDown(25.0);

            const std::string headers[6] = {"구분", "항목", "내용", "수입금액", "년월일", "영수증번호"};
            { FMLPDFRow r(headerHeight, true); for (int i = 0; i < 6; i++) r.AddCell(headers[i], colWidths[i], i == 3 ? E_MLPDFAlign::Right : E_MLPDFAlign::Left); doc.DrawRow(r, leftMargin, 9, true); }
            doc.SetY(doc.GetY() + headerHeight); // 헤더 하단(= 콘텐츠 시작점)으로 복귀

            std::map<std::string, std::map<std::string, std::vector<FMLTransactionData>>> incomeCategories;
            for (const auto& transaction : incomeList)
                incomeCategories[transaction.Category][transaction.Item].push_back(transaction);

            for (const auto& categoryPair : incomeCategories)
                drawCategoryGroup(doc, categoryPair.first, categoryPair.second, colWidths, 8, rowHeight, leftMargin, headerHeight, headers);

            doc.MoveDown(sectionSpacing);
        }

        // === 지출 섹션 ===
        if (!expenseList.empty()) {
            ctx->BT();
            ctx->Tf(fontBold, 14);
            ctx->Tm(1, 0, 0, 1, leftMargin, doc.GetY());
            ctx->Tj("지출");
            ctx->ET();
            doc.MoveDown(25.0);

            const std::string headers[6] = {"구분", "항목", "내용", "지출금액", "년월일", "영수증번호"};
            { FMLPDFRow r(headerHeight, true); for (int i = 0; i < 6; i++) r.AddCell(headers[i], colWidths[i], i == 3 ? E_MLPDFAlign::Right : E_MLPDFAlign::Left); doc.DrawRow(r, leftMargin, 9, true); }
            doc.SetY(doc.GetY() + headerHeight); // 헤더 하단(= 콘텐츠 시작점)으로 복귀

            std::map<std::string, std::map<std::string, std::vector<FMLTransactionData>>> expenseCategories;
            for (const auto& transaction : expenseList)
                expenseCategories[transaction.Category][transaction.Item].push_back(transaction);

            for (const auto& categoryPair : expenseCategories)
                drawCategoryGroup(doc, categoryPair.first, categoryPair.second, colWidths, 8, rowHeight, leftMargin, headerHeight, headers);
        }

        doc.Close();
        return true;
    }
    catch (...) {
        return false;
    }
}

// 공통 헬퍼: 금액 포맷 (천 단위 콤마)
std::string FMLPDFExporter::formatAmount(int64_t amount)
{
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << amount;
    return ss.str();
}

void FMLPDFExporter::drawHeader(void* pdfWriter, const std::string& title)
{
    // TODO: 헤더 그리기 구현
}

void FMLPDFExporter::drawFooter(void* pdfWriter)
{
    // TODO: 푸터 그리기 구현
}
