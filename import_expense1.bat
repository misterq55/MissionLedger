@echo off
chcp 65001 > nul
set CLI=C:\Work\Others\MissionLedger\x64\Release\MissionLedgerCLI.exe
set FILE=data.ml

echo === 지출 데이터 입력 시작 (지출1) ===

REM 팀사역 - 어린이 사역
echo [팀사역 - 어린이 사역] 입력 중...
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
echo [팀사역 - 의료 사역] 입력 중...
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
echo [팀사역 - 찬양] 입력 중...
%CLI% add --file %FILE% --type expense --amount 23500 --category "팀사역" --item "찬양" --desc "켄트지 전지 25매" --date 2024-12-26 --receipt 59
%CLI% add --file %FILE% --type expense --amount 90900 --category "팀사역" --item "찬양" --desc "멀티탭" --date 2025-01-03 --receipt 60
%CLI% add --file %FILE% --type expense --amount 22770 --category "팀사역" --item "찬양" --desc "코멧 모조 전지 30매 + 2건" --date 2025-01-02 --receipt 61
%CLI% add --file %FILE% --type expense --amount 30810 --category "팀사역" --item "찬양" --desc "마이크 케이블 5m 1개 + 2건" --date 2025-01-03 --receipt 62

REM 기타 - 선교사님 요청 물품
echo [기타 - 선교사님 요청 물품] 입력 중...
%CLI% add --file %FILE% --type expense --amount 30650 --category "기타" --item "선교사님 요청 물품" --desc "통기타줄 * 5" --date 2024-12-19 --receipt 71
%CLI% add --file %FILE% --type expense --amount 100200 --category "기타" --item "선교사님 요청 물품" --desc "에어컨 필터 * 10" --date 2024-12-19 --receipt 71
%CLI% add --file %FILE% --type expense --amount 10890 --category "기타" --item "선교사님 요청 물품" --desc "선교사님 차 손잡이 * 3" --date 2025-01-03 --receipt 72
%CLI% add --file %FILE% --type expense --amount 4000 --category "기타" --item "선교사님 요청 물품" --desc "선교사님 차 손잡이" --date 2025-01-03 --receipt 73
%CLI% add --file %FILE% --type expense --amount 47200 --category "기타" --item "선교사님 요청 물품" --desc "다이소" --date 2025-01-04 --receipt 74

REM 기타 - 기타 지출
echo [기타 - 기타 지출] 입력 중...
%CLI% add --file %FILE% --type expense --amount 47200 --category "기타" --item "다이소" --date 2025-01-04 --receipt 74
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

echo === 입력 완료 ===
echo 결과 확인:
%CLI% list %FILE%

pause
