@echo off
chcp 65001 > nul
set CLI=x64\Release\MissionLedgerCLI.exe
set FILE=data.ml

echo ========================================
echo     MissionLedger 데이터 일괄 입력
echo ========================================
echo.

REM ========================================
REM 수입 데이터
REM ========================================
echo [1/3] 수입 데이터 입력 중...
echo.

REM 회비
echo   - 회비 입력 중...
%CLI% add --file %FILE% --type income --amount 800000 --category "회비" --item "박세영" --date 2024-11-14
%CLI% add --file %FILE% --type income --amount 800000 --category "회비" --item "서문규" --date 2024-11-14
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "진예주" --date 2024-11-15
%CLI% add --file %FILE% --type income --amount 800000 --category "회비" --item "이정수" --date 2024-11-21
%CLI% add --file %FILE% --type income --amount 1000000 --category "회비" --item "김영미" --date 2024-11-21
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "최주영" --date 2024-11-21
%CLI% add --file %FILE% --type income --amount 300000 --category "회비" --item "장세린" --date 2024-11-22
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "서찬민" --date 2024-11-23
%CLI% add --file %FILE% --type income --amount 300000 --category "회비" --item "장세린" --date 2024-11-25
%CLI% add --file %FILE% --type income --amount 1000000 --category "회비" --item "하종규" --date 2024-11-25
%CLI% add --file %FILE% --type income --amount 1000000 --category "회비" --item "박은경" --date 2024-11-25
%CLI% add --file %FILE% --type income --amount 800000 --category "회비" --item "신인재" --date 2024-11-25
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "이하연" --date 2024-11-25
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "조수아" --date 2024-11-25
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "박예은" --date 2024-11-25
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "박희원" --date 2024-11-26
%CLI% add --file %FILE% --type income --amount 800000 --category "회비" --item "박경린" --date 2024-11-26
%CLI% add --file %FILE% --type income --amount 1000000 --category "회비" --item "김대헌" --date 2024-11-27
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "김승혜" --date 2024-11-28
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "윤서현" --date 2024-11-28
%CLI% add --file %FILE% --type income --amount 500000 --category "회비" --item "김하은" --date 2024-11-28
%CLI% add --file %FILE% --type income --amount 200000 --category "회비" --item "장세린" --date 2024-11-29
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "김유니" --date 2024-12-05
%CLI% add --file %FILE% --type income --amount 400000 --category "회비" --item "박세철" --date 2024-12-12
%CLI% add --file %FILE% --type income --amount 600000 --category "회비" --item "이재혁" --date 2024-12-26

REM 후원
echo   - 후원 입력 중...
%CLI% add --file %FILE% --type income --amount 200000 --category "후원" --item "당회 지원" --date 2024-12-01
%CLI% add --file %FILE% --type income --amount 800000 --category "후원" --item "목적 헌금" --date 2024-11-26
%CLI% add --file %FILE% --type income --amount 900000 --category "후원" --item "목적 헌금" --date 2024-12-03
%CLI% add --file %FILE% --type income --amount 840000 --category "후원" --item "목적 헌금" --date 2024-12-10
%CLI% add --file %FILE% --type income --amount 1680000 --category "후원" --item "목적 헌금" --date 2024-12-24
%CLI% add --file %FILE% --type income --amount 2000000 --category "후원" --item "재정 지원"

REM 기타
echo   - 기타 입력 중...
%CLI% add --file %FILE% --type income --amount 1042 --category "기타" --item "결산이자" --date 2024-12-14

echo   ✓ 수입 데이터 입력 완료
echo.

REM ========================================
REM 지출 데이터 1
REM ========================================
echo [2/3] 지출 데이터 1 입력 중...
echo.

REM 항공
echo   - 항공 입력 중...
%CLI% add --file %FILE% --type expense --amount 13800000 --category "항공" --item "항공료" --desc "선결제(300만원) 제외한 비행기 티켓 금액" --date 2024-12-13
%CLI% add --file %FILE% --type expense --amount 277490 --category "항공" --item "여행자 보험"

