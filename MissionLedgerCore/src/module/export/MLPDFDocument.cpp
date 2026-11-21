#include "MLPDFDocument.h"
#include "MLPDFRow.h"
#include "MLPDFLayout.h"
#include "PDFWriter/PDFWriter.h"
#include "PDFWriter/PDFPage.h"
#include "PDFWriter/PageContentContext.h"
#include "PDFWriter/PDFUsedFont.h"

#include <vector>
#include <string>
#include <algorithm>

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
#else
    #include <unistd.h>
    #include <limits.h>
    #include <sys/stat.h>
    #include <cstdlib>
    #ifdef __APPLE__
        #include <mach-o/dyld.h>    // _NSGetExecutablePath
        #include <cstdint>
    #endif
#endif

namespace {
    using namespace MLPDFLayout;

    constexpr double kInitialY = PageHeight - MarginTop;

    // UTF-8 한 글자의 바이트 수
    static size_t utf8CharLength(unsigned char c)
    {
        if ((c & 0x80) == 0)    return 1;
        if ((c & 0xE0) == 0xC0) return 2;
        if ((c & 0xF0) == 0xE0) return 3;
        if ((c & 0xF8) == 0xF0) return 4;
        return 1;
    }

    static std::string trimLeadingSpaces(const std::string& s)
    {
        const size_t first = s.find_first_not_of(' ');
        return first == std::string::npos ? std::string() : s.substr(first);
    }

    static std::string trimTrailingSpaces(const std::string& s)
    {
        const size_t last = s.find_last_not_of(' ');
        return last == std::string::npos ? std::string() : s.substr(0, last + 1);
    }

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

