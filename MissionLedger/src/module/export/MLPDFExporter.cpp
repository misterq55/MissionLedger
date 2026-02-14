#include "MLPDFExporter.h"
#include "PDFWriter/PDFWriter.h"
#include "PDFWriter/PDFPage.h"
#include "PDFWriter/PageContentContext.h"
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <fstream>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <limits.h>
    #include <sys/stat.h>
#endif

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

// 파일 존재 확인 (크로스 플랫폼)
static bool fileExists(const std::string& path)
{
#ifdef _WIN32
    DWORD attrib = GetFileAttributesA(path.c_str());
    return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));
#endif
}

// 실행 파일 경로 가져오기 (크로스 플랫폼)
static std::string getExecutablePath()
{
#ifdef _WIN32
    char buffer[MAX_PATH];
    DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);
    if (length > 0 && length < MAX_PATH) {
        // 파일명 제거, 디렉토리만 반환
        std::string fullPath(buffer, length);
        size_t lastSlash = fullPath.find_last_of("\\/");
        if (lastSlash != std::string::npos) {
            return fullPath.substr(0, lastSlash);
        }
    }
    return "";
#else
    char buffer[PATH_MAX];
    ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (length > 0) {
        buffer[length] = '\0';
        std::string fullPath(buffer);
        size_t lastSlash = fullPath.find_last_of('/');
        if (lastSlash != std::string::npos) {
            return fullPath.substr(0, lastSlash);
        }
    }
    return "";
#endif
}

