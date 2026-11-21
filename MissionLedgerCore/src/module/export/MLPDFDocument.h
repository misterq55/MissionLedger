#pragma once
#include <string>
#include <vector>

class PDFWriter;
class PDFPage;
class PageContentContext;
class PDFUsedFont;
class FMLPDFRow;

enum class E_MLPDFAlign { Left, Right, Center };

// 셀 안의 한 줄 (줄마다 크기/굵기/농도가 다를 수 있음: 예) 금액 + 작은 환율 정보)
struct FMLPDFTextLine {
    std::string Text;
    double Size;
    bool Bold = false;
    double Ink = 1.0;
};

// PDF 문서 상태(PDFWriter, 페이지, 컨텍스트, 폰트, 쓰기 위치)와 그리기 도구를 캡슐화하는 클래스
//
// 쓰기 위치(Y)는 "다음에 그릴 것의 위쪽"이다. 그리는 함수는 Y에서 아래로 그리고 그만큼 Y를 내린다
// 레이아웃 값은 MLPDFLayout.h에 있다
class FMLPDFDocument {
public:
    FMLPDFDocument();
    ~FMLPDFDocument();

    // PDF 파일 열기 (페이지 생성 + 폰트 로드 포함)
    bool Open(const std::string& filePath);

    // PDF 파일 닫기 (마지막 페이지 저장 + EndPDF)
    void Close();

    // 새 페이지로 전환 (현재 페이지 저장, Y위치 초기화)
    void NewPage();

    // 현재 Y에서 height만큼 그릴 자리가 없으면 새 페이지. 넘어갔으면 true
    bool EnsureSpace(double height);

    double GetY() const;
    void MoveDown(double amount);

    // === 글자 ===
    // 실제 글꼴 폭 (pt)
    double TextWidth(const std::string& text, bool bold, double size) const;

    // maxWidth 안에 들어가도록 줄바꿈 (띄어쓰기 우선, 줄 앞 공백 제거, '\n' 존중)
    std::vector<std::string> WrapText(const std::string& text, double maxWidth, bool bold, double size) const;

    // 여러 줄의 전체 높이 (줄 사이 LineGap 포함)
    static double LinesHeight(size_t lineCount, double size);

    // 기준선 위치에 한 줄 출력
    void DrawTextAt(const std::string& text, double x, double baseline, bool bold, double size, double ink = 1.0);

    // === 도형 ===
    void StrokeRect(double x, double bottom, double width, double height);
    void FillRect(double x, double bottom, double width, double height, double ink);

    // 사각형 안에 여러 줄을 가로 정렬 + 세로 가운데로 출력
    void DrawCellText(const std::vector<FMLPDFTextLine>& lines, double x, double bottom, double width, double height, E_MLPDFAlign align);
    void DrawCellText(const std::vector<std::string>& lines, double x, double bottom, double width, double height, E_MLPDFAlign align, bool bold, double size);

    // === 블록 (Y에서 아래로 그리고 Y를 내림) ===
    // 문서 제목 + 부제 (가운데 정렬, 부제가 비어 있으면 생략)
    void DrawTitle(const std::string& title, const std::string& subtitle);

    // 섹션 제목 (왼쪽 정렬)
    void DrawSectionTitle(const std::string& text, double size);

    // 표 한 행
    void DrawRow(const FMLPDFRow& row, double x, double fontSize, bool isBold = false);

private:
    bool loadFonts();
    PDFUsedFont* font(bool bold) const;

    PDFWriter* Writer;
    PDFPage* Page;
    PageContentContext* Context;
    PDFUsedFont* FontBold;
    PDFUsedFont* FontRegular;
    double YPos;
    bool IsOpen;
};