    static std::string getExecutablePath()
    {
    #ifdef _WIN32
        char buffer[MAX_PATH];
        DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);
        if (length > 0 && length < MAX_PATH) {
            std::string fullPath(buffer, length);
            size_t lastSlash = fullPath.find_last_of("\\/");
            if (lastSlash != std::string::npos)
                return fullPath.substr(0, lastSlash);
        }
        return "";
    #elif defined(__APPLE__)
        // macOS has no /proc: ask dyld, then resolve symlinks and "..".
        // A first call with a too-small buffer reports the size needed
        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);
        std::string rawPath(size, '\0');
        if (size == 0 || _NSGetExecutablePath(&rawPath[0], &size) != 0)
            return "";
        char resolved[PATH_MAX];
        if (realpath(rawPath.c_str(), resolved) == nullptr)
            return "";
        const std::string fullPath(resolved);
        const size_t lastSlash = fullPath.find_last_of('/');
        return lastSlash != std::string::npos ? fullPath.substr(0, lastSlash) : "";
    #else
        char buffer[PATH_MAX];
        ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (length > 0) {
            buffer[length] = '\0';
            std::string fullPath(buffer);
            size_t lastSlash = fullPath.find_last_of('/');
            if (lastSlash != std::string::npos)
                return fullPath.substr(0, lastSlash);
        }
        return "";
    #endif
    }

    // 검색 우선순위:
    // 1. 배포 폰트: 실행 파일 옆 fonts/ (Windows, CLI), macOS 앱 번들의 Contents/Resources/fonts/
    //    (원본: third_party/fonts/NotoSansKR, 빌드와 설치 프로그램이 복사)
    // 2. 시스템 폰트 디렉토리 (플랫폼별)
    static std::string findFontFile(const std::string& fontName, bool searchKorean)
    {
        std::vector<std::string> searchPaths;

        // 1. 배포 폰트 검색
        std::string exeDir = getExecutablePath();
        if (!exeDir.empty()) {
    #ifdef _WIN32
            const std::vector<std::string> bundledDirs = { exeDir + "\\fonts\\" };
    #elif defined(__APPLE__)
            // exeDir is Contents/MacOS inside an app bundle
            const std::vector<std::string> bundledDirs = { exeDir + "/fonts/", exeDir + "/../Resources/fonts/" };
    #else
            const std::vector<std::string> bundledDirs = { exeDir + "/fonts/" };
    #endif
            for (const auto& dir : bundledDirs) {
                if (fileExists(dir + fontName))
                    return dir + fontName;
            }
        }

        // 2. 시스템 폰트 디렉토리 검색
        if (searchKorean) {
    #ifdef _WIN32
            char winDir[MAX_PATH];
            DWORD result = GetEnvironmentVariableA("WINDIR", winDir, MAX_PATH);
            if (result > 0 && result < MAX_PATH)
                searchPaths.push_back(std::string(winDir) + "\\Fonts\\" + fontName);
    #elif defined(__APPLE__)
            searchPaths.push_back("/Library/Fonts/AppleSDGothicNeo.ttc");
            searchPaths.push_back("/System/Library/Fonts/AppleSDGothicNeo.ttc");
            searchPaths.push_back("/Library/Fonts/NotoSansCJK-Regular.ttc");
            searchPaths.push_back("/System/Library/Fonts/NotoSansCJK-Regular.ttc");
    #else
            searchPaths.push_back("/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc");
            searchPaths.push_back("/usr/share/fonts/truetype/noto/NotoSansKR-Regular.ttf");
            searchPaths.push_back("/usr/share/fonts/truetype/nanum/NanumGothic.ttf");
    #endif
        } else {
    #ifdef _WIN32
            char winDir[MAX_PATH];
            DWORD result = GetEnvironmentVariableA("WINDIR", winDir, MAX_PATH);
            if (result > 0 && result < MAX_PATH)
                searchPaths.push_back(std::string(winDir) + "\\Fonts\\" + fontName);
    #elif defined(__APPLE__)
            searchPaths.push_back("/Library/Fonts/Arial.ttf");
            searchPaths.push_back("/System/Library/Fonts/Supplemental/Arial.ttf");
    #else
            searchPaths.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
            searchPaths.push_back("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
    #endif
        }

        for (const auto& path : searchPaths) {
            if (fileExists(path))
                return path;
        }
        return "";
    }

    static PDFUsedFont* loadFontHelper(PDFWriter& pdfWriter, bool bold)
    {
        PDFUsedFont* font = nullptr;

        // 1단계: 한글 폰트 시도
        std::string koreanFont;
        if (bold) {
            koreanFont = findFontFile("NotoSansKR-Bold.otf", true);
            if (koreanFont.empty())
                koreanFont = findFontFile("malgunbd.ttf", true);
        } else {
            koreanFont = findFontFile("NotoSansKR-Regular.otf", true);
            if (koreanFont.empty())
                koreanFont = findFontFile("malgun.ttf", true);
        }

        if (!koreanFont.empty()) {
            try {
                font = pdfWriter.GetFontForFile(koreanFont);
                if (font != nullptr) return font;
            } catch (...) {}
        }

        // 2단계: 영문 폰트 시도
        std::string englishFont;
        if (bold)
            englishFont = findFontFile("arialbd.ttf", false);
        else
            englishFont = findFontFile("arial.ttf", false);

        if (!englishFont.empty()) {
            try {
                font = pdfWriter.GetFontForFile(englishFont);
                if (font != nullptr) return font;
            } catch (...) {}
        }

        // 3단계: 플랫폼별 최후 폴백
    #ifdef _WIN32
        const char* lastResortFont = bold ? "C:\\Windows\\Fonts\\arialbd.ttf"
                                          : "C:\\Windows\\Fonts\\arial.ttf";
    #elif defined(__APPLE__)
        const char* lastResortFont = "/System/Library/Fonts/Helvetica.ttc";
    #else
        const char* lastResortFont = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
    #endif

        try { font = pdfWriter.GetFontForFile(lastResortFont); } catch (...) {}
        return font;
    }
} // namespace


FMLPDFDocument::FMLPDFDocument()
    : Writer(nullptr), Page(nullptr), Context(nullptr),
      FontBold(nullptr), FontRegular(nullptr), YPos(kInitialY), IsOpen(false)
{
}

FMLPDFDocument::~FMLPDFDocument()
{
    if (IsOpen) Close();
    delete Writer;
}

