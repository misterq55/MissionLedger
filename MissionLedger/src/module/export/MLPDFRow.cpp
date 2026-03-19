#include "MLPDFRow.h"
#include "PDFWriter/PageContentContext.h"

FMLPDFRow::FMLPDFRow(double height, bool isHeader)
    : Height(height), IsHeader(isHeader)
{
}

FMLPDFRow& FMLPDFRow::AddCell(const std::string& text, double width, E_MLPDFAlign align)
{
    Cells.push_back({text, width, align});
    return *this;
}

double FMLPDFRow::GetTotalWidth() const
{
    double total = 0;
    for (const auto& cell : Cells)
        total += cell.Width;
    return total;
}

void FMLPDFRow::Draw(PageContentContext* ctx, double x, double y, PDFUsedFont* font, double fontSize) const
{
    const double totalWidth = GetTotalWidth();
    const double padding = 10.0;

    // 헤더 배경 (회색)
    if (IsHeader) {
        ctx->q();
        ctx->k(0, 0, 0, 0.1);
        ctx->re(x, y, totalWidth, Height);
        ctx->f();
        ctx->Q();
    }

    // 전체 테두리 + 세로 구분선
    ctx->q();
    ctx->w(0.5);
    ctx->k(0, 0, 0, 1);
    ctx->re(x, y, totalWidth, Height);
    ctx->S();

    double currentX = x;
    for (size_t i = 0; i + 1 < Cells.size(); i++) {
        currentX += Cells[i].Width;
        ctx->m(currentX, y);
        ctx->l(currentX, y + Height);
        ctx->S();
    }
    ctx->Q();

    // 텍스트 출력
    currentX = x;
    for (const auto& cell : Cells) {
        double textX;
        if (cell.Align == E_MLPDFAlign::Right) {
            double estimatedWidth = cell.Text.length() * fontSize * 0.55;
            textX = currentX + cell.Width - estimatedWidth - padding;
        } else if (cell.Align == E_MLPDFAlign::Center) {
            double estimatedWidth = cell.Text.length() * fontSize * 0.55;
            textX = currentX + (cell.Width - estimatedWidth) / 2.0;
        } else {
            textX = currentX + padding;
        }

        ctx->BT();
        ctx->k(0, 0, 0, 1);
        ctx->Tf(font, fontSize);
        ctx->Tm(1, 0, 0, 1, textX, y + (Height - fontSize) / 2 + 2);
        ctx->Tj(cell.Text);
        ctx->ET();

        currentX += cell.Width;
    }
}