// 폰트 파일 검색 (크로스 플랫폼)
// 검색 우선순위:
// 1. 실행 파일 근처 fonts/ 디렉토리 (배포 폰트)
// 2. 시스템 폰트 디렉토리 (플랫폼별)
// 3. Fallback 폰트 (있는 경우)
static std::string findFontFile(const std::string& fontName, bool searchKorean)
{
    std::vector<std::string> searchPaths;

    // 1. 배포 폰트 검색 (실행 파일 근처 fonts/ 디렉토리)
    std::string exeDir = getExecutablePath();
    if (!exeDir.empty()) {
#ifdef _WIN32
        std::string bundledFontPath = exeDir + "\\fonts\\" + fontName;
#else
        std::string bundledFontPath = exeDir + "/fonts/" + fontName;
#endif
        if (fileExists(bundledFontPath)) {
            return bundledFontPath;
        }
    }

    // 2. 시스템 폰트 디렉토리 검색 (플랫폼별)
    if (searchKorean) {
        // 한글 폰트 검색
#ifdef _WIN32
        // Windows: Malgun Gothic (시스템 폰트, 재배포 불가)
        char winDir[MAX_PATH];
        DWORD result = GetEnvironmentVariableA("WINDIR", winDir, MAX_PATH);
        if (result > 0 && result < MAX_PATH) {
            std::string fontPath = std::string(winDir) + "\\Fonts\\" + fontName;
            searchPaths.push_back(fontPath);
        }
#elif defined(__APPLE__)
        // macOS: Apple SD Gothic Neo
        searchPaths.push_back("/Library/Fonts/AppleSDGothicNeo.ttc");
        searchPaths.push_back("/System/Library/Fonts/AppleSDGothicNeo.ttc");
        // Noto Sans CJK KR (사용자가 설치한 경우)
        searchPaths.push_back("/Library/Fonts/NotoSansCJK-Regular.ttc");
        searchPaths.push_back("/System/Library/Fonts/NotoSansCJK-Regular.ttc");
#else
        // Linux: Noto Sans CJK KR, Nanum Gothic
        searchPaths.push_back("/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc");
        searchPaths.push_back("/usr/share/fonts/truetype/noto/NotoSansKR-Regular.ttf");
        searchPaths.push_back("/usr/share/fonts/truetype/nanum/NanumGothic.ttf");
#endif
    } else {
        // 영문 폰트 검색 (Arial 등)
#ifdef _WIN32
        char winDir[MAX_PATH];
        DWORD result = GetEnvironmentVariableA("WINDIR", winDir, MAX_PATH);
        if (result > 0 && result < MAX_PATH) {
            std::string fontPath = std::string(winDir) + "\\Fonts\\" + fontName;
            searchPaths.push_back(fontPath);
        }
#elif defined(__APPLE__)
        searchPaths.push_back("/Library/Fonts/Arial.ttf");
        searchPaths.push_back("/System/Library/Fonts/Supplemental/Arial.ttf");
#else
        // Linux: DejaVu Sans (Arial 대안)
        searchPaths.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
        searchPaths.push_back("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
#endif
    }

    // 검색 경로에서 존재하는 첫 번째 파일 반환
    for (const auto& path : searchPaths) {
        if (fileExists(path)) {
            return path;
        }
    }

    // 폰트를 찾지 못함
    return "";
}

// 폰트 로드 헬퍼 (폴백 지원, 크로스 플랫폼)
static PDFUsedFont* loadFontWithFallback(PDFWriter& pdfWriter, bool bold)
{
    PDFUsedFont* font = nullptr;

    // Fallback 체인:
    // 1. 한글 폰트 (Noto Sans KR / Malgun Gothic / Apple SD Gothic Neo / Noto Sans CJK KR)
    // 2. 영문 폰트 (Arial / Helvetica / DejaVu Sans)
    // 3. 플랫폼별 최후 폴백

    // 1단계: 한글 폰트 시도
    std::string koreanFont;
    if (bold) {
        // Bold 폰트: Noto Sans KR Bold 또는 Malgun Gothic Bold
        koreanFont = findFontFile("NotoSansKR-Bold.ttf", true);
        if (koreanFont.empty()) {
            koreanFont = findFontFile("malgunbd.ttf", true);
        }
    } else {
        // Regular 폰트: Noto Sans KR Regular 또는 Malgun Gothic
        koreanFont = findFontFile("NotoSansKR-Regular.ttf", true);
        if (koreanFont.empty()) {
            koreanFont = findFontFile("malgun.ttf", true);
        }
    }

    if (!koreanFont.empty()) {
        try {
            font = pdfWriter.GetFontForFile(koreanFont);
            if (font != nullptr) {
                return font;
            }
        } catch (...) {
            // 로드 실패, 다음 폴백 시도
        }
    }

    // 2단계: 영문 폰트 시도 (Arial)
    std::string englishFont;
    if (bold) {
        englishFont = findFontFile("arialbd.ttf", false);
    } else {
        englishFont = findFontFile("arial.ttf", false);
    }

    if (!englishFont.empty()) {
        try {
            font = pdfWriter.GetFontForFile(englishFont);
            if (font != nullptr) {
                return font;
            }
        } catch (...) {
            // 로드 실패, 다음 폴백 시도
        }
    }

    // 3단계: 플랫폼별 최후 폴백
#ifdef _WIN32
    // Windows: 기본 시스템 폰트 경로 직접 시도
    const char* lastResortFont = bold ? "C:\\Windows\\Fonts\\arialbd.ttf" : "C:\\Windows\\Fonts\\arial.ttf";
#elif defined(__APPLE__)
    // macOS: Helvetica (항상 존재)
    const char* lastResortFont = "/System/Library/Fonts/Helvetica.ttc";
#else
    // Linux: DejaVu Sans (대부분 배포판에 기본 설치)
    const char* lastResortFont = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
#endif

    try {
        font = pdfWriter.GetFontForFile(lastResortFont);
    } catch (...) {
        // 모든 폴백 실패, nullptr 반환
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

// 3컬럼 테이블 행 그리기 (카테고리/항목, 예산, 실적)
static void drawTableRow3Col(
    PageContentContext* ctx,
    double x, double y,
    double width, double height,
    const std::string& col1, const std::string& col2, const std::string& col3,
    PDFUsedFont* font, double fontSize,
    bool isHeader)
{
    const double col1Width = width * 0.5;  // 카테고리/항목: 50%
    const double col2Width = width * 0.25; // 예산: 25%
    const double col3Width = width * 0.25; // 실적: 25%
    const double padding = 5.0;

    // 헤더 배경 (회색)
    if (isHeader)
    {
        ctx->q();
        ctx->k(0, 0, 0, 0.1);
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

    // 세로 구분선 2개
    ctx->m(x + col1Width, y);
    ctx->l(x + col1Width, y + height);
    ctx->S();

    ctx->m(x + col1Width + col2Width, y);
    ctx->l(x + col1Width + col2Width, y + height);
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

    // 두 번째 열 (오른쪽 정렬)
    ctx->BT();
    ctx->k(0, 0, 0, 1);
    ctx->Tf(font, fontSize);
    double estimatedWidth2 = col2.length() * fontSize * 0.4;
    ctx->Tm(1, 0, 0, 1, x + col1Width + col2Width - estimatedWidth2 - padding - 5, y + (height - fontSize) / 2 + 2);
    ctx->Tj(col2);
    ctx->ET();

    // 세 번째 열 (오른쪽 정렬)
    ctx->BT();
    ctx->k(0, 0, 0, 1);
    ctx->Tf(font, fontSize);
    double estimatedWidth3 = col3.length() * fontSize * 0.4;
    ctx->Tm(1, 0, 0, 1, x + width - estimatedWidth3 - padding - 5, y + (height - fontSize) / 2 + 2);
    ctx->Tj(col3);
    ctx->ET();
}

// 4컬럼 테이블 헤더 그리기 (카테고리, 항목, 예산, 실적)
static void drawTableRow4ColHeader(
    PageContentContext* ctx,
    double x, double y,
    double width, double height,
    const std::string& col1, const std::string& col2, const std::string& col3, const std::string& col4,
    PDFUsedFont* font, double fontSize)
{
    const double col1Width = width * 0.2;  // 카테고리: 20%
    const double col2Width = width * 0.3;  // 항목: 30%
    const double col3Width = width * 0.25; // 예산: 25%
    const double col4Width = width * 0.25; // 실적: 25%
    const double padding = 5.0;

    // 헤더 배경 (회색)
    ctx->q();
    ctx->k(0, 0, 0, 0.1);
    ctx->re(x, y, width, height);
    ctx->f();
    ctx->Q();

    // 테두리
    ctx->q();
    ctx->w(0.5);
    ctx->k(0, 0, 0, 1);
    ctx->re(x, y, width, height);
    ctx->S();

    // 세로 구분선 3개
    ctx->m(x + col1Width, y);
    ctx->l(x + col1Width, y + height);
    ctx->S();

    ctx->m(x + col1Width + col2Width, y);
    ctx->l(x + col1Width + col2Width, y + height);
    ctx->S();

    ctx->m(x + col1Width + col2Width + col3Width, y);
    ctx->l(x + col1Width + col2Width + col3Width, y + height);
    ctx->S();
    ctx->Q();

    // 텍스트 출력
    // 카테고리
    ctx->BT();
    ctx->k(0, 0, 0, 1);
    ctx->Tf(font, fontSize);
    ctx->Tm(1, 0, 0, 1, x + padding, y + (height - fontSize) / 2 + 2);
    ctx->Tj(col1);
    ctx->ET();

    // 항목
    ctx->BT();
    ctx->k(0, 0, 0, 1);
    ctx->Tf(font, fontSize);
    ctx->Tm(1, 0, 0, 1, x + col1Width + padding, y + (height - fontSize) / 2 + 2);
    ctx->Tj(col2);
    ctx->ET();

    // 예산 (오른쪽 정렬)
    ctx->BT();
    ctx->k(0, 0, 0, 1);
    ctx->Tf(font, fontSize);
    double estimatedWidth3 = col3.length() * fontSize * 0.4;
    ctx->Tm(1, 0, 0, 1, x + col1Width + col2Width + col3Width - estimatedWidth3 - padding, y + (height - fontSize) / 2 + 2);
    ctx->Tj(col3);
    ctx->ET();

    // 실적 (오른쪽 정렬)
    ctx->BT();
    ctx->k(0, 0, 0, 1);
    ctx->Tf(font, fontSize);
    double estimatedWidth4 = col4.length() * fontSize * 0.4;
    ctx->Tm(1, 0, 0, 1, x + width - estimatedWidth4 - padding, y + (height - fontSize) / 2 + 2);
    ctx->Tj(col4);
    ctx->ET();
}

// 4컬럼 테이블 항목 행 그리기 (항목, 예산, 실적만 - 카테고리는 나중에 병합)
static void drawTableRow4ColItem(
    PageContentContext* ctx,
    double x, double y,
    double width, double height,
    const std::string& col2, const std::string& col3, const std::string& col4,
    PDFUsedFont* font, double fontSize)
{
    const double col1Width = width * 0.2;  // 카테고리: 20%
    const double col2Width = width * 0.3;  // 항목: 30%
    const double col3Width = width * 0.25; // 예산: 25%
    const double col4Width = width * 0.25; // 실적: 25%
    const double padding = 5.0;

    double itemX = x + col1Width;

    // 테두리 (항목, 예산, 실적 셀만)
    ctx->q();
    ctx->w(0.5);
    ctx->k(0, 0, 0, 1);

    // 항목 셀
    ctx->re(itemX, y, col2Width, height);
    ctx->S();

    // 예산 셀
    ctx->re(itemX + col2Width, y, col3Width, height);
    ctx->S();

    // 실적 셀
    ctx->re(itemX + col2Width + col3Width, y, col4Width, height);
    ctx->S();

    ctx->Q();

    // 텍스트 출력
    // 항목 (왼쪽 정렬)
    ctx->BT();
    ctx->k(0, 0, 0, 1);
    ctx->Tf(font, fontSize);
    ctx->Tm(1, 0, 0, 1, itemX + padding, y + (height - fontSize) / 2 + 2);
    ctx->Tj(col2);
    ctx->ET();

    // 예산 (오른쪽 정렬)
    ctx->BT();
    ctx->k(0, 0, 0, 1);
    ctx->Tf(font, fontSize);
    double estimatedWidth3 = col3.length() * fontSize * 0.4;
    ctx->Tm(1, 0, 0, 1, itemX + col2Width + col3Width - estimatedWidth3 - padding, y + (height - fontSize) / 2 + 2);
    ctx->Tj(col3);
    ctx->ET();

    // 실적 (오른쪽 정렬)
    ctx->BT();
    ctx->k(0, 0, 0, 1);
    ctx->Tf(font, fontSize);
    double estimatedWidth4 = col4.length() * fontSize * 0.4;
    ctx->Tm(1, 0, 0, 1, x + width - estimatedWidth4 - padding, y + (height - fontSize) / 2 + 2);
    ctx->Tj(col4);
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
        const double pageWidth = 595.0;
        const double bottomMargin = 80.0; // 하단 여백

        // 페이지 넘김 체크 람다 함수
        auto checkAndCreateNewPage = [&]() {
            if (yPos < bottomMargin) {
                pdfWriter.EndPageContentContext(contentContext);
                pdfWriter.WritePageAndRelease(page);

                page = new PDFPage();
                page->SetMediaBox(PDFRectangle(0, 0, 595, 842));
                contentContext = pdfWriter.StartPageContentContext(page);
                yPos = 800.0;
            }
        };

        // === 제목 (중앙 정렬 + 자동 크기 조절) ===
        double titleFontSize = 20.0;

        // UTF-8 문자 개수 계산 (한글=3바이트, 영문=1바이트)
        size_t charCount = 0;
        for (size_t i = 0; i < data.Title.length(); )
        {
            unsigned char c = static_cast<unsigned char>(data.Title[i]);
            if (c < 0x80) { i += 1; charCount++; }       // ASCII (1바이트)
            else if (c < 0xE0) { i += 2; charCount++; }  // 2바이트 문자
            else if (c < 0xF0) { i += 3; charCount++; }  // 3바이트 문자 (한글 등)
            else { i += 4; charCount++; }                // 4바이트 문자
        }

        // 제목 너비 추정 (한글 기준 fontSize * 0.7 정도)
        double titleWidth = charCount * titleFontSize * 0.7;
        const double maxTitleWidth = pageWidth - 100.0;  // 좌우 여백 50씩

        // 제목이 너무 길면 폰트 크기 축소
        if (titleWidth > maxTitleWidth)
        {
            titleFontSize = titleFontSize * maxTitleWidth / titleWidth;
            titleWidth = charCount * titleFontSize * 0.7;
        }

        // 중앙 정렬 위치 계산
        const double titleX = (pageWidth - titleWidth) / 2.0;

        contentContext->BT();
        contentContext->k(0, 0, 0, 1); // Black
        contentContext->Tf(fontBold, titleFontSize);
        contentContext->Tm(1, 0, 0, 1, titleX, yPos);
        contentContext->Tj(data.Title);
        contentContext->ET();

        yPos -= 40.0;

        // === 1. 요약 - 총 수입 / 총 지출 (예산) ===
        checkAndCreateNewPage();
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                     "총 수입 (예산)", formatAmount(data.TotalIncome) + "원", fontBold, 10, false);
        yPos -= rowHeight;

        checkAndCreateNewPage();
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                     "총 지출 (예산)", formatAmount(data.TotalExpense) + "원", fontBold, 10, false);
        yPos -= rowHeight + sectionSpacing;

        // === 1. 요약 - 총 실수입 / 총 실지출 / 잔액 (실적) ===
        checkAndCreateNewPage();
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                     "총 실수입", formatAmount(data.TotalActualIncome) + "원", fontBold, 10, false);
        yPos -= rowHeight;

        checkAndCreateNewPage();
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                     "총 실지출", formatAmount(data.TotalActualExpense) + "원", fontBold, 10, false);
        yPos -= rowHeight;

        checkAndCreateNewPage();
        drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                     "잔액", formatAmount(data.TotalBalance) + "원", fontBold, 10, false);
        yPos -= rowHeight + sectionSpacing;

        // === 2. 환율 정보 ===
        if (!data.ExchangeRates.empty())
        {
            checkAndCreateNewPage();
            contentContext->BT();
            contentContext->Tf(fontBold, 14);
            contentContext->Tm(1, 0, 0, 1, leftMargin, yPos);
            contentContext->Tj("환율 정보");
            contentContext->ET();
            yPos -= 30.0;

            // 헤더 행
            checkAndCreateNewPage();
            drawTableRow(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                         "통화", "평균 환율", fontBold, 11, true);
            yPos -= headerHeight;

            for (const auto& rate : data.ExchangeRates)
            {
                checkAndCreateNewPage();
                std::stringstream ss;
                ss << std::fixed << std::setprecision(2) << rate.second << "원";
                drawTableRow(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                             rate.first, ss.str(), fontRegular, 10, false);
                yPos -= rowHeight;
            }

            yPos -= sectionSpacing;
        }

        // === 3. 수입 비교 (예산 vs 실적) ===
        checkAndCreateNewPage();
        contentContext->BT();
        contentContext->Tf(fontBold, 14);
        contentContext->Tm(1, 0, 0, 1, leftMargin, yPos);
        contentContext->Tj("수입 비교");
        contentContext->ET();
        yPos -= 30.0;

        // 헤더 행 (3열: 카테고리, 예산, 실적)
        checkAndCreateNewPage();
        drawTableRow3Col(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                        "카테고리", "예산", "실적", fontBold, 11, true);
        yPos -= headerHeight;

        // 수입 데이터 - 모든 카테고리 수집 (예산 + 실적)
        std::set<std::string> incomeCategories;
        for (const auto& cat : data.BudgetIncomeCategories) {
            incomeCategories.insert(cat.first);
        }
        for (const auto& cat : data.ActualIncomeCategories) {
            incomeCategories.insert(cat.first);
        }

        // 카테고리별 데이터 출력
        for (const auto& category : incomeCategories)
        {
            checkAndCreateNewPage();
            int64_t budgetAmount = 0;
            int64_t actualAmount = 0;

            auto budgetIt = data.BudgetIncomeCategories.find(category);
            if (budgetIt != data.BudgetIncomeCategories.end()) {
                budgetAmount = budgetIt->second;
            }

            auto actualIt = data.ActualIncomeCategories.find(category);
            if (actualIt != data.ActualIncomeCategories.end()) {
                actualAmount = actualIt->second;
            }

            drawTableRow3Col(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                           category,
                           formatAmount(budgetAmount) + "원",
                           formatAmount(actualAmount) + "원",
                           fontRegular, 10, false);
            yPos -= rowHeight;
        }

        // 합계 행
        checkAndCreateNewPage();
        drawTableRow3Col(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                        "합계",
                        formatAmount(data.TotalIncome) + "원",
                        formatAmount(data.TotalActualIncome) + "원",
                        fontBold, 9, true);
        yPos -= headerHeight + sectionSpacing;

        // === 4. 지출 비교 (예산 vs 실적) ===
        checkAndCreateNewPage();
        contentContext->BT();
        contentContext->Tf(fontBold, 14);
        contentContext->Tm(1, 0, 0, 1, leftMargin, yPos);
        contentContext->Tj("지출 비교");
        contentContext->ET();
        yPos -= 30.0;

        // 헤더 행 (3열: 카테고리, 예산, 실적)
        checkAndCreateNewPage();
        drawTableRow3Col(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                        "카테고리", "예산", "실적", fontBold, 11, true);
        yPos -= headerHeight;

        // 지출 데이터 - 카테고리별 합계 계산
        std::map<std::string, int64_t> budgetExpenseCategories;
        std::map<std::string, int64_t> actualExpenseCategories;

        // 예산 지출 카테고리별 합계
        for (const auto& category : data.BudgetExpenseItems) {
            int64_t categoryTotal = 0;
            for (const auto& item : category.second) {
                categoryTotal += item.second;
            }
            budgetExpenseCategories[category.first] = categoryTotal;
        }

        // 실적 지출 카테고리별 합계
        for (const auto& category : data.ActualExpenseItems) {
            int64_t categoryTotal = 0;
            for (const auto& item : category.second) {
                categoryTotal += item.second;
            }
            actualExpenseCategories[category.first] = categoryTotal;
        }

        // 모든 카테고리 수집
        std::set<std::string> allExpenseCategories;
        for (const auto& cat : budgetExpenseCategories) {
            allExpenseCategories.insert(cat.first);
        }
        for (const auto& cat : actualExpenseCategories) {
            allExpenseCategories.insert(cat.first);
        }

        // 카테고리별 데이터 출력
        for (const auto& category : allExpenseCategories)
        {
            checkAndCreateNewPage();
            int64_t budgetAmount = 0;
            int64_t actualAmount = 0;

            auto budgetIt = budgetExpenseCategories.find(category);
            if (budgetIt != budgetExpenseCategories.end()) {
                budgetAmount = budgetIt->second;
            }

            auto actualIt = actualExpenseCategories.find(category);
            if (actualIt != actualExpenseCategories.end()) {
                actualAmount = actualIt->second;
            }

            drawTableRow3Col(contentContext, leftMargin, yPos, tableWidth, rowHeight,
                           category,
                           formatAmount(budgetAmount) + "원",
                           formatAmount(actualAmount) + "원",
                           fontRegular, 10, false);
            yPos -= rowHeight;
        }

        // 합계 행
        checkAndCreateNewPage();
        drawTableRow3Col(contentContext, leftMargin, yPos, tableWidth, headerHeight,
                        "합계",
                        formatAmount(data.TotalExpense) + "원",
                        formatAmount(data.TotalActualExpense) + "원",
                        fontBold, 9, true);
        yPos -= headerHeight;

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
                // 환율 정보 추가 (한 줄로)
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
            ctx->Tf(font, fontSize);
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
        const double colWidths[6] = {50, 60, 170, 135, 60, 50};  // 구분, 항목, 내용, 금액, 년월일, 영수증번호
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