bool FMLPDFDocument::Open(const std::string& filePath)
{
    Writer = new PDFWriter();
    Writer->StartPDF(filePath, ePDFVersion13);

    if (!loadFonts()) {
        delete Writer;
        Writer = nullptr;
        return false;
    }

    Page = new PDFPage();
    Page->SetMediaBox(PDFRectangle(0, 0, PageWidth, PageHeight));
    Context = Writer->StartPageContentContext(Page);
    YPos = kInitialY;
    IsOpen = true;
    return true;
}

void FMLPDFDocument::Close()
{
    if (!IsOpen) return;
    Writer->EndPageContentContext(Context);
    Writer->WritePageAndRelease(Page);
    Writer->EndPDF();
    Page = nullptr;
    Context = nullptr;
    IsOpen = false;
}

void FMLPDFDocument::NewPage()
{
    Writer->EndPageContentContext(Context);
    Writer->WritePageAndRelease(Page);

    Page = new PDFPage();
    Page->SetMediaBox(PDFRectangle(0, 0, PageWidth, PageHeight));
    Context = Writer->StartPageContentContext(Page);
    YPos = kInitialY;
}

bool FMLPDFDocument::EnsureSpace(double height)
{
    // 새 페이지 맨 위에서도 안 들어가면 그대로 그린다 (무한 페이지 방지)
    if (YPos - height < MarginBottom && YPos < kInitialY) {
        NewPage();
        return true;
    }
    return false;
}

double FMLPDFDocument::GetY() const          { return YPos; }
void FMLPDFDocument::MoveDown(double amount) { YPos -= amount; }

// === 글자 ===

PDFUsedFont* FMLPDFDocument::font(bool bold) const
{
    return bold ? FontBold : FontRegular;
}

double FMLPDFDocument::TextWidth(const std::string& text, bool bold, double size) const
{
    if (text.empty()) return 0.0;
    return font(bold)->CalculateTextAdvance(text, size);
}

std::vector<std::string> FMLPDFDocument::WrapText(const std::string& text, double maxWidth, bool bold, double size) const
{
    std::vector<std::string> lines;

    // '\n'으로 나눈 문단마다 줄바꿈
    size_t paragraphStart = 0;
    while (true) {
        const size_t newline = text.find('\n', paragraphStart);
        std::string paragraph = text.substr(paragraphStart, newline == std::string::npos ? std::string::npos : newline - paragraphStart);
        if (!paragraph.empty() && paragraph.back() == '\r') paragraph.pop_back();

        std::string line;
        size_t i = 0;
        while (i < paragraph.size()) {
            const size_t charLength = std::min(utf8CharLength(static_cast<unsigned char>(paragraph[i])), paragraph.size() - i);
            const std::string next = line + paragraph.substr(i, charLength);

            if (line.empty() || TextWidth(next, bold, size) <= maxWidth) {
                line = next;
                i += charLength;
                continue;
            }

            // 넘침: 줄 안의 마지막 띄어쓰기에서 자르고, 없으면 글자 단위로 자른다
            const size_t lastSpace = line.find_last_of(' ');
            if (lastSpace != std::string::npos && lastSpace > 0 && paragraph[i] != ' ') {
                lines.push_back(trimTrailingSpaces(line.substr(0, lastSpace)));
                line = trimLeadingSpaces(line.substr(lastSpace + 1));
            } else {
                lines.push_back(trimTrailingSpaces(line));
                line.clear();
                // 다음 줄은 공백으로 시작하지 않는다
                while (i < paragraph.size() && paragraph[i] == ' ') ++i;
            }
        }
        lines.push_back(trimTrailingSpaces(line));

        if (newline == std::string::npos) break;
        paragraphStart = newline + 1;
    }

    return lines;
}

double FMLPDFDocument::LinesHeight(size_t lineCount, double size)
{
    if (lineCount == 0) return 0.0;
    return lineCount * size + (lineCount - 1) * LineGap;
}

