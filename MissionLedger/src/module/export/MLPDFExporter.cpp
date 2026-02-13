#include "MLPDFExporter.h"
#include "PDFWriter/PDFWriter.h"
#include "PDFWriter/PDFPage.h"
#include "PDFWriter/PageContentContext.h"
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>

// 금액 포맷 헬퍼 (천 단위 콤마)
static std::string formatAmountHelper(int64_t amount)
{
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << amount;
    return ss.str();
}

// 파일 경로에서 확장자 없는 파일명 추출
static std::string extractFilename(const std::string& filePath)
{
    // 경로 구분자 찾기 (\ 또는 /)
    size_t lastSlash = filePath.find_last_of("/\\");
    std::string filename = (lastSlash != std::string::npos) ? filePath.substr(lastSlash + 1) : filePath;

    // 확장자 제거
    size_t lastDot = filename.find_last_of('.');
    if (lastDot != std::string::npos) {
        filename = filename.substr(0, lastDot);
    }

    return filename;
}

// 폰트 로드 헬퍼 (폴백 지원)
static PDFUsedFont* loadFontWithFallback(PDFWriter& pdfWriter, bool bold)
{
    const char* primaryFont = bold ? "C:\\Windows\\Fonts\\malgunbd.ttf" : "C:\\Windows\\Fonts\\malgun.ttf";
    const char* fallbackFont = bold ? "C:\\Windows\\Fonts\\arialbd.ttf" : "C:\\Windows\\Fonts\\arial.ttf";

    PDFUsedFont* font = nullptr;

    try {
        font = pdfWriter.GetFontForFile(primaryFont);
    } catch (...) {
        // 주 폰트 로드 실패 시 폴백 폰트 시도
        try {
            font = pdfWriter.GetFontForFile(fallbackFont);
        } catch (...) {
            // 폴백도 실패하면 nullptr 반환 (호출자가 처리)
        }
    }

    return font;
}

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

        // 한글 폰트 로드 (폴백 지원)
        PDFUsedFont* fontBold = loadFontWithFallback(pdfWriter, true);
        PDFUsedFont* fontRegular = loadFontWithFallback(pdfWriter, false);

        if (!fontBold || !fontRegular) {
            return false;  // 폰트 로드 실패
        }

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

