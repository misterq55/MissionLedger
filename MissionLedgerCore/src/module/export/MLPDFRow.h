#pragma once
#include <string>
#include <vector>
#include "MLPDFDocument.h"

struct FMLPDFCell {
    std::string Text;
    double Width;
    E_MLPDFAlign Align;
    bool Bold;
};

// 표 한 행 (한 줄 텍스트 셀들, 높이 고정)
class FMLPDFRow {
public:
    FMLPDFRow(double height, bool isHeader = false);

    FMLPDFRow& AddCell(const std::string& text, double width, E_MLPDFAlign align = E_MLPDFAlign::Left, bool bold = false);

    // top: 행의 위쪽 Y. 행 전체를 굵게 하려면 isBold, 셀 하나만이면 AddCell의 bold
    void Draw(FMLPDFDocument& doc, double x, double top, double fontSize, bool isBold) const;

    double GetHeight() const { return Height; }
    double GetTotalWidth() const;

private:
    std::vector<FMLPDFCell> Cells;
    double Height;
    bool IsHeader;
};