void FMLPDFDocument::DrawTextAt(const std::string& text, double x, double baseline, bool bold, double size, double ink)
{
    if (text.empty()) return;
    Context->BT();
    Context->k(0, 0, 0, ink);
    Context->Tf(font(bold), size);
    Context->Tm(1, 0, 0, 1, x, baseline);
    Context->Tj(text);
    Context->ET();
}

// === 도형 ===

void FMLPDFDocument::StrokeRect(double x, double bottom, double width, double height)
{
    Context->q();
    Context->w(LineWidth);
    Context->K(0, 0, 0, 1);
    Context->re(x, bottom, width, height);
    Context->S();
    Context->Q();
}

void FMLPDFDocument::FillRect(double x, double bottom, double width, double height, double ink)
{
    Context->q();
    Context->k(0, 0, 0, ink);
    Context->re(x, bottom, width, height);
    Context->f();
    Context->Q();
}

void FMLPDFDocument::DrawCellText(const std::vector<FMLPDFTextLine>& lines, double x, double bottom, double width, double height, E_MLPDFAlign align)
{
    if (lines.empty()) return;

    // 줄 상자(높이 = 글자 크기)를 LineGap 간격으로 쌓은 블록을 셀 세로 가운데에 둔다
    double blockHeight = 0.0;
    for (const auto& line : lines) blockHeight += line.Size;
    blockHeight += (lines.size() - 1) * LineGap;

    double boxTop = bottom + height / 2.0 + blockHeight / 2.0;
    for (const auto& line : lines) {
        // 줄 상자 가운데 = 기준선 + TextMiddle × 크기
        const double baseline = boxTop - line.Size / 2.0 - TextMiddle * line.Size;

        double textX = x + CellPadX;
        if (align != E_MLPDFAlign::Left) {
            const double textWidth = TextWidth(line.Text, line.Bold, line.Size);
            textX = (align == E_MLPDFAlign::Right)
                ? x + width - CellPadX - textWidth
                : x + (width - textWidth) / 2.0;
        }
        DrawTextAt(line.Text, textX, baseline, line.Bold, line.Size, line.Ink);

        boxTop -= line.Size + LineGap;
    }
}

void FMLPDFDocument::DrawCellText(const std::vector<std::string>& lines, double x, double bottom, double width, double height, E_MLPDFAlign align, bool bold, double size)
{
    std::vector<FMLPDFTextLine> textLines;
    for (const auto& line : lines) textLines.push_back({line, size, bold, InkText});
    DrawCellText(textLines, x, bottom, width, height, align);
}

// === 블록 ===

void FMLPDFDocument::DrawTitle(const std::string& title, const std::string& subtitle)
{
    const double titleWidth = TextWidth(title, true, FontTitle);
    DrawTextAt(title, (PageWidth - titleWidth) / 2.0, YPos - FontTitle * TextAscent, true, FontTitle, InkText);
    YPos -= FontTitle;

    if (!subtitle.empty()) {
        // 긴 부제는 페이지 폭에 맞게 글자를 줄인다
        double size = FontSubtitle;
        const double maxWidth = TableWidth;
        double width = TextWidth(subtitle, false, size);
        if (width > maxWidth) {
            size *= maxWidth / width;
            width = TextWidth(subtitle, false, size);
        }
        YPos -= TitleGap;
        DrawTextAt(subtitle, (PageWidth - width) / 2.0, YPos - size * TextAscent, false, size, InkSubtitle);
        YPos -= size;
    }

    YPos -= HeaderGap;
}

void FMLPDFDocument::DrawSectionTitle(const std::string& text, double size)
{
    DrawTextAt(text, MarginX, YPos - size * TextAscent, true, size, InkText);
    YPos -= size + SectionTitleGap;
}

void FMLPDFDocument::DrawRow(const FMLPDFRow& row, double x, double fontSize, bool isBold)
{
    row.Draw(*this, x, YPos, fontSize, isBold);
    YPos -= row.GetHeight();
}

bool FMLPDFDocument::loadFonts()
{
    FontBold    = loadFontHelper(*Writer, true);
    FontRegular = loadFontHelper(*Writer, false);
    return FontBold != nullptr && FontRegular != nullptr;
}
