#include "MLPDFExporter.h"
#include "PDFWriter/PDFWriter.h"
#include "PDFWriter/PDFPage.h"
#include "PDFWriter/PageContentContext.h"
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>

// 표 그리기 헬퍼 함수
static void drawTableRow(
    PageContentContext* ctx,
    double x, double y,
    double width, double height,
    const std::string& col1, const std::string& col2,
    PDFUsedFont* font, double fontSize,
    bool isHeader)
{
    const double col1Width = width * 0.6;
    const double col2Width = width * 0.4;
    const double padding = 5.0;

    // 헤더 배경 (회색)
    if (isHeader)
    {
        ctx->q();
        ctx->k(0, 0, 0, 0.1); // 10% black (light gray)
        ctx->re(x, y, width, height);
        ctx->f();
        ctx->Q();
    }

    // 테두리
    ctx->q();
    ctx->w(0.5);
    ctx->k(0, 0, 0, 1);
    ctx->re(x, y, width, height);
    ctx->S();

    // 세로 구분선
    ctx->m(x + col1Width, y);
    ctx->l(x + col1Width, y + height);
    ctx->S();
    ctx->Q();

    // 텍스트 출력
    ctx->BT();
    ctx->k(0, 0, 0, 1);
    ctx->Tf(font, fontSize);

    // 첫 번째 열 (왼쪽 정렬)
    ctx->Tm(1, 0, 0, 1, x + padding, y + (height - fontSize) / 2 + 2);
    ctx->Tj(col1);

    ctx->ET();

    // 두 번째 열 (오른쪽 정렬) - 대략적인 위치 계산
    ctx->BT();
    ctx->k(0, 0, 0, 1);
    ctx->Tf(font, fontSize);
    // 문자열 길이 대략 추정 (한글: fontSize * 0.7, 영문/숫자: fontSize * 0.5)
    double estimatedWidth = col2.length() * fontSize * 0.4;
    ctx->Tm(1, 0, 0, 1, x + width - estimatedWidth - padding - 5, y + (height - fontSize) / 2 + 2);
    ctx->Tj(col2);

    ctx->ET();
}