// 거래 내역 표 행 그리기 (6열: 구분, 항목, 내용, 금액, 년월일, 영수증번호)
static void drawTransactionRow(
    PageContentContext* ctx,
    double x, double y, double height,
    const std::string cols[6],
    const double colWidths[6],
    PDFUsedFont* font, double fontSize,
    bool isHeader)
{
    double totalWidth = 0;
    for (int i = 0; i < 6; i++) {
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
    for (int i = 0; i < 5; i++) {
        currentX += colWidths[i];
        ctx->m(currentX, y);
        ctx->l(currentX, y + height);
        ctx->S();
    }
    ctx->Q();

    // 텍스트 출력
    currentX = x;
    for (int i = 0; i < 6; i++)
    {
        ctx->BT();
        ctx->k(0, 0, 0, 1);
        ctx->Tf(font, fontSize);

        double textX;
        if (i == 3) { // 금액 열 (오른쪽 정렬)
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

// UTF-8 문자열을 지정된 너비에 맞게 여러 줄로 분할
static std::vector<std::string> wrapText(const std::string& text, double maxWidth, double fontSize)
{
    std::vector<std::string> lines;
    if (text.empty()) {
        lines.push_back("");
        return lines;
    }

    // 한글 문자는 fontSize와 거의 동일한 너비를 차지
    // 여유 공간을 위해 약간 크게 설정
    const double avgCharWidth = fontSize * 1.1;  // 안전한 여백 확보
    const int maxCharsPerLine = static_cast<int>(maxWidth / avgCharWidth);

    if (maxCharsPerLine <= 0) {
        lines.push_back(text);
        return lines;
    }

    // UTF-8 문자 단위로 처리
    std::string currentLine;
    int charCount = 0;
    size_t i = 0;

    while (i < text.length())
    {
        // UTF-8 문자 하나의 바이트 수 계산
        unsigned char c = text[i];
        int charBytes = 1;
        if ((c & 0x80) == 0) {
            charBytes = 1;  // ASCII
        } else if ((c & 0xE0) == 0xC0) {
            charBytes = 2;
        } else if ((c & 0xF0) == 0xE0) {
            charBytes = 3;  // 한글 등
        } else if ((c & 0xF8) == 0xF0) {
            charBytes = 4;
        }

        // 현재 문자 추출
        std::string currentChar = text.substr(i, charBytes);

        // 줄 길이 체크
        if (charCount >= maxCharsPerLine && !currentLine.empty())
        {
            lines.push_back(currentLine);
            currentLine.clear();
            charCount = 0;
        }

        currentLine += currentChar;
        charCount++;
        i += charBytes;
    }

    // 마지막 줄 추가
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }

    return lines;
}

// 카테고리 그룹 그리기 (Category 셀 병합, 내부에 여러 Item 그룹 포함)
static void drawCategoryGroup(
    PDFWriter& pdfWriter,
    PDFPage*& page,
    PageContentContext*& ctx,
    double& yPos,
    const std::string& category,
    const std::map<std::string, std::vector<FMLTransactionData>>& itemGroups,
    const double colWidths[6],
    PDFUsedFont* font,
    PDFUsedFont* fontBold,
    double fontSize,
    double rowHeight,
    double leftMargin,
    double headerHeight,
    const std::string headers[6])
{
    if (itemGroups.empty()) return;

    double categoryStartY = yPos;
    double x = leftMargin;
    double itemX = x + colWidths[0];

    // 각 항목 그룹 그리기
    for (const auto& itemGroup : itemGroups)
    {
        const auto& transactions = itemGroup.second;

        // 항목 텍스트 래핑 (모든 줄 사용)
        std::vector<std::string> itemLines = wrapText(itemGroup.first, colWidths[1] - 6, fontSize);

        // 각 거래의 높이 계산 (내용 래핑 고려)
        std::vector<double> transactionHeights;
        double itemHeight = 0;

        for (const auto& transaction : transactions)
        {
            std::vector<std::string> descLines = wrapText(transaction.Description, colWidths[2] - 6, fontSize);
            size_t lineCount = descLines.size();
            if (lineCount == 0) lineCount = 1;

            double txHeight = rowHeight * lineCount;
            transactionHeights.push_back(txHeight);
            itemHeight += txHeight;
        }

        // 항목 텍스트의 최소 필요 높이 확보
        double itemTextHeight = itemLines.size() * rowHeight;
        if (itemHeight < itemTextHeight) {
            // 부족한 높이를 첫 번째 거래에 추가
            double deficit = itemTextHeight - itemHeight;
            if (!transactionHeights.empty()) {
                transactionHeights[0] += deficit;
            }
            itemHeight = itemTextHeight;
        }

        // 페이지 공간 체크
        if (yPos - itemHeight < 30)
        {
            // 현재 페이지의 구분 셀 완성
            double categoryHeight = categoryStartY - yPos;
            if (categoryHeight > 0) {
                ctx->q();
                ctx->w(0.5);
                ctx->k(0, 0, 0, 1);
                ctx->re(x, yPos, colWidths[0], categoryHeight);
                ctx->S();
                ctx->Q();

                ctx->BT();
                ctx->k(0, 0, 0, 1);
                ctx->Tf(font, fontSize);
                double textY = yPos + categoryHeight / 2 - fontSize / 2 + 2;
                ctx->Tm(1, 0, 0, 1, x + 3, textY);
                ctx->Tj(category);
                ctx->ET();
            }

            // 새 페이지 생성
            pdfWriter.EndPageContentContext(ctx);
            pdfWriter.WritePageAndRelease(page);

            page = new PDFPage();
            page->SetMediaBox(PDFRectangle(0, 0, 595, 842));
            ctx = pdfWriter.StartPageContentContext(page);
            yPos = 800.0;

            drawTransactionRow(ctx, leftMargin, yPos, headerHeight,
                              headers, colWidths, fontBold, 9, true);
            yPos -= headerHeight;

            categoryStartY = yPos;
        }

        // 2열(항목) 병합 셀 그리기
        ctx->q();
        ctx->w(0.5);
        ctx->k(0, 0, 0, 1);
        ctx->re(itemX, yPos - itemHeight, colWidths[1], itemHeight);
        ctx->S();
        ctx->Q();

        // 항목 텍스트 (세로 중앙 정렬, 여러 줄 표시)
        // 전체 텍스트의 시각적 높이 계산
        double totalItemVisualHeight = fontSize * itemLines.size() + 3 * (itemLines.size() - 1);
        double cellBottom = yPos - itemHeight;
        // 위쪽 여백 계산 (최소 여백 보장)
        double topPadding = (itemHeight - totalItemVisualHeight) / 2;
        if (topPadding < 2) topPadding = 2;  // 최소 여백
        // 첫 줄의 baseline 위치
        double itemTextY = cellBottom + itemHeight - topPadding - fontSize;
        for (const auto& line : itemLines)
        {
            ctx->BT();
            ctx->k(0, 0, 0, 1);
            ctx->Tf(font, fontSize);
            ctx->Tm(1, 0, 0, 1, itemX + 3, itemTextY);
            ctx->Tj(line);
            ctx->ET();
            itemTextY -= fontSize + 3;
        }

        // 나머지 열 - 개별 행으로 그리기
        double contentStartX = itemX + colWidths[1];
        double currentY = yPos;

        for (size_t i = 0; i < transactions.size(); i++)
        {
            const auto& transaction = transactions[i];
            double txHeight = transactionHeights[i];
            double rowY = currentY - txHeight;

            // 내용 셀 (여러 줄 지원)
            ctx->q();
            ctx->w(0.5);
            ctx->k(0, 0, 0, 1);
            ctx->re(contentStartX, rowY, colWidths[2], txHeight);
            ctx->S();
            ctx->Q();

            std::vector<std::string> descLines = wrapText(transaction.Description, colWidths[2] - 6, fontSize);
            // 내용 텍스트 (세로 중앙 정렬)
            // 전체 텍스트의 시각적 높이 계산
            double totalDescVisualHeight = fontSize * descLines.size() + 3 * (descLines.size() - 1);
            // 위쪽 여백 계산 (최소 여백 보장)
            double descTopPadding = (txHeight - totalDescVisualHeight) / 2;
            if (descTopPadding < 2) descTopPadding = 2;  // 최소 여백
            // 첫 줄의 baseline 위치
            double textY = rowY + txHeight - descTopPadding - fontSize;
            for (const auto& line : descLines)
            {
                ctx->BT();
                ctx->k(0, 0, 0, 1);
                ctx->Tf(font, fontSize);
                ctx->Tm(1, 0, 0, 1, contentStartX + 3, textY);
                ctx->Tj(line);
                ctx->ET();
                textY -= fontSize + 3;
            }

            // 금액 셀 (오른쪽 정렬)
            double amountX = contentStartX + colWidths[2];
            ctx->q();
            ctx->w(0.5);
            ctx->k(0, 0, 0, 1);
            ctx->re(amountX, rowY, colWidths[3], txHeight);
            ctx->S();
            ctx->Q();

            std::string amountStr = formatAmountHelper(transaction.Amount) + "원";
            double estimatedWidth = amountStr.length() * fontSize * 0.45;
            ctx->BT();
            ctx->k(0, 0, 0, 1);
            ctx->Tf(font, fontSize);
            ctx->Tm(1, 0, 0, 1, amountX + colWidths[3] - estimatedWidth - 5, rowY + (txHeight - fontSize) / 2 + 2);
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
            ctx->Tf(font, fontSize);
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
            ctx->Tf(font, fontSize);
            ctx->Tm(1, 0, 0, 1, receiptX + 3, rowY + (txHeight - fontSize) / 2 + 2);
            ctx->Tj(transaction.ReceiptNumber);
            ctx->ET();

            currentY = rowY;
        }

        yPos = currentY;
    }

    // 마지막 페이지의 구분 셀 완성
    double categoryHeight = categoryStartY - yPos;
    if (categoryHeight > 0) {
        ctx->q();
        ctx->w(0.5);
        ctx->k(0, 0, 0, 1);
        ctx->re(x, yPos, colWidths[0], categoryHeight);
        ctx->S();
        ctx->Q();

        ctx->BT();
        ctx->k(0, 0, 0, 1);
        ctx->Tf(font, fontSize);
        double textY = yPos + categoryHeight / 2 - fontSize / 2 + 2;
        ctx->Tm(1, 0, 0, 1, x + 3, textY);
        ctx->Tj(category);
        ctx->ET();
    }
}

// 거래 내역서 PDF 생성 (개별 거래 내역 표시)
bool FMLPDFExporter::ExportTransactionList(
    const std::vector<FMLTransactionData>& transactions,
    const std::string& filePath)
{
    try {
        PDFWriter pdfWriter;
        pdfWriter.StartPDF(filePath, ePDFVersion13);

        // 페이지 생성 (A4 portrait: 595 x 842 points)
        PDFPage* page = new PDFPage();
        page->SetMediaBox(PDFRectangle(0, 0, 595, 842));

        PageContentContext* contentContext = pdfWriter.StartPageContentContext(page);

        // 한글 폰트 로드 (폴백 지원)
        PDFUsedFont* fontBold = loadFontWithFallback(pdfWriter, true);
        PDFUsedFont* fontRegular = loadFontWithFallback(pdfWriter, false);

        if (!fontBold || !fontRegular) {
            return false;  // 폰트 로드 실패
        }

        // 파일명 추출 (제목에 사용)
        std::string filename = extractFilename(filePath);

        double yPos = 800.0;
        const double leftMargin = 30.0;
        const double rowHeight = 18.0;
        const double headerHeight = 22.0;
        const double sectionSpacing = 20.0;  // 섹션 간격 증가: 12 → 20

        // 열 너비 정의 (구분, 항목, 내용, 금액, 년월일, 영수증번호) - 세로 방향 최적화
        const double colWidths[6] = {50, 60, 200, 95, 70, 50};  // 구분, 항목, 내용, 금액, 년월일, 영수증번호
        double tableWidth = 0;
        for (int i = 0; i < 6; i++) {
            tableWidth += colWidths[i];
        }

        // === 제목 (파일명) ===
        contentContext->BT();
        contentContext->k(0, 0, 0, 1);
        contentContext->Tf(fontBold, 18);
        contentContext->Tm(1, 0, 0, 1, leftMargin, yPos);
        contentContext->Tj(filename);
        contentContext->ET();
        yPos -= 25.0;

        // === 합계 요약 섹션 ===
        int64_t totalIncome = 0;
        int64_t totalExpense = 0;

        // 수입/지출 분리
        std::vector<FMLTransactionData> incomeList;
        std::vector<FMLTransactionData> expenseList;

        for (const auto& transaction : transactions)
        {
            if (transaction.Type == E_MLTransactionType::Income)
            {
                incomeList.push_back(transaction);
                totalIncome += transaction.Amount;
            }
            else if (transaction.Type == E_MLTransactionType::Expense)
            {
                expenseList.push_back(transaction);
                totalExpense += transaction.Amount;
            }
        }

        int64_t balance = totalIncome - totalExpense;

        // 요약 표 (3행: 총 수입, 총 지출, 남은 금액)
        contentContext->BT();
        contentContext->Tf(fontBold, 14);
        contentContext->Tm(1, 0, 0, 1, leftMargin, yPos);
        contentContext->Tj("합계");
        contentContext->ET();
        yPos -= 30.0;

        const double summaryWidth = 300.0;
        drawTableRow(contentContext, leftMargin, yPos, summaryWidth, headerHeight,
                     "총 수입", formatAmount(totalIncome) + "원", fontBold, 10, true);
        yPos -= headerHeight;

        drawTableRow(contentContext, leftMargin, yPos, summaryWidth, headerHeight,
                     "총 지출", formatAmount(totalExpense) + "원", fontBold, 10, true);
        yPos -= headerHeight;

        drawTableRow(contentContext, leftMargin, yPos, summaryWidth, headerHeight,
                     "남은 금액", formatAmount(balance) + "원", fontBold, 10, true);
        yPos -= headerHeight + sectionSpacing;

        // === 카테고리별 합계 ===
        // 카테고리별 금액 집계
        std::map<std::string, int64_t> incomeCategoryTotals;
        std::map<std::string, int64_t> expenseCategoryTotals;

        for (const auto& transaction : incomeList) {
            incomeCategoryTotals[transaction.Category] += transaction.Amount;
        }
        for (const auto& transaction : expenseList) {
            expenseCategoryTotals[transaction.Category] += transaction.Amount;
        }

        // 수입 카테고리 합계 표 (표 형식)
        if (!incomeCategoryTotals.empty()) {
            contentContext->BT();
            contentContext->Tf(fontBold, 11);
            contentContext->Tm(1, 0, 0, 1, leftMargin, yPos);
            contentContext->Tj("수입 카테고리별");
            contentContext->ET();
            yPos -= 18.0;

            const double categoryColWidth = 90.0;
            const double categoryRowHeight = 16.0;
            size_t numCols = incomeCategoryTotals.size() + 1; // +1 for 총합
            double tableWidth = categoryColWidth * numCols;

            // 헤더 행 (카테고리명들)
            double currentX = leftMargin;
            for (const auto& categoryPair : incomeCategoryTotals) {
                // 셀 테두리
                contentContext->q();
                contentContext->w(0.5);
                contentContext->k(0, 0, 0, 1);
                contentContext->k(0, 0, 0, 0.1);
                contentContext->re(currentX, yPos, categoryColWidth, categoryRowHeight);
                contentContext->B();
                contentContext->Q();

                // 텍스트
                contentContext->BT();
                contentContext->k(0, 0, 0, 1);
                contentContext->Tf(fontRegular, 8);
                contentContext->Tm(1, 0, 0, 1, currentX + 3, yPos + 4);
                contentContext->Tj(categoryPair.first);
                contentContext->ET();

                currentX += categoryColWidth;
            }
            // 총합 열
            contentContext->q();
            contentContext->w(0.5);
            contentContext->k(0, 0, 0, 1);
            contentContext->k(0, 0, 0, 0.1);
            contentContext->re(currentX, yPos, categoryColWidth, categoryRowHeight);
            contentContext->B();
            contentContext->Q();

            contentContext->BT();
            contentContext->k(0, 0, 0, 1);
            contentContext->Tf(fontBold, 8);
            contentContext->Tm(1, 0, 0, 1, currentX + 3, yPos + 4);
            contentContext->Tj("총 수입");
            contentContext->ET();
            yPos -= categoryRowHeight;

            // 금액 행
            currentX = leftMargin;
            for (const auto& categoryPair : incomeCategoryTotals) {
                // 셀 테두리
                contentContext->q();
                contentContext->w(0.5);
                contentContext->k(0, 0, 0, 1);
                contentContext->re(currentX, yPos, categoryColWidth, categoryRowHeight);
                contentContext->S();
                contentContext->Q();

                // 금액 (오른쪽 정렬)
                std::string amountStr = formatAmount(categoryPair.second);
                double estimatedWidth = amountStr.length() * 8 * 0.45;
                contentContext->BT();
                contentContext->k(0, 0, 0, 1);
                contentContext->Tf(fontRegular, 8);
                contentContext->Tm(1, 0, 0, 1, currentX + categoryColWidth - estimatedWidth - 3, yPos + 4);
                contentContext->Tj(amountStr);
                contentContext->ET();

                currentX += categoryColWidth;
            }
            // 총합
            contentContext->q();
            contentContext->w(0.5);
            contentContext->k(0, 0, 0, 1);
            contentContext->re(currentX, yPos, categoryColWidth, categoryRowHeight);
            contentContext->S();
            contentContext->Q();

            std::string totalStr = formatAmount(totalIncome);
            double estimatedWidth = totalStr.length() * 8 * 0.45;
            contentContext->BT();
            contentContext->k(0, 0, 0, 1);
            contentContext->Tf(fontBold, 8);
            contentContext->Tm(1, 0, 0, 1, currentX + categoryColWidth - estimatedWidth - 3, yPos + 4);
            contentContext->Tj(totalStr);
            contentContext->ET();
            yPos -= categoryRowHeight + 10;
        }

        // 지출 카테고리 합계 표 (표 형식)
        if (!expenseCategoryTotals.empty()) {
            contentContext->BT();
            contentContext->Tf(fontBold, 11);
            contentContext->Tm(1, 0, 0, 1, leftMargin, yPos);
            contentContext->Tj("지출 카테고리별");
            contentContext->ET();
            yPos -= 18.0;

            const double categoryColWidth = 90.0;
            const double categoryRowHeight = 16.0;
            size_t numCols = expenseCategoryTotals.size() + 1;

            // 헤더 행
            double currentX = leftMargin;
            for (const auto& categoryPair : expenseCategoryTotals) {
                contentContext->q();
                contentContext->w(0.5);
                contentContext->k(0, 0, 0, 1);
                contentContext->k(0, 0, 0, 0.1);
                contentContext->re(currentX, yPos, categoryColWidth, categoryRowHeight);
                contentContext->B();
                contentContext->Q();

                contentContext->BT();
                contentContext->k(0, 0, 0, 1);
                contentContext->Tf(fontRegular, 8);
                contentContext->Tm(1, 0, 0, 1, currentX + 3, yPos + 4);
                contentContext->Tj(categoryPair.first);
                contentContext->ET();

                currentX += categoryColWidth;
            }
            // 총합 열
            contentContext->q();
            contentContext->w(0.5);
            contentContext->k(0, 0, 0, 1);
            contentContext->k(0, 0, 0, 0.1);
            contentContext->re(currentX, yPos, categoryColWidth, categoryRowHeight);
            contentContext->B();
            contentContext->Q();

            contentContext->BT();
            contentContext->k(0, 0, 0, 1);
            contentContext->Tf(fontBold, 8);
            contentContext->Tm(1, 0, 0, 1, currentX + 3, yPos + 4);
            contentContext->Tj("총 지출");
            contentContext->ET();
            yPos -= categoryRowHeight;

            // 금액 행
            currentX = leftMargin;
            for (const auto& categoryPair : expenseCategoryTotals) {
                contentContext->q();
                contentContext->w(0.5);
                contentContext->k(0, 0, 0, 1);
                contentContext->re(currentX, yPos, categoryColWidth, categoryRowHeight);
                contentContext->S();
                contentContext->Q();

                std::string amountStr = formatAmount(categoryPair.second);
                double estimatedWidth = amountStr.length() * 8 * 0.45;
                contentContext->BT();
                contentContext->k(0, 0, 0, 1);
                contentContext->Tf(fontRegular, 8);
                contentContext->Tm(1, 0, 0, 1, currentX + categoryColWidth - estimatedWidth - 3, yPos + 4);
                contentContext->Tj(amountStr);
                contentContext->ET();

                currentX += categoryColWidth;
            }
            // 총합
            contentContext->q();
            contentContext->w(0.5);
            contentContext->k(0, 0, 0, 1);
            contentContext->re(currentX, yPos, categoryColWidth, categoryRowHeight);
            contentContext->S();
            contentContext->Q();

            std::string totalStr = formatAmount(totalExpense);
            double estimatedWidth = totalStr.length() * 8 * 0.45;
            contentContext->BT();
            contentContext->k(0, 0, 0, 1);
            contentContext->Tf(fontBold, 8);
            contentContext->Tm(1, 0, 0, 1, currentX + categoryColWidth - estimatedWidth - 3, yPos + 4);
            contentContext->Tj(totalStr);
            contentContext->ET();
            yPos -= categoryRowHeight + sectionSpacing;
        }

        // === 수입 섹션 ===
        if (!incomeList.empty())
        {
            contentContext->BT();
            contentContext->Tf(fontBold, 14);
            contentContext->Tm(1, 0, 0, 1, leftMargin, yPos);
            contentContext->Tj("수입");
            contentContext->ET();
            yPos -= 25.0;

            // 헤더 행
            const std::string headers[6] = {"구분", "항목", "내용", "수입금액", "년월일", "영수증번호"};
            drawTransactionRow(contentContext, leftMargin, yPos, headerHeight,
                              headers, colWidths, fontBold, 9, true);
            yPos -= headerHeight;

            // 계층적 그룹화: Category → Item → Transactions
            std::map<std::string, std::map<std::string, std::vector<FMLTransactionData>>> incomeCategories;
            for (const auto& transaction : incomeList)
            {
                incomeCategories[transaction.Category][transaction.Item].push_back(transaction);
            }

            // 카테고리별로 그리기
            for (const auto& categoryPair : incomeCategories)
            {
                const auto& category = categoryPair.first;
                const auto& itemGroups = categoryPair.second;

                drawCategoryGroup(pdfWriter, page, contentContext, yPos, category, itemGroups,
                                 colWidths, fontRegular, fontBold, 8, rowHeight, leftMargin, headerHeight, headers);
            }

            yPos -= sectionSpacing;
        }

        // === 지출 섹션 ===
        if (!expenseList.empty())
        {
            contentContext->BT();
            contentContext->Tf(fontBold, 14);
            contentContext->Tm(1, 0, 0, 1, leftMargin, yPos);
            contentContext->Tj("지출");
            contentContext->ET();
            yPos -= 25.0;

            // 헤더 행
            const std::string headers[6] = {"구분", "항목", "내용", "지출금액", "년월일", "영수증번호"};
            drawTransactionRow(contentContext, leftMargin, yPos, headerHeight,
                              headers, colWidths, fontBold, 9, true);
            yPos -= headerHeight;

            // 계층적 그룹화: Category → Item → Transactions
            std::map<std::string, std::map<std::string, std::vector<FMLTransactionData>>> expenseCategories;
            for (const auto& transaction : expenseList)
            {
                expenseCategories[transaction.Category][transaction.Item].push_back(transaction);
            }

            // 카테고리별로 그리기
            for (const auto& categoryPair : expenseCategories)
            {
                const auto& category = categoryPair.first;
                const auto& itemGroups = categoryPair.second;

                drawCategoryGroup(pdfWriter, page, contentContext, yPos, category, itemGroups,
                                 colWidths, fontRegular, fontBold, 8, rowHeight, leftMargin, headerHeight, headers);
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
