#pragma once

// PDF 내보내기 레이아웃 값 (결산 보고서, 거래 내역서 공통)
// 모양 조절(여백, 글자 크기, 행 높이, 간격, 열 폭)은 이 파일에서만 한다
// 단위: pt (1/72 inch). PDF 좌표는 아래가 0
namespace MLPDFLayout
{
    // === 페이지 (A4) ===
    constexpr double PageWidth    = 595.0;
    constexpr double PageHeight   = 842.0;
    constexpr double MarginTop    = 42.0;
    constexpr double MarginBottom = 40.0;
    constexpr double MarginX      = 35.0;                      // 왼쪽 = 오른쪽
    constexpr double TableWidth   = PageWidth - MarginX * 2;   // 모든 표 공통 폭 (525)

    // === 글자 크기 ===
    constexpr double FontTitle      = 18.0;   // 문서 제목 ("결산 보고서", "거래 내역서")
    constexpr double FontSubtitle   = 10.5;   // 부제 (.ml 파일 이름)
    constexpr double FontSection    = 13.0;   // 섹션 제목 ("수입 비교", "지출" 등)
    constexpr double FontSubSection = 10.5;   // 작은 섹션 제목 ("수입 카테고리별" 등)
    constexpr double FontTable      = 9.0;    // 표 본문과 헤더
    constexpr double FontNote       = 7.5;    // 금액 아래 환율 정보

    // === 색 (검정 농도: 0 = 흰색, 1 = 검정) ===
    constexpr double InkText      = 1.0;
    constexpr double InkSubtitle  = 0.6;
    constexpr double InkNote      = 0.6;
    constexpr double FillHeader   = 0.1;      // 헤더/합계 행 배경
    constexpr double LineWidth    = 0.5;      // 표 선 굵기

    // === 표 ===
    constexpr double RowHeight    = 20.0;     // 한 줄 행 (여러 줄이면 늘어남)
    constexpr double HeaderHeight = 22.0;     // 헤더/합계 행
    constexpr double CellPadX     = 5.0;      // 셀 안쪽 좌우 여백 (왼쪽/오른쪽 정렬 공통)
    constexpr double CellPadY     = 4.0;      // 여러 줄 셀의 위아래 여백
    constexpr double LineGap      = 3.0;      // 여러 줄 텍스트의 줄 사이

    // === 글자 세로 위치 (글자 크기에 대한 비율) ===
    constexpr double TextMiddle   = 0.36;     // 기준선 → 글자 눈높이 가운데 (셀 세로 가운데 정렬)
    constexpr double TextAscent   = 0.80;     // 기준선 → 글자 윗부분 (제목을 위쪽에 맞출 때)

    // === 간격 ===
    constexpr double TitleGap        = 6.0;   // 제목 → 부제
    constexpr double HeaderGap       = 20.0;  // 제목(부제) → 첫 내용
    constexpr double SectionTitleGap = 7.0;   // 섹션 제목 → 표
    constexpr double SectionGap      = 20.0;  // 표 → 다음 섹션 제목
    constexpr double TableGap        = 10.0;  // 같은 섹션 안 표 사이
    constexpr double ChunkGap        = 3.0;   // 줄이 넘어간 카테고리 합계 표 사이

    // === 결산 보고서 열 비율 (합 = 1) ===
    constexpr double SettlementLabelRatio      = 0.6;                   // 2열 표: 항목 | 금액
    constexpr double SettlementCompareRatio[3] = { 0.5, 0.25, 0.25 };  // 카테고리 | 예산 | 실적

    // === 거래 내역서 ===
    // 열 폭: 구분 | 항목 | 내용 | 금액 | 년월일 | 영수증번호 (합 = TableWidth)
    constexpr double TransactionCols[6] = { 55.0, 70.0, 170.0, 108.0, 60.0, 62.0 };
    constexpr double SummaryLabelRatio   = 0.6;   // 합계 표: 항목 | 금액
    constexpr double CategoryMinColWidth = 75.0;  // 카테고리별 합계 표의 최소 열 폭 (넘으면 다음 줄)
}
