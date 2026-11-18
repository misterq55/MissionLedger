#pragma once
#include <string>
#include <vector>

class PageContentContext;
class PDFUsedFont;

enum class E_MLPDFAlign { Left, Right, Center };

struct FMLPDFCell {
    std::string Text;
    double Width;
    E_MLPDFAlign Align;
};

class FMLPDFRow {
public:
    FMLPDFRow(double height, bool isHeader = false);

    FMLPDFRow& AddCell(const std::string& text, double width, E_MLPDFAlign align = E_MLPDFAlign::Left);
    void Draw(PageContentContext* ctx, double x, double y, PDFUsedFont* font, double fontSize) const;

    double GetHeight() const { return Height; }
    double GetTotalWidth() const;

private:
    std::vector<FMLPDFCell> Cells;
    double Height;
    bool IsHeader;
};
