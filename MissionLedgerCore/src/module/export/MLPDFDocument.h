#pragma once
#include <string>

class PDFWriter;
class PDFPage;
class PageContentContext;
class PDFUsedFont;
class FMLPDFRow;

// PDF 문서 상태(PDFWriter, 페이지, 컨텍스트, 폰트, Y위치)를 캡슐화하는 클래스
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

    // Y위치가 하단 여백 이하면 NewPage() 호출, 전환 발생 시 true 반환
    bool CheckPageBreak();

    // 행을 현재 Y위치에 그리고 자동으로 MoveDown 처리
    void DrawRow(const FMLPDFRow& row, double x, double fontSize, bool isBold = false);

    PageContentContext* GetContext() const;
    PDFUsedFont* GetFontBold() const;
    PDFUsedFont* GetFontRegular() const;
    double GetY() const;
    double GetPageWidth() const;
    void MoveDown(double amount);
    void SetY(double y);

private:
    bool loadFonts();

    PDFWriter* Writer;
    PDFPage* Page;
    PageContentContext* Context;
    PDFUsedFont* FontBold;
    PDFUsedFont* FontRegular;
    double YPos;
    bool IsOpen;
};
