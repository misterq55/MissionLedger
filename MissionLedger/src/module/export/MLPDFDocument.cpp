#include "MLPDFDocument.h"
#include "MLPDFRow.h"
#include "PDFWriter/PDFWriter.h"
#include "PDFWriter/PDFPage.h"
#include "PDFWriter/PageContentContext.h"

#include <vector>
#include <string>

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
#else
    #include <unistd.h>
    #include <limits.h>
    #include <sys/stat.h>
#endif

namespace {
    constexpr double kPageWidth    = 595.0;
    constexpr double kPageHeight   = 842.0;
    constexpr double kInitialY     = 800.0;
    constexpr double kBottomMargin = 80.0;

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
    // 1. 실행 파일 근처 fonts/ 디렉토리 (배포 폰트)
    // 2. 시스템 폰트 디렉토리 (플랫폼별)
    static std::string findFontFile(const std::string& fontName, bool searchKorean)
    {
        std::vector<std::string> searchPaths;

        // 1. 배포 폰트 검색
        std::string exeDir = getExecutablePath();
        if (!exeDir.empty()) {
    #ifdef _WIN32
            std::string bundledFontPath = exeDir + "\\fonts\\" + fontName;
    #else
            std::string bundledFontPath = exeDir + "/fonts/" + fontName;
    #endif
            if (fileExists(bundledFontPath))
                return bundledFontPath;
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
            koreanFont = findFontFile("NotoSansKR-Bold.ttf", true);
            if (koreanFont.empty())
                koreanFont = findFontFile("malgunbd.ttf", true);
        } else {
            koreanFont = findFontFile("NotoSansKR-Regular.ttf", true);
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
    Page->SetMediaBox(PDFRectangle(0, 0, kPageWidth, kPageHeight));
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
    Page->SetMediaBox(PDFRectangle(0, 0, kPageWidth, kPageHeight));
    Context = Writer->StartPageContentContext(Page);
    YPos = kInitialY;
}

bool FMLPDFDocument::CheckPageBreak()
{
    if (YPos < kBottomMargin) {
        NewPage();
        return true;
    }
    return false;
}

void FMLPDFDocument::DrawRow(const FMLPDFRow& row, double x, double fontSize, bool isBold)
{
    row.Draw(Context, x, YPos, isBold ? FontBold : FontRegular, fontSize);
    YPos -= row.GetHeight();
}

PageContentContext* FMLPDFDocument::GetContext() const { return Context; }
PDFUsedFont* FMLPDFDocument::GetFontBold() const      { return FontBold; }
PDFUsedFont* FMLPDFDocument::GetFontRegular() const   { return FontRegular; }
double FMLPDFDocument::GetY() const                   { return YPos; }
double FMLPDFDocument::GetPageWidth() const           { return kPageWidth; }
void FMLPDFDocument::MoveDown(double amount)          { YPos -= amount; }
void FMLPDFDocument::SetY(double y)                   { YPos = y; }

bool FMLPDFDocument::loadFonts()
{
    FontBold    = loadFontHelper(*Writer, true);
    FontRegular = loadFontHelper(*Writer, false);
    return FontBold != nullptr && FontRegular != nullptr;
}