REM 사역준비
echo   - 사역준비 입력 중...
%CLI% add --file %FILE% --type expense --amount 42300 --category "사역준비" --item "사역팀 모임비" --desc "맥도날드" --date 2024-10-06 --receipt 1
%CLI% add --file %FILE% --type expense --amount 52500 --category "사역준비" --item "사역팀 모임비" --desc "BBQ" --date 2024-10-20 --receipt 2
%CLI% add --file %FILE% --type expense --amount 74000 --category "사역준비" --item "사역팀 모임비" --desc "얌얌 하우스" --date 2024-10-27 --receipt 3
%CLI% add --file %FILE% --type expense --amount 9500 --category "사역준비" --item "사역팀 모임비" --desc "컴포즈 커피" --date 2024-10-27 --receipt 4
%CLI% add --file %FILE% --type expense --amount 55000 --category "사역준비" --item "사역팀 모임비" --desc "신풍 곱창" --date 2024-11-03 --receipt 5
%CLI% add --file %FILE% --type expense --amount 18000 --category "사역준비" --item "사역팀 모임비" --desc "오토김밥 영등포점" --date 2024-11-03 --receipt 6
%CLI% add --file %FILE% --type expense --amount 60200 --category "사역준비" --item "사역팀 모임비" --desc "KFC" --date 2024-11-17 --receipt 7
%CLI% add --file %FILE% --type expense --amount 53800 --category "사역준비" --item "사역팀 모임비" --desc "보스턴 커피" --date 2024-12-07 --receipt 8
%CLI% add --file %FILE% --type expense --amount 25200 --category "사역준비" --item "사역팀 모임비" --desc "보스턴 커피" --date 2024-12-07 --receipt 9
%CLI% add --file %FILE% --type expense --amount 60000 --category "사역준비" --item "사역팀 모임비" --desc "BBQ" --date 2024-12-28 --receipt 10
%CLI% add --file %FILE% --type expense --amount 53980 --category "사역준비" --item "전체 모임비" --desc "과자" --date 2024-11-06 --receipt 11
%CLI% add --file %FILE% --type expense --amount 170400 --category "사역준비" --item "전체 모임비" --desc "한솥 도시락" --date 2024-11-06 --receipt 12
%CLI% add --file %FILE% --type expense --amount 6720 --category "사역준비" --item "전체 모임비" --desc "피크닉 사과(280) * 24" --date 2024-11-08 --receipt 13
%CLI% add --file %FILE% --type expense --amount 148600 --category "사역준비" --item "전체 모임비" --desc "빅맥 세트 * 16 + 빅맥 단품 2개" --date 2024-11-24 --receipt 14
%CLI% add --file %FILE% --type expense --amount 14400 --category "사역준비" --item "전체 모임비" --desc "피크닉 사과(300) * 48" --date 2024-12-06 --receipt 15
%CLI% add --file %FILE% --type expense --amount 226800 --category "사역준비" --item "전체 모임비" --desc "청년 치킨" --date 2024-12-08 --receipt 16
%CLI% add --file %FILE% --type expense --amount 2800 --category "사역준비" --item "전체 모임비" --desc "위생장갑" --date 2024-12-15 --receipt 17
%CLI% add --file %FILE% --type expense --amount 7900 --category "사역준비" --item "전체 모임비" --desc "곽티슈" --date 2024-12-15 --receipt 18
%CLI% add --file %FILE% --type expense --amount 18000 --category "사역준비" --item "전체 모임비" --desc "베지밀" --date 2025-01-02 --receipt 19
%CLI% add --file %FILE% --type expense --amount 12050 --category "사역준비" --item "전체 모임비" --desc "음료수" --date 2025-01-01 --receipt 20
%CLI% add --file %FILE% --type expense --amount 24000 --category "사역준비" --item "전체 모임비" --desc "그레이스 카페" --date 2025-01-04 --receipt 21
%CLI% add --file %FILE% --type expense --amount 150000 --category "사역준비" --item "전체 모임비" --desc "지코바 치킨" --date 2025-01-19 --receipt 22
%CLI% add --file %FILE% --type expense --amount 157100 --category "사역준비" --item "전체 모임비" --desc "노모어 피자" --date 2025-01-19 --receipt 23
%CLI% add --file %FILE% --type expense --amount 10100 --category "사역준비" --item "사역팀 로밍" --desc "박성언 목사" --date 2025-01-05 --receipt 24
%CLI% add --file %FILE% --type expense --amount 29000 --category "사역준비" --item "사역팀 로밍" --desc "서문규" --date 2025-01-05 --receipt 25
%CLI% add --file %FILE% --type expense --amount 44000 --category "사역준비" --item "사역팀 로밍" --desc "김하은" --date 2025-01-05 --receipt 26
%CLI% add --file %FILE% --type expense --amount 22850 --category "사역준비" --item "사역팀 로밍" --desc "장세린" --date 2024-12-31 --receipt 27
%CLI% add --file %FILE% --type expense --amount 19500 --category "사역준비" --item "사역팀 로밍" --desc "김유니" --date 2025-01-02 --receipt 28
%CLI% add --file %FILE% --type expense --amount 45000 --category "사역준비" --item "인쇄물" --desc "X배너 * 3" --date 2024-10-19 --receipt 29
%CLI% add --file %FILE% --type expense --amount 260000 --category "사역준비" --item "인쇄물" --desc "신청서 2000장" --date 2024-10-19 --receipt 29
%CLI% add --file %FILE% --type expense --amount 165000 --category "사역준비" --item "인쇄물" --desc "기도카드" --date 2024-11-16
%CLI% add --file %FILE% --type expense --amount 40000 --category "사역준비" --item "인쇄물" --desc "현수막" --date 2024-12-22 --receipt 31
%CLI% add --file %FILE% --type expense --amount 406800 --category "사역준비" --item "단체티" --desc "티셔츠 * 52 (선교사님 부부 포함)" --date 2024-12-19 --receipt 32
%CLI% add --file %FILE% --type expense --amount 200650 --category "사역준비" --item "여행 간식비" --date 2024-12-30 --receipt 33
%CLI% add --file %FILE% --type expense --amount 36700 --category "사역준비" --item "여행 간식비" --date 2024-12-30 --receipt 34
%CLI% add --file %FILE% --type expense --amount 12640 --category "사역준비" --item "여행 간식비" --date 2024-12-30 --receipt 35

