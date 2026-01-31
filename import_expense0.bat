@echo off
chcp 65001 > nul
set CLI=C:\Work\Others\MissionLedger\x64\Release\MissionLedgerCLI.exe
set FILE=data.ml

echo === 지출 데이터 입력 시작 ===

REM 항공
echo [항공] 입력 중...
%CLI% add --file %FILE% --type expense --amount 13800000 --category "항공" --item "항공료" --desc "선결제(300만원) 제외한 비행기 티켓 금액" --date 2024-12-13
%CLI% add --file %FILE% --type expense --amount 277490 --category "항공" --item "여행자 보험"

REM 사역준비
echo [사역준비] 입력 중...
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

echo === 입력 완료 ===
echo 결과 확인:
%CLI% list %FILE%

pause
