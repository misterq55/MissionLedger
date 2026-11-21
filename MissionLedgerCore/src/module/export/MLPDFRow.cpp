#include "MLPDFRow.h"
#include "MLPDFLayout.h"

FMLPDFRow::FMLPDFRow(double height, bool isHeader)
    : Height(height), IsHeader(isHeader)
{
}

FMLPDFRow& FMLPDFRow::AddCell(const std::string& text, double width, E_MLPDFAlign align, bool bold)
{
    Cells.push_back({text, width, align, bold});
    return *this;
}

double FMLPDFRow::GetTotalWidth() const
{
    double total = 0;
    for (const auto& cell : Cells)
        total += cell.Width;
    return total;
}

void FMLPDFRow::Draw(FMLPDFDocument& doc, double x, double top, double fontSize, bool isBold) const
{
    const double bottom = top - Height;

    // 헤더/합계 행 배경 (회색)
    if (IsHeader)
        doc.FillRect(x, bottom, GetTotalWidth(), Height, MLPDFLayout::FillHeader);

    // 셀 테두리 + 텍스트
    double cellX = x;
    for (const auto& cell : Cells) {
        doc.StrokeRect(cellX, bottom, cell.Width, Height);
        doc.DrawCellText({cell.Text}, cellX, bottom, cell.Width, Height, cell.Align, isBold || cell.Bold, fontSize);
        cellX += cell.Width;
    }
}
