#pragma once
#include <string>
#include <vector>
#include "MLDefine.h"

class FMLPDFExporter {
public:
    // 결산 보고서 PDF 생성 (제목 data.Title, 부제 data.Subtitle)
    static bool ExportSettlement(
        const FMLSettlmentData& data,
        const std::string& filePath);

    // 거래 내역서 PDF 생성 (부제: 보통 .ml 파일 이름, 비어 있으면 생략)
    static bool ExportTransactionList(
        const std::vector<FMLTransactionData>& transactions,
        const std::string& subtitle,
        const std::string& filePath);

private:
    // 공통 헬퍼 메서드
    static void drawHeader(void* pdfWriter, const std::string& title);
    static void drawFooter(void* pdfWriter);
};