echo   ✓ 지출 데이터 1 입력 완료
echo.

REM ========================================
REM 지출 데이터 2
REM ========================================
echo [3/3] 지출 데이터 2 입력 중...
echo.

REM 팀사역 - 어린이 사역
echo   - 팀사역 (어린이) 입력 중...
%CLI% add --file %FILE% --type expense --amount 588800 --category "팀사역" --item "어린이 사역" --desc "어린이 사역 1" --date 2024-12-19 --receipt 36
%CLI% add --file %FILE% --type expense --amount 122000 --category "팀사역" --item "어린이 사역" --desc "어린이 사역 2-1" --date 2024-12-19 --receipt 37
%CLI% add --file %FILE% --type expense --amount 76200 --category "팀사역" --item "어린이 사역" --desc "어린이 사역 2-2" --date 2024-12-20 --receipt 38
%CLI% add --file %FILE% --type expense --amount 3500 --category "팀사역" --item "어린이 사역" --desc "어린이 사역 2-3" --date 2024-12-20 --receipt 39
%CLI% add --file %FILE% --type expense --amount 10000 --category "팀사역" --item "어린이 사역" --desc "어린이 사역 3-1" --date 2024-12-29 --receipt 40
%CLI% add --file %FILE% --type expense --amount 63800 --category "팀사역" --item "어린이 사역" --desc "어린이 사역 3-2" --date 2024-12-24 --receipt 41
%CLI% add --file %FILE% --type expense --amount 16000 --category "팀사역" --item "어린이 사역" --desc "어린이 사역 3-3" --date 2024-12-23 --receipt 42
%CLI% add --file %FILE% --type expense --amount 37340 --category "팀사역" --item "어린이 사역" --desc "어린이 사역 4-1" --date 2025-01-01 --receipt 43
%CLI% add --file %FILE% --type expense --amount 33700 --category "팀사역" --item "어린이 사역" --desc "어린이 사역 4-2" --date 2024-12-31 --receipt 44
%CLI% add --file %FILE% --type expense --amount 8200 --category "팀사역" --item "어린이 사역" --desc "어린이 사역 4-3" --date 2025-01-02 --receipt 45
%CLI% add --file %FILE% --type expense --amount 29000 --category "팀사역" --item "어린이 사역" --desc "어린이 사역 4-4" --date 2024-12-31 --receipt 46
%CLI% add --file %FILE% --type expense --amount 2000 --category "팀사역" --item "어린이 사역" --desc "어린이 사역 4-5" --date 2024-12-31 --receipt 47