// 결산 보고서 PDF 생성
bool FMLPDFExporter::ExportSettlement(
    const FMLSettlmentData& data,
    const std::string& filePath)
{
    try {
        PDFWriter pdfWriter;
        pdfWriter.StartPDF(filePath, ePDFVersion13);

        // 페이지 생성 (A4 size: 595 x 842 points)
        PDFPage* page = new PDFPage();
        page->SetMediaBox(PDFRectangle(0, 0, 595, 842));

        PageContentContext* contentContext = pdfWriter.StartPageContentContext(page);

        // 한글 폰트 로드
        PDFUsedFont* fontBold = pdfWriter.GetFontForFile("C:\\Windows\\Fonts\\malgunbd.ttf");
        PDFUsedFont* fontRegular = pdfWriter.GetFontForFile("C:\\Windows\\Fonts\\malgun.ttf");

        double yPos = 800.0; // 시작 Y 위치
        const double leftMargin = 50.0;
        const double tableWidth = 495.0;
        const double rowHeight = 20.0;
        const double headerHeight = 25.0;
        const double sectionSpacing = 15.0;

        // === 제목 ===
        contentContext->BT();
        contentContext->k(0, 0, 0, 1); // Black
        contentContext->Tf(fontBold, 24);
        contentContext->Tm(1, 0, 0, 1, 220, yPos);
        contentContext->Tj("결산 보고서");
        contentContext->ET();

        yPos -= 40.0;

        // === 예산 섹션 ===
        contentContext->BT();
        contentContext->Tf(fontBold, 16);
        contentContext->Tm(1, 0, 0, 1, leftMargin, yPos);
        contentContext->Tj("예산");
        contentContext->ET();
        yPos -= 25.0;

        // 헤더 행
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                     "카테고리", "금액", fontBold, 11, true);
        yPos -= headerHeight;

        // 수입 소제목
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                     "[수입]", "", fontBold, 10, false);
        yPos -= rowHeight;

        // 예산 수입 데이터
        for (const auto& category : data.BudgetIncomeCategories)
        {
            std::string amountStr = formatAmount(category.second) + "원";
            drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                         "  " + category.first, amountStr, fontRegular, 10, false);
            yPos -= rowHeight;
        }

        // 지출 소제목
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                     "[지출]", "", fontBold, 10, false);
        yPos -= rowHeight;

        // 예산 지출 데이터
        for (const auto& category : data.BudgetExpenseCategories)
        {
            std::string amountStr = formatAmount(category.second) + "원";
            drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                         "  " + category.first, amountStr, fontRegular, 10, false);
            yPos -= rowHeight;
        }

        // 예산 합계 행
        std::string budgetTotalIncome = formatAmount(data.TotalIncome) + "원";
        std::string budgetTotalExpense = formatAmount(data.TotalExpense) + "원";
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                     "합계 (수입 / 지출)", budgetTotalIncome + " / " + budgetTotalExpense,
                     fontBold, 11, true);
        yPos -= headerHeight + sectionSpacing;

        // === 실적 섹션 ===
        contentContext->BT();
        contentContext->Tf(fontBold, 16);
        contentContext->Tm(1, 0, 0, 1, leftMargin, yPos);
        contentContext->Tj("실적");
        contentContext->ET();
        yPos -= 25.0;

        // 헤더 행
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                     "카테고리", "금액", fontBold, 11, true);
        yPos -= headerHeight;

        // 수입 소제목
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                     "[수입]", "", fontBold, 10, false);
        yPos -= rowHeight;

        // 실적 수입 데이터
        for (const auto& category : data.ActualIncomeCategories)
        {
            std::string amountStr = formatAmount(category.second) + "원";
            drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                         "  " + category.first, amountStr, fontRegular, 10, false);
            yPos -= rowHeight;
        }

        // 지출 소제목
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                     "[지출]", "", fontBold, 10, false);
        yPos -= rowHeight;

        // 실적 지출 데이터
        for (const auto& category : data.ActualExpenseCategories)
        {
            std::string amountStr = formatAmount(category.second) + "원";
            drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                         "  " + category.first, amountStr, fontRegular, 10, false);
            yPos -= rowHeight;
        }

        // 실적 합계 행
        std::string actualTotalIncome = formatAmount(data.TotalActualIncome) + "원";
        std::string actualTotalExpense = formatAmount(data.TotalActualExpense) + "원";
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                     "합계 (수입 / 지출)", actualTotalIncome + " / " + actualTotalExpense,
                     fontBold, 11, true);
        yPos -= headerHeight + sectionSpacing;

        // === 비교 섹션 ===
        contentContext->BT();
        contentContext->Tf(fontBold, 16);
        contentContext->Tm(1, 0, 0, 1, leftMargin, yPos);
        contentContext->Tj("예산 대비 실적");
        contentContext->ET();
        yPos -= 25.0;

        // 헤더 행
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                     "항목", "차이", fontBold, 11, true);
        yPos -= headerHeight;

        int64_t incomeDiff = data.TotalActualIncome - data.TotalIncome;
        int64_t expenseDiff = data.TotalActualExpense - data.TotalExpense;

        // 수입 차이
        std::string incomeDiffStr = formatAmount(incomeDiff) + "원";
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                     "수입 차이", incomeDiffStr, fontRegular, 10, false);
        yPos -= rowHeight;

        // 지출 차이
        std::string expenseDiffStr = formatAmount(expenseDiff) + "원";
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                     "지출 차이", expenseDiffStr, fontRegular, 10, false);
        yPos -= rowHeight;

        // 잔액
        std::string balanceStr = formatAmount(data.TotalBalance) + "원";
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                     "잔액", balanceStr, fontBold, 11, true);
        yPos -= headerHeight + sectionSpacing;

        // === 환율 정보 ===
        if (!data.ExchangeRates.empty())
        {
            contentContext->BT();
            contentContext->Tf(fontBold, 16);
            contentContext->Tm(1, 0, 0, 1, leftMargin, yPos);
            contentContext->Tj("환율 정보");
            contentContext->ET();
            yPos -= 25.0;

            // 헤더 행
            drawTableRow(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                         "통화", "평균 환율", fontBold, 11, true);
            yPos -= headerHeight;

            for (const auto& rate : data.ExchangeRates)
            {
                std::stringstream ss;
                ss << std::fixed << std::setprecision(2) << rate.second << "원";
                drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                             rate.first, ss.str(), fontRegular, 10, false);
                yPos -= rowHeight;
            }
        }

        pdfWriter.EndPageContentContext(contentContext);
        pdfWriter.WritePageAndRelease(page);
        pdfWriter.EndPDF();

        return true;
    }
    catch (...) {
        return false;
    }
}

// 거래 내역 표 행 그리기 (7열)
static void drawTransactionRow(
    PageContentContext* ctx,
    double x, double y, double height,
    const std::string cols[7],
    const double colWidths[7],
    PDFUsedFont* font, double fontSize,
    bool isHeader)
{
    double totalWidth = 0;
    for (int i = 0; i < 7; i++) {
        totalWidth += colWidths[i];
    }

    // 헤더 배경
    if (isHeader)
    {
        ctx->q();
        ctx->k(0, 0, 0, 0.1);
        ctx->re(x, y, totalWidth, height);
        ctx->f();
        ctx->Q();
    }

    // 전체 테두리
    ctx->q();
    ctx->w(0.5);
    ctx->k(0, 0, 0, 1);
    ctx->re(x, y, totalWidth, height);
    ctx->S();

    // 세로 구분선
    double currentX = x;
    for (int i = 0; i < 6; i++) {
        currentX += colWidths[i];
        ctx->m(currentX, y);
        ctx->l(currentX, y + height);
        ctx->S();
    }
    ctx->Q();

    // 텍스트 출력
    currentX = x;
    for (int i = 0; i < 7; i++)
    {
        ctx->BT();
        ctx->k(0, 0, 0, 1);
        ctx->Tf(font, fontSize);

        double textX;
        if (i == 5) { // 금액 열 (오른쪽 정렬)
            double estimatedWidth = cols[i].length() * fontSize * 0.4;
            textX = currentX + colWidths[i] - estimatedWidth - 3;
        } else { // 나머지 열 (왼쪽 정렬)
            textX = currentX + 3;
        }

        ctx->Tm(1, 0, 0, 1, textX, y + (height - fontSize) / 2 + 2);
        ctx->Tj(cols[i]);
        ctx->ET();

        currentX += colWidths[i];
    }
}