REM 팀사역 - 의료 사역
echo   - 팀사역 (의료) 입력 중...
%CLI% add --file %FILE% --type expense --amount 1002200 --category "팀사역" --item "의료 사역" --desc "한방약" --date 2024-12-17 --receipt 48
%CLI% add --file %FILE% --type expense --amount 35000 --category "팀사역" --item "의료 사역" --desc "비타민 C 500정" --date 2024-12-17 --receipt 49
%CLI% add --file %FILE% --type expense --amount 44600 --category "팀사역" --item "의료 사역" --desc "지퍼백" --date 2024-12-16 --receipt 50
%CLI% add --file %FILE% --type expense --amount 16000 --category "팀사역" --item "의료 사역" --desc "반투명 쇼핑백 100매" --date 2024-12-16 --receipt 51
%CLI% add --file %FILE% --type expense --amount 94600 --category "팀사역" --item "의료 사역" --desc "배게커버, 침, 알콜솜" --date 2024-12-17 --receipt 52
%CLI% add --file %FILE% --type expense --amount 253000 --category "팀사역" --item "의료 사역" --desc "제일 한방 파프수에스" --date 2024-12-19 --receipt 53
%CLI% add --file %FILE% --type expense --amount 227100 --category "팀사역" --item "의료 사역" --desc "약" --date 2024-12-19 --receipt 54
%CLI% add --file %FILE% --type expense --amount 18000 --category "팀사역" --item "의료 사역" --desc "물티슈 및 문구류" --date 2024-12-23 --receipt 55
%CLI% add --file %FILE% --type expense --amount 53950 --category "팀사역" --item "의료 사역" --desc "코멧 2단 캠핑 매트" --date 2024-12-25 --receipt 56
%CLI% add --file %FILE% --type expense --amount 8600 --category "팀사역" --item "의료 사역" --desc "라벨 견출지" --date 2024-12-28 --receipt 57
%CLI% add --file %FILE% --type expense --amount 9100 --category "팀사역" --item "의료 사역" --desc "철불독크립, 특대" --date 2024-12-29 --receipt 58

REM 팀사역 - 찬양
echo   - 팀사역 (찬양) 입력 중...
%CLI% add --file %FILE% --type expense --amount 23500 --category "팀사역" --item "찬양" --desc "켄트지 전지 25매" --date 2024-12-26 --receipt 59
%CLI% add --file %FILE% --type expense --amount 90900 --category "팀사역" --item "찬양" --desc "멀티탭" --date 2025-01-03 --receipt 60
%CLI% add --file %FILE% --type expense --amount 22770 --category "팀사역" --item "찬양" --desc "코멧 모조 전지 30매 + 2건" --date 2025-01-02 --receipt 61
%CLI% add --file %FILE% --type expense --amount 30810 --category "팀사역" --item "찬양" --desc "마이크 케이블 5m 1개 + 2건" --date 2025-01-03 --receipt 62

REM 기타
echo   - 기타 입력 중...
%CLI% add --file %FILE% --type expense --amount 30650 --category "기타" --item "선교사님 요청 물품" --desc "통기타줄 * 5" --date 2024-12-19 --receipt 71
%CLI% add --file %FILE% --type expense --amount 100200 --category "기타" --item "선교사님 요청 물품" --desc "에어컨 필터 * 10" --date 2024-12-19 --receipt 71
%CLI% add --file %FILE% --type expense --amount 10890 --category "기타" --item "선교사님 요청 물품" --desc "선교사님 차 손잡이 * 3" --date 2025-01-03 --receipt 72
%CLI% add --file %FILE% --type expense --amount 4000 --category "기타" --item "선교사님 요청 물품" --desc "선교사님 차 손잡이" --date 2025-01-03 --receipt 73
%CLI% add --file %FILE% --type expense --amount 47200 --category "기타" --item "선교사님 요청 물품" --desc "다이소" --date 2025-01-04 --receipt 74
%CLI% add --file %FILE% --type expense --amount 3000 --category "기타" --item "다이소" --date 2025-01-01 --receipt 75
%CLI% add --file %FILE% --type expense --amount 15000 --category "기타" --item "약" --date 2025-01-04 --receipt 76
%CLI% add --file %FILE% --type expense --amount 22600 --category "기타" --item "약" --date 2025-01-03 --receipt 77
%CLI% add --file %FILE% --type expense --amount 16000 --category "기타" --item "다이소" --date 2025-01-03 --receipt 78
%CLI% add --file %FILE% --type expense --amount 10200 --category "기타" --item "다이소" --date 2025-01-17 --receipt 79
%CLI% add --file %FILE% --type expense --amount 512000 --category "기타" --item "선교사님 헌금" --date 2024-12-24
%CLI% add --file %FILE% --type expense --amount 102400 --category "기타" --item "예비비" --desc "페소 환전" --date 2024-12-24
%CLI% add --file %FILE% --type expense --amount 100000 --category "기타" --item "이송국 집사님 사례비" --date 2024-12-24
%CLI% add --file %FILE% --type expense --amount 100000 --category "기타" --item "유류비" --date 2024-12-24 --receipt 80
%CLI% add --file %FILE% --type expense --amount 10000 --category "기타" --item "복사카드" --receipt 81
%CLI% add --file %FILE% --type expense --amount 119600 --category "기타" --item "이민가방 * 4" --date 2024-12-29 --receipt 82

echo   ✓ 지출 데이터 2 입력 완료
echo.

REM ========================================
REM 완료
REM ========================================
echo ========================================
echo     전체 데이터 입력 완료!
echo ========================================
echo.
echo 결과 확인:
%CLI% list %FILE%
echo.

pause