// 거래 내역서 PDF 생성 (카테고리/항목별 그룹화)
bool FMLPDFExporter::ExportTransactionList(
    const std::vector<FMLTransactionData>& transactions,
    const std::string& filePath)
{
    try {
        PDFWriter pdfWriter;
        pdfWriter.StartPDF(filePath, ePDFVersion13);

        // 페이지 생성 (A4 size: 595 x 842 points)
        PDFPage* page = new PDFPage();
        page->SetMediaBox(PDFRectangle(0, 0, 595, 842));

        PageContentContext* contentContext = pdfWriter.StartPageContentContext(page);

        // 한글 폰트 로드
        PDFUsedFont* fontBold = pdfWriter.GetFontForFile("C:\\Windows\\Fonts\\malgunbd.ttf");
        PDFUsedFont* fontRegular = pdfWriter.GetFontForFile("C:\\Windows\\Fonts\\malgun.ttf");

        double yPos = 800.0;
        const double leftMargin = 50.0;
        const double tableWidth = 495.0;
        const double rowHeight = 20.0;
        const double headerHeight = 25.0;
        const double sectionSpacing = 15.0;

        // === 제목 ===
        contentContext->BT();
        contentContext->k(0, 0, 0, 1);
        contentContext->Tf(fontBold, 24);
        contentContext->Tm(1, 0, 0, 1, 210, yPos);
        contentContext->Tj("거래 내역서");
        contentContext->ET();
        yPos -= 40.0;

        // === 카테고리/항목별 그룹화 ===
        // Key: "Category|Item", Value: {amount, count}
        std::map<std::string, std::pair<int64_t, int>> incomeGroups;
        std::map<std::string, std::pair<int64_t, int>> expenseGroups;
        int64_t totalIncome = 0;
        int64_t totalExpense = 0;

        for (const auto& transaction : transactions)
        {
            std::string key = transaction.Category + "|" + transaction.Item;

            if (transaction.Type == E_MLTransactionType::Income)
            {
                incomeGroups[key].first += transaction.Amount;
                incomeGroups[key].second += 1;
                totalIncome += transaction.Amount;
            }
            else if (transaction.Type == E_MLTransactionType::Expense)
            {
                expenseGroups[key].first += transaction.Amount;
                expenseGroups[key].second += 1;
                totalExpense += transaction.Amount;
            }
        }

        // === 수입 섹션 ===
        if (!incomeGroups.empty())
        {
            contentContext->BT();
            contentContext->Tf(fontBold, 16);
            contentContext->Tm(1, 0, 0, 1, leftMargin, yPos);
            contentContext->Tj("수입");
            contentContext->ET();
            yPos -= 25.0;

            // 헤더 행 (카테고리, 항목, 금액, 거래건수)
            drawTableRow(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                         "카테고리 / 항목", "금액 / 거래건수", fontBold, 11, true);
            yPos -= headerHeight;

            // 데이터 행
            for (const auto& group : incomeGroups)
            {
                size_t pos = group.first.find('|');
                std::string category = group.first.substr(0, pos);
                std::string item = group.first.substr(pos + 1);
                std::string label = category + " / " + item;

                std::string amountStr = formatAmount(group.second.first) + "원 / " +
                                       std::to_string(group.second.second) + "건";

                drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                             label, amountStr, fontRegular, 10, false);
                yPos -= rowHeight;
            }

            // 수입 소계
            std::string incomeSubtotal = formatAmount(totalIncome) + "원";
            drawTableRow(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                         "수입 소계", incomeSubtotal, fontBold, 11, true);
            yPos -= headerHeight + sectionSpacing;
        }

        // === 지출 섹션 ===
        if (!expenseGroups.empty())
        {
            contentContext->BT();
            contentContext->Tf(fontBold, 16);
            contentContext->Tm(1, 0, 0, 1, leftMargin, yPos);
            contentContext->Tj("지출");
            contentContext->ET();
            yPos -= 25.0;

            // 헤더 행
            drawTableRow(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                         "카테고리 / 항목", "금액 / 거래건수", fontBold, 11, true);
            yPos -= headerHeight;

            // 데이터 행
            for (const auto& group : expenseGroups)
            {
                size_t pos = group.first.find('|');
                std::string category = group.first.substr(0, pos);
                std::string item = group.first.substr(pos + 1);
                std::string label = category + " / " + item;

                std::string amountStr = formatAmount(group.second.first) + "원 / " +
                                       std::to_string(group.second.second) + "건";

                drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                             label, amountStr, fontRegular, 10, false);
                yPos -= rowHeight;
            }

            // 지출 소계
            std::string expenseSubtotal = formatAmount(totalExpense) + "원";
            drawTableRow(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                         "지출 소계", expenseSubtotal, fontBold, 11, true);
            yPos -= headerHeight + sectionSpacing;
        }

        // === 전체 합계 ===
        contentContext->BT();
        contentContext->Tf(fontBold, 16);
        contentContext->Tm(1, 0, 0, 1, leftMargin, yPos);
        contentContext->Tj("합계");
        contentContext->ET();
        yPos -= 25.0;

        drawTableRow(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                     "항목", "금액", fontBold, 11, true);
        yPos -= headerHeight;

        drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                     "수입", formatAmount(totalIncome) + "원", fontRegular, 10, false);
        yPos -= rowHeight;

        drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                     "지출", formatAmount(totalExpense) + "원", fontRegular, 10, false);
        yPos -= rowHeight;

        int64_t balance = totalIncome - totalExpense;
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                     "잔액", formatAmount(balance) + "원", fontBold, 11, true);

        pdfWriter.EndPageContentContext(contentContext);
        pdfWriter.WritePageAndRelease(page);
        pdfWriter.EndPDF();

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
